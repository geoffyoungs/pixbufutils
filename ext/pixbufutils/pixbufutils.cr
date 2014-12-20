%name pixbufutils
%pkg-config gdk-pixbuf-2.0
%include gdk-pixbuf/gdk-pixbuf.h
%include rbglib.h
%include rbgobject.h
%include tiffio.h

%include gdk/gdk.h

%option gtk=no
%pkg-config gdk-2.0
%lib tiff

%map GdkPixbuf* > VALUE : GOBJ2RVAL(%%)
%map VALUE > GdkPixbuf* : GDK_PIXBUF(RVAL2GOBJ(%%))

%map GtkWidget* > VALUE : GOBJ2RVAL(%%)
%map VALUE > GtkWidget* : GTK_WIDGET(RVAL2GOBJ(%%))
%map VALUE > gulong : NUM2UINT(%%)

%{

#define PIXEL(row, channels, x)  ((pixel_t)(row + (channels * x)))

typedef struct {
unsigned char r,g,b,a;
} * pixel_t;

extern void Init_pixbufutils(void);

#ifndef IGNORE
#define IGNORE(x) x=x
#endif

#include <unistd.h>
#include <math.h>

#include "extract-alpha.h"
#include "blur.h"
#include "sharpen.h"
#include "greyscale.h"
#include "rotate.h"
#include "tiff.h"
#include "gamma.h"
#include "mask.h"
#include "blend5050.h"
#include "tint.h"
#include "soften-edges.h"
#include "histogram.h"
#include "auto-equalize.h"


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

%}

%map GdkInterpType > VALUE : GENUM2RVAL(%%, GDK_TYPE_INTERP_TYPE)
%map VALUE > GdkInterpType  : RVAL2GENUM(%%, GDK_TYPE_INTERP_TYPE)
%map GdkRectangle* > VALUE : GBOXED2RVAL(%%, GDK_TYPE_RECTANGLE)
%map unref_pixbuf > VALUE : unref_pixbuf((%%))

%map GdkColor* > VALUE : GBOXED2RVAL(%%, GDK_TYPE_COLOR)
%map VALUE > GdkColor* : RVAL2BOXED(%%, GDK_TYPE_COLOR)

%map VALUE > GdkDrawable*  : RVAL2GOBJ(%%)
%map VALUE > GdkGC*  : RVAL2BOXED(%%, GDK_TYPE_GC)
%map VALUE > GdkRectangle*  : RVAL2BOXED(%%, GDK_TYPE_RECTANGLE)

module PixbufUtils
%% module methods
	def unref_pixbuf:self.remove_alpha(GdkPixbuf *src, GdkColor *col)
		GdkPixbuf *dest;
		uint w,h, pixcol;

    IGNORE(self);
		pixcol = (((col->red >> 8)&0xff) << 24) |
			(((col->green >> 8)&0xff) << 16) |
			(((col->blue >> 8)&0xff) << 8) | 0xFF;

		w=gdk_pixbuf_get_width(src);
		h=gdk_pixbuf_get_height(src);

		dest = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, w, h);
		if(!dest)
			rb_raise(rb_eNoMemError, "Unable to allocate memory for pixbuf");

		gdk_pixbuf_fill(dest, pixcol);
		gdk_pixbuf_composite(src, dest, 0, 0, w, h, 0, 0, 1.0, 1.0, GDK_INTERP_NEAREST, 0xff);

		return dest;
	end
	def unref_pixbuf:self.sharpen(GdkPixbuf *src, int radius)
    IGNORE(self);
		return pixbuf_sharpen(src, radius);
	end
	def unref_pixbuf:self.extract_alpha(GdkPixbuf *src, int cutoff = 127, bool force_2bit = FALSE)
    IGNORE(self);
		return pixbuf_extract_alpha(src, cutoff, force_2bit);
	end
	def unref_pixbuf:self.blur(GdkPixbuf *src, int radius)
    IGNORE(self);
		return pixbuf_blur(gdk_pixbuf_copy(src), radius);
	end
	def unref_pixbuf:self.rotate_90(GdkPixbuf *src, bool counter_clockwise)
    IGNORE(self);
		return pixbuf_rotate(src, counter_clockwise ? ANGLE_270 : ANGLE_90);
	end
	def unref_pixbuf:self.rotate_cw(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_rotate(src, ANGLE_90);
	end
	def unref_pixbuf:self.rotate_180(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_rotate(src, ANGLE_180);
	end
	def unref_pixbuf:self.rotate(GdkPixbuf *src, int angle)
    IGNORE(self);
		g_assert(angle == 0 || angle == 90 || angle == 180 || angle == 270);
		return pixbuf_rotate(src, (rotate_angle_t)angle);
	end
	def unref_pixbuf:self.rotate_ccw(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_rotate(src, ANGLE_270);
	end
	def GdkPixbuf*:self.gamma!(GdkPixbuf *src, double level)
    IGNORE(self);
		return pixbuf_gamma(src, src, level);
	end
	def GdkPixbuf*:self.soften_edges!(GdkPixbuf *src, int size)
    IGNORE(self);
		return pixbuf_soften_edges(src, size);
	end
	def unref_pixbuf:self.gamma(GdkPixbuf *src, double level)
    IGNORE(self);
		return pixbuf_gamma(src, gdk_pixbuf_copy(src), level);
	end
	def GdkPixbuf*:self.greyscale!(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_greyscale(src, src);
	end
	def bool:self.to_tiff(GdkPixbuf *src, char *filename)
    IGNORE(self);
		return pixbuf_save_tiff(src, filename);
	end
	def unref_pixbuf:self.greyscale(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_greyscale(src, gdk_pixbuf_copy(src));
	end
	def GdkPixbuf*:self.greyscale_go!(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_greyscale_go(src, src);
	end
	def unref_pixbuf:self.greyscale_go(GdkPixbuf *src)
    IGNORE(self);
		return pixbuf_greyscale_go(src, gdk_pixbuf_copy(src));
	end
	def unref_pixbuf:self.tint(GdkPixbuf *src, int r, int g, int b, int alpha=255)
    IGNORE(self);
		return pixbuf_tint(src, gdk_pixbuf_copy(src), r, g, b, alpha);
	end
	def unref_pixbuf:self.perspect_v(GdkPixbuf *src, int top_x1, int top_x2, int bot_x1, int bot_x2)
    IGNORE(self);
		return pixbuf_perspect_v(src, top_x1, top_x2, bot_x1, bot_x2);
	end
	def unref_pixbuf:self.mask(GdkPixbuf *src, GdkPixbuf *mask)
    IGNORE(self);
		return pixbuf_mask(src, mask);
	end
  def unref_pixbuf:self.auto_equalize(GdkPixbuf *src)
    IGNORE(self);
    return auto_equalize(src, gdk_pixbuf_copy(src));
  end
	def unref_pixbuf:self.blend5050(GdkPixbuf *src1, GdkPixbuf *src2)
    IGNORE(self);
		return pixbuf_blend5050(src1, src2);
	end
	def self.mask_area(GdkPixbuf *mask, int cutoff = 127)
    IGNORE(self);
		return pixbuf_mask_area(mask, cutoff);
	end
	def unref_pixbuf:self.scale_max(GdkPixbuf *src, gulong max, GdkInterpType interp = GDK_INTERP_BILINEAR)
		gulong width, height, largest;
		gdouble scale;

    IGNORE(self);
		width = gdk_pixbuf_get_width(src);
		height = gdk_pixbuf_get_height(src);
		largest = MAX(width, height);
		if (max > largest)
		{
			return gdk_pixbuf_copy(src);
		}

		scale = (double)(max) / (double)(largest);

		return gdk_pixbuf_scale_simple(src, (int)(scale * width), (int)(scale * height), interp);
	end
	def self.draw_scaled(GdkDrawable *drawable, GdkPixbuf *src,  int x, int y,
		int width, int height, GdkInterpType interp = GDK_INTERP_BILINEAR)

		GdkPixbuf *tmp;


    IGNORE(self);
		if((width == gdk_pixbuf_get_width(src)) && (height == gdk_pixbuf_get_height(src)))
		{
			tmp = src;
		}
		else if (((tmp = (GdkPixbuf*)g_object_get_data(G_OBJECT(src), "pixbuf_utils_scaled")) == NULL)
			|| ((width != gdk_pixbuf_get_width(tmp)) || (height != gdk_pixbuf_get_height(tmp))))
		{
			if (tmp) fprintf(stderr, "Old pixbuf is %i,%i\n", gdk_pixbuf_get_width(tmp), gdk_pixbuf_get_height(tmp));
			fprintf(stderr, "Scaling pixbuf to %i,%i\n", width, height);
			tmp = gdk_pixbuf_scale_simple(src, width, height, interp);
			g_object_set_data_full(G_OBJECT(src), "pixbuf_utils_scaled", tmp, g_object_unref);
		}
		g_return_val_if_fail(tmp != NULL, Qfalse);

		gdk_draw_pixbuf(/* GdkDrawable *drawable */		drawable,
                                             /* GdkGC *gc */		NULL,
                                             /* GdkPixbuf *pixbuf */	tmp,
                                             /* gint src_x */		0,
                                             /* gint src_y */		0,
                                             /* gint dest_x */		x,
                                             /* gint dest_y */		y,
                                             /* gint width */		width,
                                             /* gint height */		height,
                                             /* GdkRgbDither dither */	GDK_RGB_DITHER_NORMAL,
                                             /* gint x_dither */ 	0,
                                             /* gint y_dither */ 	0);
		return Qtrue;

	end
	def self.draw_scaled_clip(GdkDrawable *drawable, GdkPixbuf *src,  int x, int y,
		int width, int height, GdkRectangle *clip_area, GdkInterpType interp = GDK_INTERP_BILINEAR)

		GdkPixbuf *tmp;
		GdkGC *gc;

    IGNORE(self);
		if((width == gdk_pixbuf_get_width(src)) && (height == gdk_pixbuf_get_height(src)))
		{
			tmp = src;
		}
		else if (((tmp = (GdkPixbuf*)g_object_get_data(G_OBJECT(src), "pixbuf_utils_scaled")) == NULL)
			|| ((width != gdk_pixbuf_get_width(tmp)) || (height != gdk_pixbuf_get_height(tmp))))
		{
			if (tmp) fprintf(stderr, "Old pixbuf is %i,%i\n", gdk_pixbuf_get_width(tmp), gdk_pixbuf_get_height(tmp));
			fprintf(stderr, "Scaling pixbuf to %i,%i\n", width, height);
				tmp = gdk_pixbuf_scale_simple(src, width, height, interp);
				g_object_set_data_full(G_OBJECT(src), "pixbuf_utils_scaled", tmp, g_object_unref);
		}
		g_return_val_if_fail(tmp != NULL, Qfalse);

		gc = gdk_gc_new(drawable);
		gdk_gc_set_clip_rectangle(GDK_GC(gc), clip_area);

		gdk_draw_pixbuf(/* GdkDrawable *drawable */		drawable,
                                             /* GdkGC *gc */		gc,
                                             /* GdkPixbuf *pixbuf */	tmp,
                                             /* gint src_x */		clip_area->x,
                                             /* gint src_y */		clip_area->y,
                                             /* gint dest_x */		x + clip_area->x,
                                             /* gint dest_y */		y + clip_area->y,
                                             /* gint width */		clip_area->width,
                                             /* gint height */		clip_area->height,
                                             /* GdkRgbDither dither */	GDK_RGB_DITHER_NORMAL,
                                             /* gint x_dither */ 	0,
                                             /* gint y_dither */ 	0);
		g_object_unref(gc);

		return Qtrue;

	end
end

