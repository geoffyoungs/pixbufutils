#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  /* Includes */
  #include <ruby.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #if defined GCC
    #define OPTIONAL_ATTR __attribute__((unused))
  #else
    #define OPTIONAL_ATTR
  #endif
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
PixbufUtils_CLASS_remove_alpha(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_col OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_sharpen(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_radius OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_extract_alpha(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_blur(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_radius OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_contrast(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_vibrance(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_saturation(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_brightness(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_filter(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE filter OPTIONAL_ATTR, VALUE __v_divisor OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_rotate_90(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_counter_clockwise OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_rotate_cw(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_rotate_180(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_rotate(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_angle OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_rotate_ccw(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_gamma_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_level OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_soften_edges_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_size OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_gamma(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_level OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_greyscale_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_to_tiff(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_filename OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_greyscale(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_greyscale_go_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_greyscale_go(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_tint(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_perspect_v(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_top_x1 OPTIONAL_ATTR, VALUE __v_top_x2 OPTIONAL_ATTR, VALUE __v_bot_x1 OPTIONAL_ATTR, VALUE __v_bot_x2 OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_mask(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_mask OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_auto_equalize(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_blend5050(VALUE self OPTIONAL_ATTR , VALUE __v_src1 OPTIONAL_ATTR, VALUE __v_src2 OPTIONAL_ATTR);
static VALUE
PixbufUtils_CLASS_mask_area(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_scale_max(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_draw_scaled(int __p_argc, VALUE *__p_argv, VALUE self);
static VALUE
PixbufUtils_CLASS_draw_scaled_clip(int __p_argc, VALUE *__p_argv, VALUE self);

/* Inline C code */


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
#include "filter.h"


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
PixbufUtils_CLASS_remove_alpha(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_col OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  GdkColor * col; GdkColor * __orig_col;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_col = col = RVAL2BOXED(__v_col, GDK_TYPE_COLOR);

#line 259 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

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
PixbufUtils_CLASS_sharpen(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_radius OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int radius; int __orig_radius;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_radius = radius = NUM2INT(__v_radius);

#line 280 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_sharpen(src, radius))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_extract_alpha(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
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


#line 284 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_extract_alpha(src, cutoff, force_2bit))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_blur(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_radius OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int radius; int __orig_radius;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_radius = radius = NUM2INT(__v_radius);

#line 288 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_blur(gdk_pixbuf_copy(src), radius))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_contrast(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int adjust; int __orig_adjust;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_adjust = adjust = NUM2INT(__v_adjust);

#line 292 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_adjust_contrast(src, gdk_pixbuf_copy(src), adjust))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_vibrance(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int adjust; int __orig_adjust;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_adjust = adjust = NUM2INT(__v_adjust);

#line 296 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_adjust_vibrance(src, gdk_pixbuf_copy(src), adjust))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_saturation(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int adjust; int __orig_adjust;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_adjust = adjust = NUM2INT(__v_adjust);

#line 300 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_adjust_saturation(src, gdk_pixbuf_copy(src), adjust))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_brightness(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_adjust OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int adjust; int __orig_adjust;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_adjust = adjust = NUM2INT(__v_adjust);

#line 304 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_adjust_brightness(src, gdk_pixbuf_copy(src), adjust))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_filter(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE filter OPTIONAL_ATTR, VALUE __v_divisor OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  double divisor; double __orig_divisor;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  Check_Type(filter, T_ARRAY);
  __orig_divisor = divisor = NUM2DBL(__v_divisor);

#line 308 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

  do {
  int matrix_size = RARRAY_LEN(filter), len, i;
  double * matrix  ;
 IGNORE(self);
  len = (int) sqrt((double)matrix_size);
  if ((len * len) != matrix_size) { rb_raise(rb_eArgError, "Invalid matrix size - sqrt(%i)*sqrt(%i) != %i", matrix_size, matrix_size, matrix_size);
  } matrix = ALLOCA_N(double, matrix_size);
  for (i = 0;
  i < matrix_size;
  i ++) { matrix[i] = NUM2DBL(RARRAY_PTR(filter)[i]);
  } do { __p_retval = unref_pixbuf((pixbuf_convolution_matrix(src, gdk_pixbuf_copy(src), len, matrix, divisor))); goto out; } while(0);

  } while(0);

out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_90(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_counter_clockwise OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  bool counter_clockwise; bool __orig_counter_clockwise;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_counter_clockwise = counter_clockwise = RTEST(__v_counter_clockwise);

#line 326 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, counter_clockwise ? ANGLE_270 : ANGLE_90))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_cw(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 330 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, ANGLE_90))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_180(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 334 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, ANGLE_180))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_angle OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int angle; int __orig_angle;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_angle = angle = NUM2INT(__v_angle);

#line 338 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  g_assert(angle == 0 || angle == 90 || angle == 180 || angle == 270);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, (rotate_angle_t)angle))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_rotate_ccw(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 343 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_rotate(src, ANGLE_270))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_gamma_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_level OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  double level; double __orig_level;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_level = level = NUM2DBL(__v_level);

#line 347 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_gamma(src, src, level)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_soften_edges_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_size OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  int size; int __orig_size;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_size = size = NUM2INT(__v_size);

#line 351 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_soften_edges(src, size)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_gamma(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_level OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  double level; double __orig_level;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_level = level = NUM2DBL(__v_level);

#line 355 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_gamma(src, gdk_pixbuf_copy(src), level))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 359 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_greyscale(src, src)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_to_tiff(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_filename OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  char * filename; char * __orig_filename;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_filename = filename = ( NIL_P(__v_filename) ? NULL : StringValuePtr(__v_filename) );

#line 363 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval =  ((pixbuf_save_tiff(src, filename)) ? Qtrue : Qfalse); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 367 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_greyscale(src, gdk_pixbuf_copy(src)))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale_go_pling(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 371 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = GOBJ2RVAL(pixbuf_greyscale_go(src, src)); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_greyscale_go(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 375 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_greyscale_go(src, gdk_pixbuf_copy(src)))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_tint(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  VALUE __v_src = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  VALUE __v_r = Qnil;
  int r; int __orig_r;
  VALUE __v_g = Qnil;
  int g; int __orig_g;
  VALUE __v_b = Qnil;
  int b; int __orig_b;
  VALUE __v_alpha = Qnil;
  int alpha; int __orig_alpha;

  /* Scan arguments */
  rb_scan_args(__p_argc, __p_argv, "41",&__v_src, &__v_r, &__v_g, &__v_b, &__v_alpha);

  /* Set defaults */
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

  __orig_r = r = NUM2INT(__v_r);

  __orig_g = g = NUM2INT(__v_g);

  __orig_b = b = NUM2INT(__v_b);

  if (__p_argc > 4)
    __orig_alpha = alpha = NUM2INT(__v_alpha);
  else
    alpha = 255;


#line 379 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_tint(src, gdk_pixbuf_copy(src), r, g, b, alpha))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_perspect_v(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_top_x1 OPTIONAL_ATTR, VALUE __v_top_x2 OPTIONAL_ATTR, VALUE __v_bot_x1 OPTIONAL_ATTR, VALUE __v_bot_x2 OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
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

#line 383 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_perspect_v(src, top_x1, top_x2, bot_x1, bot_x2))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_mask(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR, VALUE __v_mask OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  GdkPixbuf * mask; GdkPixbuf * __orig_mask;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));
  __orig_mask = mask = GDK_PIXBUF(RVAL2GOBJ(__v_mask));

#line 387 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((pixbuf_mask(src, mask))); goto out; } while(0);
out:
;
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_auto_equalize(VALUE self OPTIONAL_ATTR , VALUE __v_src OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src; GdkPixbuf * __orig_src;
  __orig_src = src = GDK_PIXBUF(RVAL2GOBJ(__v_src));

#line 391 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = unref_pixbuf((auto_equalize(src, gdk_pixbuf_copy(src)))); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_blend5050(VALUE self OPTIONAL_ATTR , VALUE __v_src1 OPTIONAL_ATTR, VALUE __v_src2 OPTIONAL_ATTR)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
  GdkPixbuf * src1; GdkPixbuf * __orig_src1;
  GdkPixbuf * src2; GdkPixbuf * __orig_src2;
  __orig_src1 = src1 = GDK_PIXBUF(RVAL2GOBJ(__v_src1));
  __orig_src2 = src2 = GDK_PIXBUF(RVAL2GOBJ(__v_src2));

#line 395 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
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
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
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


#line 399 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"
  IGNORE(self);
  do { __p_retval = pixbuf_mask_area(mask, cutoff); goto out; } while(0);
out:
;
  return __p_retval;
}

static VALUE
PixbufUtils_CLASS_scale_max(int __p_argc, VALUE *__p_argv, VALUE self)
{
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
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


#line 403 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

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
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
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


#line 421 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

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
  VALUE __p_retval OPTIONAL_ATTR = Qnil;
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


#line 457 "/home/geoff/Projects/pixbufutils/ext/pixbufutils/pixbufutils.cr"

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
  rb_define_singleton_method(mPixbufUtils, "contrast", PixbufUtils_CLASS_contrast, 2);
  rb_define_singleton_method(mPixbufUtils, "vibrance", PixbufUtils_CLASS_vibrance, 2);
  rb_define_singleton_method(mPixbufUtils, "saturation", PixbufUtils_CLASS_saturation, 2);
  rb_define_singleton_method(mPixbufUtils, "brightness", PixbufUtils_CLASS_brightness, 2);
  rb_define_singleton_method(mPixbufUtils, "filter", PixbufUtils_CLASS_filter, 3);
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
  rb_define_singleton_method(mPixbufUtils, "tint", PixbufUtils_CLASS_tint, -1);
  rb_define_singleton_method(mPixbufUtils, "perspect_v", PixbufUtils_CLASS_perspect_v, 5);
  rb_define_singleton_method(mPixbufUtils, "mask", PixbufUtils_CLASS_mask, 2);
  rb_define_singleton_method(mPixbufUtils, "auto_equalize", PixbufUtils_CLASS_auto_equalize, 1);
  rb_define_singleton_method(mPixbufUtils, "blend5050", PixbufUtils_CLASS_blend5050, 2);
  rb_define_singleton_method(mPixbufUtils, "mask_area", PixbufUtils_CLASS_mask_area, -1);
  rb_define_singleton_method(mPixbufUtils, "scale_max", PixbufUtils_CLASS_scale_max, -1);
  rb_define_singleton_method(mPixbufUtils, "draw_scaled", PixbufUtils_CLASS_draw_scaled, -1);
  rb_define_singleton_method(mPixbufUtils, "draw_scaled_clip", PixbufUtils_CLASS_draw_scaled_clip, -1);
}
