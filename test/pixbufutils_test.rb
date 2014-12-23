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
    FileUtils.mkdir_p("output/auto_equalize")
    Dir.glob("samples/*.*").each do |jpeg|
      pb = Gdk::Pixbuf.new(jpeg)
      equal = PixbufUtils.auto_equalize(pb)
      #equal.save("#{fn}.auto_equalize.jpg", "jpeg")

      width = 600
      pb = pb.scale(width, (width * pb.height/pb.width).to_i)
      equal = equal.scale(pb.width, pb.height)
      img = Cairo::ImageSurface.new(:rgb24, pb.width, pb.height)
      cr = Cairo::Context.new(img)
      cr.set_source_pixbuf(pb)
      cr.paint(1.0)

      cr.move_to(pb.width * 0.45, 0)
      cr.line_to(pb.width, 0)
      cr.line_to(pb.width, pb.height)
      cr.line_to(pb.width * 0.55, pb.height)
      cr.line_to(pb.width * 0.45, 0)
      cr.save do
      cr.clip()
      cr.set_source_pixbuf(equal)
      cr.paint(1.0)
      end

      cr.set_source_rgb(0,0,0)
      cr.move_to(10, 10)

      layout = cr.create_pango_layout
      layout.text = "Original"
      layout.font_description = Pango::FontDescription.new("Sans Bold 12")
      cr.update_pango_layout(layout)
      cr.move_to(10,10)
      cr.show_pango_layout(layout)
      cr.set_source_rgb(1,1,1)
      cr.move_to(8, 8)
      cr.show_pango_layout(layout)
      cr.set_source_rgb(0,0,0)
      cr.move_to(10, 10)


      layout = cr.create_pango_layout
      layout.text = "Corrected"
      layout.font_description = Pango::FontDescription.new("Sans Bold 12")
      cr.update_pango_layout(layout)
      width = layout.pixel_extents[0].width
      cr.move_to(pb.width - 10 - width,10)
      cr.show_pango_layout(layout)
      cr.set_source_rgb(1,1,1)
      cr.move_to(pb.width - width - 8, 8)
      cr.show_pango_layout(layout)


      img.write_to_png(combine="output/auto_equalize/#{File.basename jpeg}.png")
      pb.save(orig="output/auto_equalize/#{File.basename jpeg, '.JPG'}.orig.jpg", "jpeg")
      equal.save(eql="output/auto_equalize/#{File.basename jpeg, '.JPG'}.equalized.jpg", "jpeg")
      system("convert", orig, "histogram:#{ ho = orig.sub(/\.jpg$/, ".hist.jpg") }")
      system("convert", eql, "histogram:#{ he= eql.sub(/\.jpg$/, ".hist.jpg") }")
      system("montage", '-quality', '90', "-tile", "2x", "-mode", "Concatenate", orig, ho, eql, he, combine, "output/auto_equalize/#{File.basename jpeg}.montage.jpg" )
      system("rm", "-f", ho, he, orig, eql)
    end
  end
end


