#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
/* Includes */
#include <ruby.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gdk-pixbuf/gdk-pixbuf.h"
#include "rbglib.h"
#include "rbgobject.h"
#include "tiffio.h"
#include "gdk/gdk.h"

/* Setup types */
/* Try not to clash with other definitions of bool... */
typedef int rubber_bool;
#define bool rubber_bool

/* Prototypes */
#include "rbglib.h"

static VALUE mPixbufUtils;
static VALUE
PixbufUtils_CLASS_remove_alpha(VALUE self, VALUE __v_src, VALUE __v_col);
static VALUE
PixbufUtils_CLASS_sharpen(VALUE self, VALUE __v_src, VALUE __v_radius);
static VALUE
PixbufUtils_CLASS_extract_alpha(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_blur(VALUE self, VALUE __v_src, VALUE __v_radius);
static VALUE
PixbufUtils_CLASS_rotate_90(VALUE self, VALUE __v_src, VALUE __v_counter_clockwise);
static VALUE
PixbufUtils_CLASS_rotate_cw(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_rotate_180(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_rotate(VALUE self, VALUE __v_src, VALUE __v_angle);
static VALUE
PixbufUtils_CLASS_rotate_ccw(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_gamma_pling(VALUE self, VALUE __v_src, VALUE __v_level);
static VALUE
PixbufUtils_CLASS_soften_edges_pling(VALUE self, VALUE __v_src, VALUE __v_size);
static VALUE
PixbufUtils_CLASS_gamma(VALUE self, VALUE __v_src, VALUE __v_level);
static VALUE
PixbufUtils_CLASS_greyscale_pling(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_to_tiff(VALUE self, VALUE __v_src, VALUE __v_filename);
static VALUE
PixbufUtils_CLASS_greyscale(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_greyscale_go_pling(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_greyscale_go(VALUE self, VALUE __v_src);
static VALUE
PixbufUtils_CLASS_tint(VALUE self, VALUE __v_src, VALUE __v_r, VALUE __v_g, VALUE __v_b);
static VALUE
PixbufUtils_CLASS_perspect_v(VALUE self, VALUE __v_src, VALUE __v_top_x1, VALUE __v_top_x2, VALUE __v_bot_x1, VALUE __v_bot_x2);
static VALUE
PixbufUtils_CLASS_mask(VALUE self, VALUE __v_src, VALUE __v_mask);
static VALUE
PixbufUtils_CLASS_blend5050(VALUE self, VALUE __v_src1, VALUE __v_src2);
static VALUE
PixbufUtils_CLASS_mask_area(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_scale_max(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_draw_scaled(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_draw_scaled_clip(int __p_argc, VALUE *__p_argv, VALUE self);

/* Inline C code */


extern void Init_pixbufutils(void);

#ifndef IGNORE
#define IGNORE(x) x=x
#endif

static GdkPixbuf *pixbuf_extract_alpha(GdkPixbuf *src, int cutoff, int force_2bit)
{
	int        s_has_alpha, d_has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	GdkPixbuf *dest;
	guchar    *s_pix, *sp;
        guchar    *d_pix, *dp;
	int        i, j, pix_width, grey;
	
	g_return_val_if_fail(src != NULL, NULL);
	
	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	s_has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);
	g_return_val_if_fail(s_has_alpha, NULL);

	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, s_width, s_height);
	
	g_return_val_if_fail(dest != NULL, NULL);

	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	d_width = gdk_pixbuf_get_width(dest);
	d_height = gdk_pixbuf_get_height(dest);
	d_has_alpha = gdk_pixbuf_get_has_alpha(dest);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);
	
	g_return_val_if_fail(d_width == s_width, NULL);
	g_return_val_if_fail(d_height == s_height, NULL);
	
	pix_width = (s_has_alpha ? 4 : 3);

	for (i = 0; i < s_height; i++) {
		sp = s_pix;
		dp = d_pix;
		
		for (j = 0; j < s_width; j++) {
			grey = sp[3];
			
			if (grey < cutoff)
				grey = 0;
				
			if (force_2bit) 
				grey = (grey >= cutoff ? 255 : 0);
			
			dp[0] = grey;	/* red */
			dp[1] = grey;	/* green */
			dp[2] = grey;	/* blue */
			
			dp += 3;
			sp += 4;
		}
		
		d_pix += d_rowstride;
		s_pix += s_rowstride;
	}

	return dest;
}


typedef enum {
	ANGLE_0 = 0,
	ANGLE_90 = 90,
	ANGLE_180 = 180,
	ANGLE_270 = 270
} rotate_angle_t;

static GdkPixbuf *
pixbuf_blur(GdkPixbuf *src, gint radius)
{
	/* Pixels and Rowstride To Perform Blur On */
	guchar *pixels;
	GdkPixbuf *pixbuf;
	gint rowstride, width, height;
	
	pixbuf = gdk_pixbuf_copy(src);

	/* Get Pixels and Rowstride Of Image Buffer */
	if (pixbuf)
	{
		pixels = gdk_pixbuf_get_pixels (pixbuf);
		rowstride = gdk_pixbuf_get_rowstride (pixbuf);
		width = gdk_pixbuf_get_width(pixbuf);
		height = gdk_pixbuf_get_height(pixbuf);
		/*printf("%i, %i, %i, %i\n", rowstride, width, height, rowstride/width);*/
	}
	else
	{
		return NULL;
	}
	if (radius > 1)
	{      
		/* Some Important Consts */	
		gint bytes = rowstride/width;
		gboolean alpha = (bytes == 4);

		gint div = radius+radius+1;
		gint divsum = ((div+1)>>1)*((div+1)>>1);
		gint dv[256*divsum]; /* Table of Const RGBA Values */
	
		/* Some Important Variables */
		guchar stack[div][bytes];
		gint stackpointer;
		gint stackstart;
		gint vmin[MAX(width,height)];
		guchar *sir;
		gint rbs;
	
		gint current = 0;
	
		/* RGBA Sums
			0 - Sum of Incoming pixels(the radius pixels above the Center/to left of Center)
			1 - Sum of Outgoing pixels(the Center and the radius pixels below the Center/to right of Center)
			2 - Sum of All pixels within radius
		*/
		gint rgba_sums[3][bytes];
		
		/* RGBA Values */
		guchar rgba[bytes][width*height];

		/* Temp Indexes/Counters */
		gint x, y, i, p, yp, yi=0, yw=0;

		for (i=0; i<256*divsum; i++)
		{
			dv[i] = (i/divsum);
		}

		for (y=0; y < height; y++)
		{
			/* initialize sums to zero */
			for (i = 0; i<bytes; i++)
			{
				rgba_sums[0][i] = 0;
				rgba_sums[1][i] = 0;
				rgba_sums[2][i] = 0;
			}

			/* Calculate Initial Sums For radius */
			for(i=-radius; i<=radius; i++)
			{
				current = (yi + MIN(width - 1, MAX(i,0)))*bytes;

				sir = stack[i+radius];

				sir[0] = pixels[current + 0];
				sir[1] = pixels[current + 1];
				sir[2] = pixels[current + 2];
				sir[3] = pixels[current + 3];

				rbs = (radius + 1) - abs(i);

				rgba_sums[2][0] += sir[0]*rbs;
				rgba_sums[2][1] += sir[1]*rbs;
				rgba_sums[2][2] += sir[2]*rbs;
				if (alpha)
				{
					rgba_sums[2][3] += sir[3]*rbs;
				}

				if (i>0)
				{
					rgba_sums[0][0] += sir[0];
					rgba_sums[0][1] += sir[1];
					rgba_sums[0][2] += sir[2];
					if (alpha)
					{
						rgba_sums[0][3] += sir[3];
					}
				} 
				else 
				{
					rgba_sums[1][0] += sir[0];
					rgba_sums[1][1] += sir[1];
					rgba_sums[1][2] += sir[2];
					if (alpha)
					{
						rgba_sums[1][3] += sir[3];
					}
				}
			}
			
			stackpointer = radius;

			for (x=0; x<width; x++)
			{
				rgba[0][yi] = dv[rgba_sums[2][0]];
				rgba[1][yi] = dv[rgba_sums[2][1]];
				rgba[2][yi] = dv[rgba_sums[2][2]];
				if (alpha)
				{
					rgba[3][yi] = dv[rgba_sums[2][3]];
				}
      
				rgba_sums[2][0] -= rgba_sums[1][0];
				rgba_sums[2][1] -= rgba_sums[1][1];
				rgba_sums[2][2] -= rgba_sums[1][2];
				if (alpha)
				{
					rgba_sums[2][3] -= rgba_sums[1][3];
				}
			
				stackstart = stackpointer - radius + div;
				sir = stack[stackstart%div];
	      
				rgba_sums[1][0] -= sir[0];
				rgba_sums[1][1] -= sir[1];
				rgba_sums[1][2] -= sir[2];
				if (alpha)
				{
					rgba_sums[1][3] -= sir[3];
				}
	      
				if(y==0)
				{
					vmin[x] = MIN(x + radius + 1, width - 1);
				}
	
				current = (yw + vmin[x])*bytes;
	
				sir[0] = pixels[current + 0];
				sir[1] = pixels[current + 1];
				sir[2] = pixels[current + 2];
				if (alpha)
				{
					sir[3] = pixels[current + 3];
				}
	
				rgba_sums[0][0] += sir[0];
				rgba_sums[0][1] += sir[1];
				rgba_sums[0][2] += sir[2];
				if (alpha)
				{
					rgba_sums[0][3] += sir[3];
				}
				
				rgba_sums[2][0] += rgba_sums[0][0];
				rgba_sums[2][1] += rgba_sums[0][1];
				rgba_sums[2][2] += rgba_sums[0][2];
				if (alpha)
				{
					rgba_sums[2][3] += rgba_sums[0][3];
				}
				
				stackpointer=(stackpointer+1)%div;
				sir=stack[(stackpointer)%div];
	
				rgba_sums[1][0] += sir[0];
				rgba_sums[1][1] += sir[1];
				rgba_sums[1][2] += sir[2];
				if (alpha)
				{
					rgba_sums[1][3] += sir[3];
				}
	     			
				rgba_sums[0][0] -= sir[0];
				rgba_sums[0][1] -= sir[1];
				rgba_sums[0][2] -= sir[2];
				if (alpha)
				{
					rgba_sums[0][3] -= sir[3];
				}
	     			
				yi++;
			}
	
			yw += width;
		}
	
		for (x=0; x<width; x++)
		{
			yp=-radius*width;
	
			/* initialize sums to zero */
			for (i = 0; i<bytes; i++)
			{
				rgba_sums[0][i] = 0;
				rgba_sums[1][i] = 0;
				rgba_sums[2][i] = 0;
			}
	
			/* Calculate Initial Sums For radius */
			for(i=-radius; i<=radius; i++)
			{
				yi = MAX(0,yp) + x;
	     
				sir = stack[i+radius];
	      
				sir[0] = rgba[0][yi];
				sir[1] = rgba[1][yi];
				sir[2] = rgba[2][yi];
				if (alpha)
				{
					sir[3] = rgba[3][yi];
				}	
	
				rbs = (radius + 1) - abs(i);
	      
				rgba_sums[2][0] += rgba[0][yi]*rbs;
				rgba_sums[2][1] += rgba[1][yi]*rbs;
				rgba_sums[2][2] += rgba[2][yi]*rbs;
				if (alpha)
				{
					rgba_sums[2][3] += rgba[3][yi]*rbs;
				}     
	
				if (i>0)
				{
					rgba_sums[0][0] += sir[0];
					rgba_sums[0][1] += sir[1];
					rgba_sums[0][2] += sir[2];
					if (alpha)
					{
						rgba_sums[0][3] += sir[3];
					} 
				}
				else 
				{
					rgba_sums[1][0] += sir[0];
					rgba_sums[1][1] += sir[1];
					rgba_sums[1][2] += sir[2];
					if (alpha)
					{
						rgba_sums[1][3] += sir[3];
					}
				}
	      
				if(i < height - 1)
				{
					yp += width;
				}
			}
	
			yi = x;
			stackpointer = radius;
	
			for (y=0; y<height; y++)
			{
				current = (yi)*bytes;
	
				pixels[current + 0] = dv[rgba_sums[2][0]];
				pixels[current + 1] = dv[rgba_sums[2][1]];
				pixels[current + 2] = dv[rgba_sums[2][2]];
				if (alpha)
				{
					pixels[current + 3] = dv[rgba_sums[2][3]];
				}
	
				rgba_sums[2][0] -= rgba_sums[1][0];
				rgba_sums[2][1] -= rgba_sums[1][1];
				rgba_sums[2][2] -= rgba_sums[1][2];
				if (alpha)
				{
					rgba_sums[2][3] -= rgba_sums[1][3];
				}
	
				stackstart = stackpointer - radius + div;
				sir = stack[stackstart%div];
	     
				rgba_sums[1][0] -= sir[0];
				rgba_sums[1][1] -= sir[1];
				rgba_sums[1][2] -= sir[2];
				if (alpha)
				{
					rgba_sums[1][3] -= sir[3];
	     			}
	
				if (x == 0)
				{
					vmin[y] = MIN(y + (radius + 1), height - 1)*width;
				}
	
				p = x + vmin[y];
	      
				sir[0] = rgba[0][p];
				sir[1] = rgba[1][p];
				sir[2] = rgba[2][p];
				if (alpha)
				{
					sir[3] = rgba[3][p];
				}
	      
				rgba_sums[0][0] += sir[0];
				rgba_sums[0][1] += sir[1];
				rgba_sums[0][2] += sir[2];
				if (alpha)
				{
					rgba_sums[0][3] += sir[3];
				}
	
				rgba_sums[2][0] += rgba_sums[0][0];
				rgba_sums[2][1] += rgba_sums[0][1];
				rgba_sums[2][2] += rgba_sums[0][2];
				if (alpha)
				{
					rgba_sums[2][3] += rgba_sums[0][3];
				}
	
				stackpointer = (stackpointer+1)%div;
				sir = stack[stackpointer];
	     
				rgba_sums[1][0] += sir[0];
				rgba_sums[1][1] += sir[1];
				rgba_sums[1][2] += sir[2];
				if (alpha)
				{
					rgba_sums[1][3] += sir[3];
				}      
	
				rgba_sums[0][0] -= sir[0];
				rgba_sums[0][1] -= sir[1];
				rgba_sums[0][2] -= sir[2];
				if (alpha)
				{
					rgba_sums[0][3] -= sir[3];
				}
	
				yi += width;
			}
		}
	}
	
	return pixbuf;
}

static GdkPixbuf *
pixbuf_sharpen(GdkPixbuf *src, int strength)
{
	GdkPixbuf *dest;
	int        has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix;
        guchar    *d_pix;
	guchar    *sp;
        guchar    *dp;
	int       pix_width;
	bool	row_only = TRUE;
	int        a=0, r, g, b, x, y, mul;
	int        b_a=0, b_r, b_g, b_b;
	
	if (!src) return NULL;
	
	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	d_width = s_width;
	d_height = s_height;
	
	dest = gdk_pixbuf_copy(src);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	pix_width = (has_alpha ? 4 : 3);

 	mul = (row_only ? 3 : 5);

	for (y = 0; y < (s_height); y++)
	{
             sp = s_pix + (y * s_rowstride) + pix_width;
             dp = d_pix + (y * d_rowstride) + pix_width;

		for (x = 1; x < (s_width - 1); x++)
		{
			
			b_r = ((int)*(sp));
			b_g = ((int)*(sp+1));
			b_b = ((int)*(sp+2));
			if (has_alpha)
				b_a = ((int)*((sp+3)));
			
			r = b_r * mul;
			g = b_g * mul;
			b = b_b * mul;
			if (has_alpha)
				a = b_a * mul;

			r = ((int)*(sp)) * mul;
			g = ((int)*(sp+1)) * mul;
			b = ((int)*(sp+2)) * mul;
			if (has_alpha)
				a = ((int)*((sp+3))) * mul;

 			r -= (int)*(sp - pix_width);
			g -= (int)*(sp - pix_width + 1);
			b -= (int)*(sp - pix_width + 2);
			if (has_alpha)
				a -= (int)*(sp - pix_width + 3);
			
			r -= (int)*(sp + pix_width);
			g -= (int)*(sp + pix_width + 1);
			b -= (int)*(sp + pix_width + 2);
			if (has_alpha)
				a -= (int)*(sp + pix_width + 3);

			if (row_only == 0)
			{
				r -= (int)*(sp - (s_rowstride));
				g -= (int)*(sp + 1 - (s_rowstride));
				b -= (int)*(sp + 2 - (s_rowstride));
				if (has_alpha)
					a -= (int)*(sp + 3 - (s_rowstride));

				r -= (int)*(sp + (s_rowstride));
				g -= (int)*(sp + 1 + (s_rowstride));
				b -= (int)*(sp + 2 + (s_rowstride));
				if (has_alpha)
					a -= (int)*(sp + 3 + (s_rowstride));
			}

			r = (r & ((~r) >> 16));
			r = ((r | ((r & 256) - ((r & 256) >> 8))));
			g = (g & ((~g) >> 16));
			g = ((g | ((g & 256) - ((g & 256) >> 8))));
			b = (b & ((~b) >> 16));
			b = ((b | ((b & 256) - ((b & 256) >> 8))));
			a = (a & ((~a) >> 16));
			a = ((a | ((a & 256) - ((a & 256) >> 8))));
			
			r = 0xff & (MAX(r,0));
			g = 0xff & (MAX(g,0));
			b = 0xff & (MAX(b,0));
			if (has_alpha)
				a = 0xff & (MAX(a,0));
			
			r = ((r * strength) + b_r) / (strength + 1);
			g = ((g * strength) + b_g) / (strength + 1);
			b = ((b * strength) + b_b) / (strength + 1);
			if (has_alpha)
				a = ((a * strength) + b_a) / (strength + 1);
			
			*(dp++) = r;
			*(dp++) = g;
			*(dp++) = b;
			if (has_alpha)
				*(dp++) = a;
			
			sp += pix_width;
		}
	}
	return dest;
} // */

static GdkPixbuf *
pixbuf_rotate(GdkPixbuf *src, rotate_angle_t angle)
{
	GdkPixbuf *dest;
	int        has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix;
        guchar    *d_pix;
	guchar    *sp;
        guchar    *dp;
	int        i, j, pix_width;

	if (!src) return NULL;
	
	if (angle == ANGLE_0)
		return gdk_pixbuf_copy(src);

	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	switch (angle)
	{
	case ANGLE_90:
	case ANGLE_270:
		d_width = s_height;
		d_height = s_width;
		break;
	default:
	case ANGLE_0:/* Avoid compiler warnings... */
	case ANGLE_180:
		d_width = s_width;
		d_height = s_height;
		break;
	}
	
	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, has_alpha, 8, d_width, d_height);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	pix_width = (has_alpha ? 4 : 3);

	for (i = 0; i < s_height; i++) {
		sp = s_pix + (i * s_rowstride);
		for (j = 0; j < s_width; j++) {
			switch (angle)
			{
			case ANGLE_180:
				dp = d_pix + ((d_height - i - 1) * d_rowstride) + ((d_width - j - 1) * pix_width);
				break;
			case ANGLE_90:
				dp = d_pix + (j * d_rowstride) + ((d_width - i - 1) * pix_width);
				break;
			case ANGLE_270:
				dp = d_pix + ((d_height - j - 1) * d_rowstride) + (i * pix_width);
				break;
			default:
			case ANGLE_0:/* Avoid compiler warnings... */
				dp = d_pix + (i * d_rowstride) + (j * pix_width);
				break;
			}

			*(dp++) = *(sp++);	/* red */
			*(dp++) = *(sp++);	/* green */
			*(dp++) = *(sp++);	/* blue */
			if (has_alpha) *(dp) = *(sp++);	/* alpha */
		}
	}

	return dest;
}

#include <unistd.h>

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

#include <math.h>

static GdkPixbuf *pixbuf_gamma(GdkPixbuf *src, GdkPixbuf *dest, double gamma)
{
	int        has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix;
        guchar    *d_pix;
	guchar    *sp;
        guchar    *dp;
	int        i, j, pix_width;
	double map[256] = {0.0,};
	
	for (i=0; i < 256; i++)
		map[i] = 255 * pow((double) i/255, 1.0/gamma);

	g_return_val_if_fail(src != NULL, NULL);
	g_return_val_if_fail(dest != NULL, NULL);

	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	d_width = gdk_pixbuf_get_width(dest);
	d_height = gdk_pixbuf_get_height(dest);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);
	
	g_return_val_if_fail(d_width == s_width, NULL);
	g_return_val_if_fail(d_height == s_height, NULL);
	g_return_val_if_fail(has_alpha == gdk_pixbuf_get_has_alpha(dest), NULL);

	pix_width = (has_alpha ? 4 : 3);

	for (i = 0; i < s_height; i++) {
		sp = s_pix + (i * s_rowstride);
		dp = d_pix + (i * d_rowstride);
		for (j = 0; j < s_width; j++) {
			*(dp++) = map[*(sp++)];	/* red */
			*(dp++) = map[*(sp++)];	/* green */
			*(dp++) = map[*(sp++)];	/* blue */
			if (has_alpha) *(dp++) = map[*(sp++)];	/* alpha */
		}
	}

	return dest;
}

#define PIXEL(row, channels, x)  ((pixel_t)(row + (channels * x)))

typedef struct {
unsigned char r,g,b,a;
} * pixel_t;

#define RLUM    (0.3086)
#define GLUM    (0.6094)
#define BLUM    (0.0820)

// Graphica Obscure
#define GO_RGB_TO_GREY(r,g,b) ((int)((RLUM * (double)r) + (GLUM * (double)g) + (BLUM * (double)b)))

// Gimp Values
#define GIMP_RGB_TO_GREY(r,g,b) (((77 * r) + (151 * g) + (28 * b)) >> 8)

static GdkPixbuf *pixbuf_mask(GdkPixbuf *src, GdkPixbuf *mask)
{
	int        s_has_alpha, m_has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	int        m_width, m_height, m_rowstride;
	guchar    *s_pix, *sp;
        guchar    *d_pix, *dp;
        guchar    *m_pix, *mp;
	int        i, j, pix_width, alpha, grey;
	pixel_t    pix;
	GdkPixbuf *dest;
	
	g_return_val_if_fail(src != NULL, NULL);

	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	s_has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	g_return_val_if_fail(mask != NULL, NULL);
	
	m_width = gdk_pixbuf_get_width(mask);
	m_height = gdk_pixbuf_get_height(mask);
	m_has_alpha = gdk_pixbuf_get_has_alpha(mask);
	m_rowstride = gdk_pixbuf_get_rowstride(mask);
	m_pix = gdk_pixbuf_get_pixels(mask);
	
	g_return_val_if_fail(m_width <= s_width, NULL);
	g_return_val_if_fail(m_height <= s_height, NULL);
	
	d_width = m_width;
	d_height = m_height;
	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, d_width, d_height);
	
	g_return_val_if_fail(dest != NULL, NULL);

	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	pix_width = (m_has_alpha ? 4 : 3);



	for (i = 0; i < m_height; i++) {
		sp = s_pix + (i * s_rowstride);
		dp = d_pix + (i * d_rowstride);
		mp = m_pix + (i * m_rowstride);
		
		for (j = 0; j < m_width; j++) {
			*(dp++) = *(sp++);	/* red */
			*(dp++) = *(sp++);	/* green */
			*(dp++) = *(sp++);	/* blue */
			
			if (s_has_alpha) 
			{
				alpha = *(sp++);	/* alpha */
			}
			else
			{
				alpha = 0xff;
			}
			
			pix = PIXEL(mp, pix_width, j);
			grey = GIMP_RGB_TO_GREY(pix->r, pix->g, pix->b);
			
			*(dp++) = sqrt(alpha * (255 - grey));	/* alpha */
		}
	}

	return dest;
}

static GdkPixbuf *pixbuf_blend5050(GdkPixbuf *src1, GdkPixbuf *src2)
{
	int        s1_has_alpha, s2_has_alpha;
	int        s1_width, s1_height, s1_rowstride;
	int        d_width, d_height, d_rowstride;
	int        s2_width, s2_height, s2_rowstride;
	guchar    *s1_pix, *sp;
        guchar    *d_pix, *dp;
        guchar    *s2_pix, *mp;
	int        i, j;
	GdkPixbuf *dest;
	
	g_return_val_if_fail(src1 != NULL, NULL);
	
	s1_width      = gdk_pixbuf_get_width(src1);
	s1_height     = gdk_pixbuf_get_height(src1);
	s1_has_alpha  = gdk_pixbuf_get_has_alpha(src1);
	s1_rowstride  = gdk_pixbuf_get_rowstride(src1);
	s1_pix        = gdk_pixbuf_get_pixels(src1);
	
	g_return_val_if_fail(src2 != NULL, NULL);
	
	s2_width      = gdk_pixbuf_get_width(src2);
	s2_height     = gdk_pixbuf_get_height(src2);
	s2_has_alpha  = gdk_pixbuf_get_has_alpha(src2);
	s2_rowstride  = gdk_pixbuf_get_rowstride(src2);
	s2_pix        = gdk_pixbuf_get_pixels(src2);
	
	g_return_val_if_fail(s2_width     == s1_width,     NULL);
	g_return_val_if_fail(s2_height    == s1_height,    NULL);
	g_return_val_if_fail(s2_has_alpha == s1_has_alpha, NULL);
	
	d_width       = s2_width;
	d_height      = s2_height;
	
	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, d_width, d_height);
	
	g_return_val_if_fail(dest != NULL, NULL);
	
	d_rowstride   = gdk_pixbuf_get_rowstride(dest);
	d_pix         = gdk_pixbuf_get_pixels(dest);
	
	for (i = 0; i < s2_height; i++) {
		sp = s1_pix + (i * s1_rowstride);
		mp = s2_pix + (i * s2_rowstride);
		dp = d_pix  + (i * d_rowstride);
		
		for (j = 0; j < s2_width; j++) {
			*(dp++) = ((*(sp++)) >> 1) + ((*(mp++)) >> 1);	/* red   */
			*(dp++) = ((*(sp++)) >> 1) + ((*(mp++)) >> 1);	/* green */
			*(dp++) = ((*(sp++)) >> 1) + ((*(mp++)) >> 1);	/* blue  */
			
			if (s1_has_alpha)
				*(dp++) = ((*(sp++)) >> 1) + ((*(mp++)) >> 1);	/* alpha */
			else
				*(dp++) = 0xff;					/* alpha */
			
		}
	}
	
	return dest;
}


static GdkPixbuf *pixbuf_greyscale(GdkPixbuf *src, GdkPixbuf *dest)
{
	int        s_has_alpha, d_has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix, *sp;
        guchar    *d_pix, *dp;
	int        i, j, pix_width, grey;
	
	g_return_val_if_fail(src != NULL, NULL);
	g_return_val_if_fail(dest != NULL, NULL);
	
	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	s_has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	d_width = gdk_pixbuf_get_width(dest);
	d_height = gdk_pixbuf_get_height(dest);
	d_has_alpha = gdk_pixbuf_get_has_alpha(dest);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);
	
	g_return_val_if_fail(d_width == s_width, NULL);
	g_return_val_if_fail(d_height == s_height, NULL);
	g_return_val_if_fail(d_has_alpha == s_has_alpha, NULL);
	
	pix_width = (s_has_alpha ? 4 : 3);


	for (i = 0; i < s_height; i++) {
		sp = s_pix;
		dp = d_pix;
		
		for (j = 0; j < s_width; j++) {
			grey = GIMP_RGB_TO_GREY(sp[0], sp[1], sp[2]);
			
			dp[0] = grey;	/* red */
			dp[1] = grey;	/* green */
			dp[2] = grey;	/* blue */
			
			if (s_has_alpha) 
			{
				dp[3] = sp[3];	/* alpha */
			}

			dp += pix_width;
			sp += pix_width;
		}
		
		d_pix += d_rowstride;
		s_pix += s_rowstride;
	}

	return dest;
}

static GdkPixbuf *pixbuf_greyscale_go(GdkPixbuf *src, GdkPixbuf *dest)
{
	int        s_has_alpha, d_has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix, *sp;
        guchar    *d_pix, *dp;
	int        i, j, pix_width, grey;

	g_return_val_if_fail(src != NULL, NULL);
	g_return_val_if_fail(dest != NULL, NULL);
	
	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	s_has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	d_width = gdk_pixbuf_get_width(dest);
	d_height = gdk_pixbuf_get_height(dest);
	d_has_alpha = gdk_pixbuf_get_has_alpha(dest);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);
	
	g_return_val_if_fail(d_width == s_width, NULL);
	g_return_val_if_fail(d_height == s_height, NULL);
	g_return_val_if_fail(d_has_alpha == s_has_alpha, NULL);
	
	pix_width = (s_has_alpha ? 4 : 3);

	for (i = 0; i < s_height; i++) {
		sp = s_pix;
		dp = d_pix;
		
		for (j = 0; j < s_width; j++) {
			grey = GO_RGB_TO_GREY(sp[0], sp[1], sp[2]);
			
			dp[0] = grey;	/* red */
			dp[1] = grey;	/* green */
			dp[2] = grey;	/* blue */
			
			if (s_has_alpha) 
			{
				dp[3] = sp[3];	/* alpha */
			}

			dp += pix_width;
			sp += pix_width;
		}
		
		d_pix += d_rowstride;
		s_pix += s_rowstride;
	}

	return dest;
}

static inline char pu_clamp(int x, int min, int max)
{
	return  (x > max) ? max : (x < min ? min : x);
}

static GdkPixbuf *pixbuf_tint(GdkPixbuf *src, GdkPixbuf *dest, int r, int g, int b)
{
	int        s_has_alpha, d_has_alpha;
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix, *sp;
	guchar    *d_pix, *dp;
	int        i, j, pix_width, grey;
	
	g_return_val_if_fail(src != NULL, NULL);
	g_return_val_if_fail(dest != NULL, NULL);
	
	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	s_has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);

	d_width = gdk_pixbuf_get_width(dest);
	d_height = gdk_pixbuf_get_height(dest);
	d_has_alpha = gdk_pixbuf_get_has_alpha(dest);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);
	
	g_return_val_if_fail(d_width == s_width, NULL);
	g_return_val_if_fail(d_height == s_height, NULL);
	g_return_val_if_fail(d_has_alpha == s_has_alpha, NULL);
	
	pix_width = (s_has_alpha ? 4 : 3);

	for (i = 0; i < s_height; i++) {
		sp = s_pix;
		dp = d_pix;
		
		for (j = 0; j < s_width; j++) {
			grey = GO_RGB_TO_GREY(sp[0], sp[1], sp[2]);
			
			dp[0] = pu_clamp(grey + r, 0, 255);	/* red */
			dp[1] = pu_clamp(grey + g, 0, 255);	/* green */
			dp[2] = pu_clamp(grey + b, 0, 255);	/* blue */
			
			if (s_has_alpha) 
			{
				dp[3] = sp[3];	/* alpha */
			}

			dp += pix_width;
			sp += pix_width;
		}
		
		d_pix += d_rowstride;
		s_pix += s_rowstride;
	}

	return dest;
}

static GdkPixbuf *pixbuf_soften_edges(GdkPixbuf *dest, int size)
{
	int        d_has_alpha;
	int        d_width, d_height, d_rowstride;
        guchar    *d_pix, *dp;
	int        i, j, pix_width;
	
	g_return_val_if_fail(dest != NULL, NULL);
	
	d_width = gdk_pixbuf_get_width(dest);
	d_height = gdk_pixbuf_get_height(dest);
	d_has_alpha = gdk_pixbuf_get_has_alpha(dest);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);

	g_return_val_if_fail(d_has_alpha, NULL);
	
	pix_width = (d_has_alpha ? 4 : 3);

	for (i = 0; i < MIN(size,d_height); i++) {
		dp = d_pix + (i * d_rowstride);
		//pix = (pixel_t)dp;
		for (j = 0; j < d_width; j++) {
			dp[3] = (dp[3] * i) / size; /* alpha */
			dp += pix_width;
		}
				
		dp = d_pix + ((d_height - i - 1) * d_rowstride);
		for (j = 0; j < d_width; j++) {
			dp[3] = (dp[3] * i) / size; /* alpha */
			dp += pix_width;
		}
		
	}
	for (j = 0; j < d_height; j++) {
		//pix = (pixel_t)GINT_TO_POINTER(GPOINTER_TO_INT(d_pix) + (j * d_rowstride));
		dp = d_pix + ((d_height - i - 1) * d_rowstride);
		for (i = 0; i < MIN(size, d_width); i++) {
			dp[3] = (dp[3] * i) / size; /* alpha */
			dp += pix_width;
		}
		
		dp = d_pix + (j * d_rowstride) + (pix_width * d_width);
		for (i = 0; i < MIN(size, d_width); i++) {
			dp[3] = (dp[3] * i) / size; /* alpha */
			dp -= pix_width;
		}
	}

	return dest;
}



/*
GdkPixbuf *pixbuf_op(GdkPixbuf *src, GdkPixbuf *dest, 
*/

static GdkPixbuf *pixbuf_perspect_v(GdkPixbuf *src, int top_x1, int top_x2, int bot_x1, int bot_x2)
{
	int        s_width, s_height, s_rowstride;
	int        d_width, d_height, d_rowstride;
	guchar    *s_pix;
        guchar    *d_pix;
	guchar    *sp;
        guchar    *dp;
	gboolean   has_alpha;
	GdkPixbuf *dest;
	int        min_x, max_x, top_width, bot_width;
	int        x,y,alpha,channels,s_channels;
	int        row_width;
	double     row_offset, row_end;
	double     row_offset2, row_end2;
	double     left_theta, right_theta;
	pixel_t    spx[3];
	double     ex; 
	
	s_width = gdk_pixbuf_get_width(src);
	s_height = gdk_pixbuf_get_height(src);
	has_alpha = gdk_pixbuf_get_has_alpha(src);
	s_rowstride = gdk_pixbuf_get_rowstride(src);
	s_pix = gdk_pixbuf_get_pixels(src);
	
	
	g_return_val_if_fail((top_x2 - top_x1) <= s_width, NULL);
	g_return_val_if_fail((bot_x2 - bot_x1) <= s_width, NULL);
	
	d_width = s_width;
	d_height = s_height;
	dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, d_width, d_height);
	d_rowstride = gdk_pixbuf_get_rowstride(dest);
	d_pix = gdk_pixbuf_get_pixels(dest);
	
	// Shortcut ... should be harmless?
	// We should really only touch info
	// Within the range d_pix + (d_rowstride * y) + ((has_alpha ? 4 : 3) * x)
	bzero(d_pix, (d_rowstride * d_height));
	
	top_width = top_x2 - top_x1;
	bot_width = bot_x2 - bot_x1;
	
	min_x = MIN(top_x1, bot_x1);
	max_x = MAX(top_x2, bot_x2);
	
	s_channels = has_alpha ? 4 : 3;
	channels = 4;
	
	left_theta = ((double)(min_x == bot_x1 ? top_x1 : bot_x1) - min_x) / (double)d_height;
	right_theta = ((double)max_x - ((max_x == bot_x2) ? top_x2 : bot_x2)) / (double)d_height;
	
	
	for (y = 0; y < d_height; y++) {
		sp = s_pix + (y * s_rowstride);
		
		row_offset = min_x;
		row_end = max_x;
		
		row_offset2 = min_x;
		row_end2 = max_x;
		
		row_offset += (left_theta * ((min_x == bot_x1) ? (d_height - y - 1) : y));
		row_end -= (right_theta * ((max_x == bot_x2) ? (d_height - y - 1) : y));
		
		row_offset2 += (left_theta * ((min_x == bot_x1) ? (d_height - y) : y+1));
		row_end2 -= (right_theta * ((max_x == bot_x2) ? (d_height - y) : y+1));
		
		row_width = ceil(row_end) - floor(row_offset);
		
		dp = d_pix + (y * d_rowstride) + ((int)floor(row_offset) * channels);
		
		for (x = (int)floor(row_offset); x < (int)ceil(row_end); x++)
		{
			ex = x - floor(row_offset);
			
			if ((x < ceil(row_offset)))
			{
				alpha = ((int) ((double)255 * (ceil(row_offset) - row_offset)));
				alpha = MAX(alpha,0);
			}
			else if ((x >= floor(row_end)))
			{
				alpha = ((int) ((double)255 * (row_end - floor(row_end))));
				alpha = MAX(alpha,0);
			}
			else
			{
				alpha = 0xff;
			}
			
			spx[0] = PIXEL(sp, s_channels, (int)floor(((ex - 1) * (double)s_width) / (double)row_width));
			spx[1] = PIXEL(sp, s_channels, (int)floor((ex * (double)s_width) / (double)row_width));
			spx[2] = PIXEL(sp, s_channels, (int)floor(((ex + 1) * (double)s_width) / (double)row_width));
			
			*(dp++) = ((spx[1]->r * 14) + ((spx[0]->r + spx[2]->r))) >> 4; /* red */
			*(dp++) = ((spx[1]->g * 14) + ((spx[0]->g + spx[2]->g))) >> 4; /* green */
			*(dp++) = ((spx[1]->b * 14) + ((spx[0]->b + spx[2]->b))) >> 4; /* blue */
			if (has_alpha)
				*(dp++) = sqrt(alpha * (((spx[1]->a * 14) + ((spx[0]->a + spx[2]->a))) >> 4));	/* alpha */
			else
				*(dp++) = alpha;	/* alpha */
		}
		
	}
	
	return dest;
}

static VALUE pixbuf_mask_area(GdkPixbuf *mask, double cutoff)
{
	long int        x0 = 0, x1 = 0, y0 = 0,  y1=0;
	long int        m_has_alpha;
	long int        m_width, m_height, m_rowstride;
	guchar    *m_pix, *mp;
	long int        y, x;
	long       pix_width;
	volatile 
	double     grey;
	
	g_return_val_if_fail(mask != NULL, Qnil);
	
	m_width = gdk_pixbuf_get_width(mask);
	m_height = gdk_pixbuf_get_height(mask);
	m_has_alpha = gdk_pixbuf_get_has_alpha(mask);
	m_rowstride = gdk_pixbuf_get_rowstride(mask);
	m_pix = gdk_pixbuf_get_pixels(mask);
	
	pix_width = (m_has_alpha ? 4 : 3);

	
	x1 = m_width;
	y1 = m_height;
	
	for (y = 0; y < m_height; y++)
	{
		mp = m_pix;
		for (x = 0; x < m_width; x++) 
		{
			grey = GIMP_RGB_TO_GREY(mp[0], mp[1], mp[2]);
			if (grey < cutoff)// ie. darker than cutoff
			{
				if (x0 == 0)
				{
					x0 = MAX(x0, x);
					x1 = MIN(x1, x);
				}
				else
				{
					x0 = MIN(x0, x);
					x1 = MAX(x1, x);
				}
					
				if (y0 == 0)
				{
					y0 = MAX(y0, y);
					y1 = MIN(y1, y);
				}
				else
				{
					y0 = MIN(y0, y);
					y1 = MAX(y1, y);
				}
			}
			
			mp += pix_width;
		}
		m_pix += m_rowstride;
	}
	
	return rb_ary_new3(4, INT2NUM(x0), INT2NUM(y0), INT2NUM(x1), INT2NUM(y1));
}

static inline VALUE 
unref_pixbuf(GdkPixbuf *pixbuf)
{
	volatile VALUE pb = Qnil;
	
	pb = GOBJ2RVAL(pixbuf);
	
	g_object_unref(pixbuf);
	
	return pb;
}


/* Code */
static VALUE
PixbufUtils_CLASS_remove_alpha(VALUE self, VALUE __v_src, VALUE __v_col)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  GdkColor * col; GdkColor * __orig_col;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_col = col = RVAL2BOXED(__v_col, GDK_TYPE_COLOR);

#line 1360 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

  do {
  GdkPixbuf * dest  ;
 uint w,h, pixcol;
  IGNORE(self);
  pixcol = (((col->red >> 8)&0xff) << 24) | (((col->green >> 8)&0xff) << 16) | (((col->blue >> 8)&0xff) << 8) | 0xFF;
  w=gdk_pixbuf_get_width(src);
  h=gdk_pixbuf_get_height(src);
  dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, w, h);
  if(!dest) rb_raise(rb_eNoMemError, "Unable to allocate memory for pixbuf");
  gdk_pixbuf_fill(dest, pixcol);
  gdk_pixbuf_composite(src, dest, 0, 0, w, h, 0, 0, 1.0, 1.0, GDK_INTERP_NEAREST, 0xff);
  do { __p_retval = unref_pixbuf((dest)); goto out; } while(0);

  } while(0);

out:
;
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_sharpen(VALUE self, VALUE __v_src, VALUE __v_radius)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int radius; int __orig_radius;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_radius = radius = NUM2INT(__v_radius);

#line 1381 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_sharpen(src, radius))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_extract_alpha(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval = Qnil;
  VALUE __v_src = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  VALUE __v_cutoff = Qnil;
  int cutoff; int __orig_cutoff;
  VALUE __v_force_2bit = Qnil;
  bool force_2bit; bool __orig_force_2bit;

  /* Scan arguments */
  rb_scan_args(__p_argc, __p_argv, "12",&__v_src, &__v_cutoff, &__v_force_2bit);

  /* Set defaults */
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

  if (__p_argc > 1)
    __orig_cutoff = cutoff = NUM2INT(__v_cutoff);
  else
    cutoff = 127;

  if (__p_argc > 2)
    __orig_force_2bit = force_2bit = RTEST(__v_force_2bit);
  else
    force_2bit = FALSE;


#line 1385 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_extract_alpha(src, cutoff, force_2bit))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_blur(VALUE self, VALUE __v_src, VALUE __v_radius)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int radius; int __orig_radius;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_radius = radius = NUM2INT(__v_radius);

#line 1389 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_blur(gdk_pixbuf_copy(src), radius))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_90(VALUE self, VALUE __v_src, VALUE __v_counter_clockwise)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  bool counter_clockwise; bool __orig_counter_clockwise;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_counter_clockwise = counter_clockwise = RTEST(__v_counter_clockwise);

#line 1393 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, counter_clockwise ? ANGLE_270 : ANGLE_90))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_cw(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1397 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, ANGLE_90))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_180(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1401 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, ANGLE_180))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate(VALUE self, VALUE __v_src, VALUE __v_angle)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int angle; int __orig_angle;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_angle = angle = NUM2INT(__v_angle);

#line 1405 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  g_assert(angle == 0 || angle == 90 || angle == 180 || angle == 270);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, (rotate_angle_t)angle))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_ccw(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1410 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, ANGLE_270))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_gamma_pling(VALUE self, VALUE __v_src, VALUE __v_level)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  double level; double __orig_level;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_level = level = NUM2DBL(__v_level);

#line 1414 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_gamma(src, src, level)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_soften_edges_pling(VALUE self, VALUE __v_src, VALUE __v_size)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int size; int __orig_size;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_size = size = NUM2INT(__v_size);

#line 1418 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_soften_edges(src, size)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_gamma(VALUE self, VALUE __v_src, VALUE __v_level)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  double level; double __orig_level;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_level = level = NUM2DBL(__v_level);

#line 1422 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_gamma(src, gdk_pixbuf_copy(src), level))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale_pling(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1426 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_greyscale(src, src)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_to_tiff(VALUE self, VALUE __v_src, VALUE __v_filename)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  char * filename; char * __orig_filename;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_filename = filename = ( NIL_P(__v_filename) ? NULL : StringValuePtr(__v_filename) );

#line 1430 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval =  ((pixbuf_save_tiff(src, filename)) ? Qtrue : Qfalse); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1434 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_greyscale(src, gdk_pixbuf_copy(src)))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale_go_pling(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1438 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_greyscale_go(src, src)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale_go(VALUE self, VALUE __v_src)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 1442 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_greyscale_go(src, gdk_pixbuf_copy(src)))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_tint(VALUE self, VALUE __v_src, VALUE __v_r, VALUE __v_g, VALUE __v_b)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int r; int __orig_r;
  int g; int __orig_g;
  int b; int __orig_b;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_r = r = NUM2INT(__v_r);
  __orig_g = g = NUM2INT(__v_g);
  __orig_b = b = NUM2INT(__v_b);

#line 1446 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_tint(src, gdk_pixbuf_copy(src), r, g, b))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_perspect_v(VALUE self, VALUE __v_src, VALUE __v_top_x1, VALUE __v_top_x2, VALUE __v_bot_x1, VALUE __v_bot_x2)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int top_x1; int __orig_top_x1;
  int top_x2; int __orig_top_x2;
  int bot_x1; int __orig_bot_x1;
  int bot_x2; int __orig_bot_x2;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_top_x1 = top_x1 = NUM2INT(__v_top_x1);
  __orig_top_x2 = top_x2 = NUM2INT(__v_top_x2);
  __orig_bot_x1 = bot_x1 = NUM2INT(__v_bot_x1);
  __orig_bot_x2 = bot_x2 = NUM2INT(__v_bot_x2);

#line 1450 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_perspect_v(src, top_x1, top_x2, bot_x1, bot_x2))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_mask(VALUE self, VALUE __v_src, VALUE __v_mask)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  GdkPixbuf * mask; GdkPixbuf * __orig_mask;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_mask = mask = GDK_PIXBUF(RVAL2GOBJ(__v_mask));

#line 1454 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_mask(src, mask))); goto out; } while(0);
out:
;
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_blend5050(VALUE self, VALUE __v_src1, VALUE __v_src2)
{
  VALUE __p_retval = Qnil;
  GdkPixbuf * src1; GdkPixbuf * __orig_src1;
  GdkPixbuf * src2; GdkPixbuf * __orig_src2;
  __orig_src1 = src1 = GDK_PIXBUF(RVAL2GOBJ(__v_src1));
  __orig_src2 = src2 = GDK_PIXBUF(RVAL2GOBJ(__v_src2));

#line 1458 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_blend5050(src1, src2))); goto out; } while(0);
out:
;
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_mask_area(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval = Qnil;
  VALUE __v_mask = Qnil;
  GdkPixbuf * mask; GdkPixbuf * __orig_mask;
  VALUE __v_cutoff = Qnil;
  int cutoff; int __orig_cutoff;

  /* Scan arguments */
  rb_scan_args(__p_argc, __p_argv, "11",&__v_mask, &__v_cutoff);

  /* Set defaults */
  __orig_mask = mask = GDK_PIXBUF(RVAL2GOBJ(__v_mask));

  if (__p_argc > 1)
    __orig_cutoff = cutoff = NUM2INT(__v_cutoff);
  else
    cutoff = 127;


#line 1462 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = pixbuf_mask_area(mask, cutoff); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_scale_max(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval = Qnil;
  VALUE __v_src = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  VALUE __v_max = Qnil;
  gulong max; gulong __orig_max;
  VALUE __v_interp = Qnil;
  GdkInterpType interp; GdkInterpType __orig_interp;

  /* Scan arguments */
  rb_scan_args(__p_argc, __p_argv, "21",&__v_src, &__v_max, &__v_interp);

  /* Set defaults */
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

  __orig_max = max = NUM2UINT(__v_max);

  if (__p_argc > 2)
    __orig_interp = interp = RVAL2GENUM(__v_interp, GDK_TYPE_INTERP_TYPE);
  else
    interp = GDK_INTERP_BILINEAR;


#line 1466 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

  do {
  gulong width, height, largest;
  gdouble  scale  ;
 IGNORE(self);
  width = gdk_pixbuf_get_width(src);
  height = gdk_pixbuf_get_height(src);
  largest = MAX(width, height);
  if (max > largest) { do { __p_retval = unref_pixbuf((gdk_pixbuf_copy(src))); goto out; } while(0); } scale = (double)(max) / (double)(largest);
  do { __p_retval = unref_pixbuf((gdk_pixbuf_scale_simple(src, (int)(scale * width), (int)(scale * height), interp))); goto out; } while(0);

  } while(0);

out:
;
;
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_draw_scaled(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval = Qnil;
  VALUE __v_drawable = Qnil;
  GdkDrawable * drawable; GdkDrawable * __orig_drawable;
  VALUE __v_src = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  VALUE __v_x = Qnil;
  int x; int __orig_x;
  VALUE __v_y = Qnil;
  int y; int __orig_y;
  VALUE __v_width = Qnil;
  int width; int __orig_width;
  VALUE __v_height = Qnil;
  int height; int __orig_height;
  VALUE __v_interp = Qnil;
  GdkInterpType interp; GdkInterpType __orig_interp;

  /* Scan arguments */
  rb_scan_args(__p_argc, __p_argv, "61",&__v_drawable, &__v_src, &__v_x, &__v_y, &__v_width, &__v_height, &__v_interp);

  /* Set defaults */
  __orig_drawable = drawable = RVAL2GOBJ(__v_drawable);

  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

  __orig_x = x = NUM2INT(__v_x);

  __orig_y = y = NUM2INT(__v_y);

  __orig_width = width = NUM2INT(__v_width);

  __orig_height = height = NUM2INT(__v_height);

  if (__p_argc > 6)
    __orig_interp = interp = RVAL2GENUM(__v_interp, GDK_TYPE_INTERP_TYPE);
  else
    interp = GDK_INTERP_BILINEAR;


#line 1484 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

  do {
  GdkPixbuf * tmp  ;
 IGNORE(self);
  if((width == gdk_pixbuf_get_width(src)) && (height == gdk_pixbuf_get_height(src))) { tmp = src;
  } else if (((tmp = (GdkPixbuf*)g_object_get_data(G_OBJECT(src), "pixbuf_utils_scaled")) == NULL) || ((width != gdk_pixbuf_get_width(tmp)) || (height != gdk_pixbuf_get_height(tmp)))) { if (tmp) fprintf(stderr, "Old pixbuf is %i,%i\n", gdk_pixbuf_get_width(tmp), gdk_pixbuf_get_height(tmp));
  fprintf(stderr, "Scaling pixbuf to %i,%i\n", width, height);
  tmp = gdk_pixbuf_scale_simple(src, width, height, interp);
  g_object_set_data_full(G_OBJECT(src), "pixbuf_utils_scaled", tmp, g_object_unref);
  } g_return_val_if_fail(tmp != NULL, Qfalse);
  gdk_draw_pixbuf(/* GdkDrawable *drawable */ drawable, /* GdkGC *gc */ NULL, /* GdkPixbuf *pixbuf */ tmp, /* gint src_x */ 0, /* gint src_y */ 0, /* gint dest_x */ x, /* gint dest_y */ y, /* gint width */ width, /* gint height */ height, /* GdkRgbDither dither */ GDK_RGB_DITHER_NORMAL, /* gint x_dither */ 0, /* gint y_dither */ 0);
  do { __p_retval = Qtrue; goto out; } while(0);

  } while(0);

out:
;
;
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_draw_scaled_clip(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval = Qnil;
  VALUE __v_drawable = Qnil;
  GdkDrawable * drawable; GdkDrawable * __orig_drawable;
  VALUE __v_src = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  VALUE __v_x = Qnil;
  int x; int __orig_x;
  VALUE __v_y = Qnil;
  int y; int __orig_y;
  VALUE __v_width = Qnil;
  int width; int __orig_width;
  VALUE __v_height = Qnil;
  int height; int __orig_height;
  VALUE __v_clip_area = Qnil;
  GdkRectangle * clip_area; GdkRectangle * __orig_clip_area;
  VALUE __v_interp = Qnil;
  GdkInterpType interp; GdkInterpType __orig_interp;

  /* Scan arguments */
  rb_scan_args(__p_argc, __p_argv, "71",&__v_drawable, &__v_src, &__v_x, &__v_y, &__v_width, &__v_height, &__v_clip_area, &__v_interp);

  /* Set defaults */
  __orig_drawable = drawable = RVAL2GOBJ(__v_drawable);

  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

  __orig_x = x = NUM2INT(__v_x);

  __orig_y = y = NUM2INT(__v_y);

  __orig_width = width = NUM2INT(__v_width);

  __orig_height = height = NUM2INT(__v_height);

  __orig_clip_area = clip_area = RVAL2BOXED(__v_clip_area, GDK_TYPE_RECTANGLE);

  if (__p_argc > 7)
    __orig_interp = interp = RVAL2GENUM(__v_interp, GDK_TYPE_INTERP_TYPE);
  else
    interp = GDK_INTERP_BILINEAR;


#line 1520 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

  do {
  GdkPixbuf * tmp  ;
 GdkGC * gc  ;
 IGNORE(self);
  if((width == gdk_pixbuf_get_width(src)) && (height == gdk_pixbuf_get_height(src))) { tmp = src;
  } else if (((tmp = (GdkPixbuf*)g_object_get_data(G_OBJECT(src), "pixbuf_utils_scaled")) == NULL) || ((width != gdk_pixbuf_get_width(tmp)) || (height != gdk_pixbuf_get_height(tmp)))) { if (tmp) fprintf(stderr, "Old pixbuf is %i,%i\n", gdk_pixbuf_get_width(tmp), gdk_pixbuf_get_height(tmp));
  fprintf(stderr, "Scaling pixbuf to %i,%i\n", width, height);
  tmp = gdk_pixbuf_scale_simple(src, width, height, interp);
  g_object_set_data_full(G_OBJECT(src), "pixbuf_utils_scaled", tmp, g_object_unref);
  } g_return_val_if_fail(tmp != NULL, Qfalse);
  gc = gdk_gc_new(drawable);
  gdk_gc_set_clip_rectangle(GDK_GC(gc), clip_area);
  gdk_draw_pixbuf(/* GdkDrawable *drawable */ drawable, /* GdkGC *gc */ gc, /* GdkPixbuf *pixbuf */ tmp, /* gint src_x */ clip_area->x, /* gint src_y */ clip_area->y, /* gint dest_x */ x + clip_area->x, /* gint dest_y */ y + clip_area->y, /* gint width */ clip_area->width, /* gint height */ clip_area->height, /* GdkRgbDither dither */ GDK_RGB_DITHER_NORMAL, /* gint x_dither */ 0, /* gint y_dither */ 0);
  g_object_unref(gc);
  do { __p_retval = Qtrue; goto out; } while(0);

  } while(0);

out:
;
;
;
;
  return __p_retval;
}

/* Init */
void
Init_pixbufutils(void)
{
  mPixbufUtils = rb_define_module("PixbufUtils");
  rb_define_singleton_method(mPixbufUtils, "remove_alpha", PixbufUtils_CLASS_remove_alpha, 2);
  rb_define_singleton_method(mPixbufUtils, "sharpen", PixbufUtils_CLASS_sharpen, 2);
  rb_define_singleton_method(mPixbufUtils, "extract_alpha", PixbufUtils_CLASS_extract_alpha, -1);
  rb_define_singleton_method(mPixbufUtils, "blur", PixbufUtils_CLASS_blur, 2);
  rb_define_singleton_method(mPixbufUtils, "rotate_90", PixbufUtils_CLASS_rotate_90, 2);
  rb_define_singleton_method(mPixbufUtils, "rotate_cw", PixbufUtils_CLASS_rotate_cw, 1);
  rb_define_singleton_method(mPixbufUtils, "rotate_180", PixbufUtils_CLASS_rotate_180, 1);
  rb_define_singleton_method(mPixbufUtils, "rotate", PixbufUtils_CLASS_rotate, 2);
  rb_define_singleton_method(mPixbufUtils, "rotate_ccw", PixbufUtils_CLASS_rotate_ccw, 1);
  rb_define_singleton_method(mPixbufUtils, "gamma!", PixbufUtils_CLASS_gamma_pling, 2);
  rb_define_singleton_method(mPixbufUtils, "soften_edges!", PixbufUtils_CLASS_soften_edges_pling, 2);
  rb_define_singleton_method(mPixbufUtils, "gamma", PixbufUtils_CLASS_gamma, 2);
  rb_define_singleton_method(mPixbufUtils, "greyscale!", PixbufUtils_CLASS_greyscale_pling, 1);
  rb_define_singleton_method(mPixbufUtils, "to_tiff", PixbufUtils_CLASS_to_tiff, 2);
  rb_define_singleton_method(mPixbufUtils, "greyscale", PixbufUtils_CLASS_greyscale, 1);
  rb_define_singleton_method(mPixbufUtils, "greyscale_go!", PixbufUtils_CLASS_greyscale_go_pling, 1);
  rb_define_singleton_method(mPixbufUtils, "greyscale_go", PixbufUtils_CLASS_greyscale_go, 1);
  rb_define_singleton_method(mPixbufUtils, "tint", PixbufUtils_CLASS_tint, 4);
  rb_define_singleton_method(mPixbufUtils, "perspect_v", PixbufUtils_CLASS_perspect_v, 5);
  rb_define_singleton_method(mPixbufUtils, "mask", PixbufUtils_CLASS_mask, 2);
  rb_define_singleton_method(mPixbufUtils, "blend5050", PixbufUtils_CLASS_blend5050, 2);
  rb_define_singleton_method(mPixbufUtils, "mask_area", PixbufUtils_CLASS_mask_area, -1);
  rb_define_singleton_method(mPixbufUtils, "scale_max", PixbufUtils_CLASS_scale_max, -1);
  rb_define_singleton_method(mPixbufUtils, "draw_scaled", PixbufUtils_CLASS_draw_scaled, -1);
  rb_define_singleton_method(mPixbufUtils, "draw_scaled_clip", PixbufUtils_CLASS_draw_scaled_clip, -1);
}