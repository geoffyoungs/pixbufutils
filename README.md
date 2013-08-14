pixbufutils
===========

gdk-pixbuf related utils for ruby


Methods:
--------

Methods that modify source pixbuf
 * PixbufUtils.greyscale!(Gdk::Pixbuf src)


 * PixbufUtils.greyscale_go!(Gdk::Pixbuf src)


 * PixbufUtils.gamma!(Gdk::Pixbuf src, double level)


 * PixbufUtils.soften_edges!(Gdk::Pixbuf src, Integer size)



Methods that return a modified copy

 * PixbufUtils.remove_alpha(Gdk::Pixbuf src, Gdk::Color col)

  returns new pixbuf with no alpha channel


 * PixbufUtils.sharpen(Gdk::Pixbuf src, Integer radius)

  returns new pixbuf


 * PixbufUtils.extract_alpha(Gdk::Pixbuf src, Integer cutoff, Boolean force_2bit)
  returns new pixbuf


 * PixbufUtils.blur(Gdk::Pixbuf src, Integer radius)
  returns new pixbuf

 * PixbufUtils.rotate_90(Gdk::Pixbuf src, Boolean counter_clockwise)


 * PixbufUtils.rotate_cw(Gdk::Pixbuf src)


 * PixbufUtils.rotate_180(Gdk::Pixbuf src)


 * PixbufUtils.rotate(Gdk::Pixbuf src, Integer angle)


 * PixbufUtils.rotate_ccw(Gdk::Pixbuf src)


 * PixbufUtils.gamma(Gdk::Pixbuf src, double level)


 * PixbufUtils.to_tiff(Gdk::Pixbuf src, String filename)


 * PixbufUtils.greyscale(Gdk::Pixbuf src)


 * PixbufUtils.greyscale_go(Gdk::Pixbuf src)


 * PixbufUtils.tint(Gdk::Pixbuf src, Integer r, Integer g, Integer b, Integer alpha=255)

  Tint an image - alpha=0 -> no tint, alpha=255 -> pure tint


 * PixbufUtils.perspect_v(Gdk::Pixbuf src, Integer top_x1, Integer top_x2, Integer bot_x1, Integer bot_x2)


 * PixbufUtils.mask(Gdk::Pixbuf src, Gdk::Pixbuf mask)


 * PixbufUtils.blend5050(Gdk::Pixbuf src1, Gdk::Pixbuf src2)


 * PixbufUtils.mask_area(Gdk::Pixbuf mask, Integer cutoff)


 * PixbufUtils.scale_max(Gdk::Pixbuf src, gulong max, Gdk::InterpType interp)


 * PixbufUtils.draw_scaled(Gdk::Drawable drawable, Gdk::Pixbuf src, Integer x, Integer y, Integer width, Integer height, GdkInterpType interp)


 * PixbufUtils.draw_scaled_clip(Gdk::Drawable drawable, Gdk::Pixbuf src, Integer x, Integer y, Integer width, Integer height, Gdk::Rectangle clip_area, Gdk::InterpType interp)

