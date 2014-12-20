require 'minitest/autorun'
require 'gtk2/base'
require 'gdk_pixbuf2'
require 'cairo'
require 'pixbufutils'

class TextPixbufUtils < MiniTest::Test
  def test_gamma
    pb = Gdk::Pixbuf.new(Gdk::Pixbuf::COLORSPACE_RGB, true, 8, 4, 4)
    pb.fill!(0x7f7f7fff)

    pixel_before = pb.pixels.unpack('C*')[0]

    PixbufUtils.gamma!(pb, 1.2)

    pixel_after  = pb.pixels.unpack('C*')[0]

    expected = (255 * ((pixel_before / 255.0) ** (1.0 / 1.2))).to_i
    assert_equal expected, pixel_after
  end
  def test_tint
    pb = Gdk::Pixbuf.new(Gdk::Pixbuf::COLORSPACE_RGB, true, 8, 4, 4)
    pb.fill!(0xff0000ff)

    out = PixbufUtils.tint(pb, 0, 255, 0)

    pixels_after  = out.pixels.unpack('CCC')

    assert_equal [78, 255, 78], pixels_after
  end
  def test_equalize
    pb = Gdk::Pixbuf.new(fn = "test/IMG_8344.JPG")
    equal = PixbufUtils.auto_equalize(pb)
    equal.save("#{fn}.auto_equalize.jpg", "jpeg")
    img = Cairo::ImageSurface.new(:rgb24, pb.width, pb.height)
    cr = Cairo::Context.new(img)
    cr.set_source_pixbuf(pb)
    cr.paint(1.0)

    cr.move_to(pb.width * 0.45, 0)
    cr.line_to(pb.width, 0)
    cr.line_to(pb.width, pb.height)
    cr.line_to(pb.width * 0.55, pb.height)
    cr.line_to(pb.width * 0.45, 0)
    cr.clip()
    cr.set_source_pixbuf(equal)
    cr.paint(1.0)
    img.write_to_png("equalize.png")
  end
end


