static gboolean pixbuf_save_tiff(GdkPixbuf *pixbuf, char *filename)
{
    long        row;
    TIFF       *tif;
    struct tm  *ct;
    time_t t;
    int         width,
                height,
		rowstride;
    guchar     *pixels;
    short       photometric;
    short       samplesperpixel;
    short       bitspersample;
    static char datetime[40] = "";
    char hostname[256] = "";

    time(&t);
    ct = localtime(&t);
    sprintf(datetime, "%04i:%02i:%02i %02i:%02i:%02i",
	    1900 + ct->tm_year, ct->tm_mon + 1, ct->tm_mday,
	    ct->tm_hour, ct->tm_min, ct->tm_sec);

    tif = TIFFOpen(filename, "w");

    width = gdk_pixbuf_get_width(pixbuf);
    height = gdk_pixbuf_get_height(pixbuf);
    samplesperpixel = gdk_pixbuf_get_has_alpha(pixbuf) ? 4 : 3;
    bitspersample = gdk_pixbuf_get_bits_per_sample(pixbuf);
    photometric = PHOTOMETRIC_RGB;
    rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);


    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitspersample);
    TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
    /*#TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, inf);*/
    TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "Saved from a Gdk::Pixbuf");
    TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(tif, TIFFTAG_SOFTWARE, "LiveLink Photo Kiosk 1.0");
    TIFFSetField(tif, TIFFTAG_DATETIME, datetime);
    if(gethostname((char*)&hostname,sizeof(hostname))==0)
    TIFFSetField(tif, TIFFTAG_HOSTCOMPUTER, hostname);

    for (row = 0; row < height; row++) {
      if (TIFFWriteScanline(tif, (u_char *) pixels, row, 0) < 0) {
        fprintf(stderr, "failed a scanline write (%li)\n", row);
        break;
      }
      pixels = GINT_TO_POINTER(GPOINTER_TO_INT(pixels) + rowstride);
    }
    TIFFFlushData(tif);
    TIFFClose(tif);

    return TRUE;
}

