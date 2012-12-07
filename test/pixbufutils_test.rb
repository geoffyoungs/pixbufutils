require 'test/unit'
require 'gdk_pixbuf2'
require 'pixbufutils'

class TextPixbufUtils < Test::Unit::TestCase
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
end
