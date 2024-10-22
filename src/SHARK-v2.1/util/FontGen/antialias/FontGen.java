/*************************************************************************
 *
 * Copyright (c) 2003 MobileWizardry
 * All rights reservered.
 *
 *************************************************************************/

/*
 * @(#)FontGen.java
 *
 * -- Aaron Ardiri  (mailto:aaron_ardiri@mobilewizardry.com)
 *
 * -- Added support for Antialiased fonts 2005-07-23
 */

import java.awt.*;
import java.awt.font.*;
import java.awt.image.*;
import java.io.*;

public class FontGen
{
  static public int MAX_CHARS = 256;
  static public int EXT_CHARS = 33;
  static public char ext_chars[] =
  {
    '\u0394',
    '\u03a6',
    '\u03b1',
    '\u03b2',
    '\u03b3',
    '\u03b4',
    '\u03b5',
    '\u03bc',
    '\u03c4',
    '\u2011',
    '\u2013',
    '\u2014',
    '\u2018',
    '\u2019',
    '\u201c',
    '\u201d',
    '\u2020',
    '\u2021',
    '\u2022',
    '\u2026',
    '\u2030',
    '\u2032',
    '\u2122',
    '\u2192',
    '\u221a',
    '\u221e',
    '\u2248',
    '\u2261',
    '\u2264',
    '\u2265',
    '\u2640',
    '\uf061',
    '\uf0b3'
  };

  public static void main(String args[])
    throws Exception
  {
    System.out.println(" *");
    System.out.println(" * @(#)FontGen.java (antialias)");
    System.out.println(" *");
    System.out.println(" * -- Aaron Ardiri  (mailto:aaron_ardiri@mobilewizardry.com)");
    System.out.println(" *");
    System.out.println();

    // enough command line arguments?
    if (args.length > 1)
    {
      FontMetrics  fm;
      Container    c;
      Font         f;
      char         fc[] = new char[MAX_CHARS + EXT_CHARS];
      int          fo[] = new  int[MAX_CHARS + EXT_CHARS];
      int          fa[] = new  int[MAX_CHARS + EXT_CHARS];
      byte         fw[] = new byte[MAX_CHARS + EXT_CHARS];
      int          tw, th, o;
      int          size, cnt;
      int          _xl, _xr;
      int          _yu, _yd;

      //Image        img;
      //Graphics     g;
      BufferedImage  img;
      Graphics2D     g;
      PixelGrabber   pg;
      byte           pixels[];

      byte[] r = new byte[4]; byte[] gr = new byte[4]; byte[] b = new byte[4];
      r[0] =            gr[0] =            b[0] = (byte)0;
      r[1] = (byte)85;  gr[1] = (byte)85;  b[1] = (byte)85;
      r[2] = (byte)170; gr[2] = (byte)170; b[2] = (byte)170;
      r[3] = (byte)255; gr[3] = (byte)255; b[3] = (byte)255;
          
      IndexColorModel icm = new IndexColorModel(2,4,r,gr,b);
      
      // initialize our font
      size = Integer.parseInt(args[1]);
           if (args[2].equals("PLAIN"))      f = new Font(args[0], Font.PLAIN, size);
      else if (args[2].equals("BOLD"))       f = new Font(args[0], Font.BOLD, size);
      else if (args[2].equals("ITALIC"))     f = new Font(args[0], Font.ITALIC, size);
      else if (args[2].equals("BOLDITALIC")) f = new Font(args[0], Font.ITALIC | Font.BOLD, size);
      else throw new Exception("PLAIN, BOLD, ITALIC, BOLDITALIC keyword required");

      // create a value component
      c = new Frame();
      c.setFont(f);
      c.addNotify();              // need to do this to allow graphics

      // obtain the font metrics
      fm = c.getFontMetrics(f);

      // lets figure out the font metrics of each character
      tw = size << 1;
      th = size << 1;
      o  = size >> 2;
      img = new BufferedImage(tw,th, BufferedImage.TYPE_BYTE_BINARY, icm);      
      g   = img.createGraphics();

      cnt = MAX_CHARS + EXT_CHARS;
      
//
// 0..255
//
           
      for (int i=0; i<MAX_CHARS; i++)
      {
        g.setPaint(new Color(0xFFFFFFFF));
        g.fillRect(0,0, img.getWidth(),img.getHeight());    
        g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, 
                           RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g.setPaint(new Color(0,0,0));
                                        
        g.setFont(f);
        g.drawString(""+(char)i, o, fm.getMaxAscent());

        // lets look at the pixels
        pg  = new PixelGrabber(img, 0, 0, tw, th, false);
        try { pg.grabPixels(); } catch (InterruptedException ie) {}
        pixels = (byte[])pg.getPixels();

        _xl = _xr = 0;
        if (i != 32)
        {
          _xl = 0;
          _xr = tw-1;

  xl_loop:

          while (_xl < tw)
          {
            for (int j=0; j<th; j++)
            {
              if (pixels[(j*tw)+_xl] != 3) break xl_loop;
            }
           _xl++;
          }

  xr_loop:

          while (_xr > 0)
          {
            for (int j=0; j<th; j++)
            {
              if (pixels[(j*tw)+_xr] != 3) break xr_loop;
            }
            _xr--;
          }
        }

        if (_xl > _xr)
        {
          fa[i] = 0;
          fw[i] = fw[0];
        }
        else
        {
          fa[i] = -_xl;
          fw[i] = (byte)(_xr - _xl + (fm.charWidth(' ')-1)); // kerning of font [space between]
        }
      }

// 256+

      for (int i=0; i<EXT_CHARS; i++)
      {
        g.setPaint(new Color(0xFFFFFFFF));
        g.fillRect(0,0, img.getWidth(),img.getHeight());    
        g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, 
                           RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g.setPaint(new Color(0,0,0));
                                        
        g.setFont(f);
        g.drawString(""+(char)ext_chars[i], o, fm.getMaxAscent());

        // lets look at the pixels
        pg  = new PixelGrabber(img, 0, 0, tw, th, false);
        try { pg.grabPixels(); } catch (InterruptedException ie) {}
        pixels = (byte[])pg.getPixels();

        _xl = _xr = 0;
        if (true)
        {
          _xl = 0;
          _xr = tw-1;

  xl_loop:

          while (_xl < tw)
          {
            for (int j=0; j<th; j++)
            {
              if (pixels[(j*tw)+_xl] != 3) break xl_loop;
            }
           _xl++;
          }

  xr_loop:

          while (_xr > 0)
          {
            for (int j=0; j<th; j++)
            {
              if (pixels[(j*tw)+_xr] != 3) break xr_loop;
            }
            _xr--;
          }
        }

        if (_xl > _xr)
        {
          fa[MAX_CHARS+i] = 0;
          fw[MAX_CHARS+i] = fw[0];
        }
        else
        {
          fa[MAX_CHARS+i] = -_xl;
          fw[MAX_CHARS+i] = (byte)(_xr - _xl + (fm.charWidth(' ')-1)); // kerning of font [space between]
        }
      }

      // work with the basics of the font
      tw = 0;
      th = fm.getHeight();
      for (int i=0; i<cnt; i++)
      {
        if (i < MAX_CHARS) fc[i] = (char)i; else fc[i] = ext_chars[i-MAX_CHARS];
        if (!f.canDisplay(fc[i]))
        {
          fw[i] = fw[0];
          fa[i] = fa[0];
          fc[i] = 0;        // non printable characters
        }
        if ((fc[i] >= '1') && (fc[i] <= '9'))
          fw[i] = fw['0'];  // force with for chars

        fo[i]  = tw;
        tw    += fw[i];
      }

      if ((tw % 4) != 0) tw = ((tw + 3) / 4) * 4;     // make sure its on 4-rounded width

      // draw to the offscreen window
      img = new BufferedImage(tw, th, BufferedImage.TYPE_BYTE_BINARY, icm);      
      g   = img.createGraphics();

      // draw all the characters to the buffer (hope for best)
      g.setPaint(new Color(0xFFFFFFFF));
      g.fillRect(0,0, img.getWidth(),img.getHeight());    
      g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, 
                         RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
      g.setPaint(new Color(0,0,0));
      
      g.setFont(f);
      for (int i=0; i<cnt; i++)
        g.drawString(""+fc[i], o + fo[i] + fa[i], fm.getMaxAscent());

      // lets look at the pixels
      pg  = new PixelGrabber(img, 0, 0, tw, th, false);
      try { pg.grabPixels(); } catch (InterruptedException ie) {}
      pixels = (byte [])pg.getPixels();
            
      // calculate absolute upper/lower for font
      _yu = 0;
      _yd = th-1;

  yu_loop:

      while (_yu < th)
      {
        for (int i=0; i<tw; i++)
        {
          if (pixels[(_yu*tw)+i] != 3) break yu_loop;
        }
        _yu++;
      }

  yd_loop:

      while (_yd > 0)
      {
        for (int i=0; i<tw; i++)
        {
          if (pixels[(_yd*tw)+i] != 3) break yd_loop;
        }
        _yd--;
      }
      _yd++;  // upper boundary < _yd, need one pixel extra for display

      // new total height value
      th = _yd - _yu;

      //
      // display final font statistics
      //

      System.out.println("FONT SOURCE:");
      System.out.println("  " + f.getFontName() + " " + f.getSize());
      System.out.println();

      System.out.println("FONT METRICS:");
      System.out.println();
      for (int i=0; i<cnt; i++)
      {
        if (i == 256) System.out.println("EXTENDED:");
        System.out.print(fc[i] + "=" + ((fw[i] < 10) ? " " : "") + fw[i] + ", ");
        if ((i % 8) == 7) System.out.println();
      }
      System.out.println();
      System.out.println("  pixels (w) = " + tw);
      System.out.println("  pixels (h) = " + th);
      System.out.println("");

      // display the string passed as a parameter (if available)
      if (args.length > 4)
      {
        System.out.println();
                                 
        for (int j=_yu; j<_yd; j++)
        {
          for (int x=0; x<args[4].length(); x++)
          {
            int index = args[4].charAt(x);
            for (int i=fo[index]; i<fo[index+1]; i++)
            {
              char ch;

                   if (pixels[(j * tw) + i] == 0) ch = '.'; 
              else if (pixels[(j * tw) + i] == 1) ch = ':'; 
              else if (pixels[(j * tw) + i] == 2) ch = ';'; 
              else ch = 'x';
                          
              System.out.print(ch);
            }
          }
          System.out.println();
        }
        System.out.println("");
      }
          
      //
      // save the font to a file
      //

      String fName = args[3];
      try
      {
        BufferedOutputStream out =
          new BufferedOutputStream(new FileOutputStream(new File(fName)));
 
        // HEADER:
        out.write('F');
        out.write('N');
        out.write('T');                            // signature
        out.write('2');                            // depth
        out.write((tw & 0xff00) >> 8);
        out.write((tw & 0x00ff));                  // width
        out.write((th & 0xff00) >> 8);
        out.write((th & 0x00ff));                  // height
        out.write((cnt & 0xff00) >> 8);
        out.write((cnt & 0x00ff));                 // char count
        out.write(fw, 0, cnt);                     // font widths
        for (int j=0; j<EXT_CHARS; j++)
        {
          out.write((ext_chars[j] & 0xff00) >> 8);
          out.write((ext_chars[j] & 0x00ff));      // unicode value
          out.write(0x01);
          out.write((j & 0x00ff));                 // table offset
        }
        
        // DATA:
        for (int j=_yu; j<_yd; j++)
        {
          int data = 0;

          // dump each row of pixels, one by one
          for (int i=0; i<tw; i += 4)
          {
            data = 0;
            data = data | (3- pixels[(j * tw) + i]   << 6);
            data = data | (3- pixels[(j * tw) + i+1] << 4);
            data = data | (3- pixels[(j * tw) + i+2] << 2);
            data = data |  3- pixels[(j * tw) + i+3];
                        
            out.write(data);
          }
        }
                
        out.flush();
        out.close();
      }
      catch (Exception e) {e.printStackTrace(); }

      // need to termiante the GUI thread
      System.exit(0);
    }
    else
    {
      System.out.println("USAGE:");
      System.out.println("  java FontGen {typeface} {size} {type} {outfile} {sample string}");
      System.out.println("");
    }
  }
}

/********************************* EOF ***********************************/
