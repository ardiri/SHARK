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
 * -- Added support for antialiased fonts  2005-07-23
 * -- Added support for win-1252 code page 2006-04-26
 */

import java.awt.*;
import java.awt.font.*;
import java.awt.image.*;
import java.io.*;

public class FontGen
{
  static public int MAX_CHARS = 256;
  static public int EXT_CHARS = 61;
  static public char ext_chars[] =
  {
    '\u0113', //      eMC
    '\u0251', //      eMC
    '\u02da', //      eMC
    '\u0387', //      eMC
    '\u0394', //      eMC
    '\u03a6', 
    '\u03b1', // BNF, eMC
    '\u03b2', //      eMC
    '\u03b3', //      eMC
    '\u03b4', //      eMC
    '\u03b5', //      eMC
    '\u03ba', //      eMC
    '\u03bb', //      eMC
    '\u03bc', //      eMC
    '\u03bf', //      eMC
    '\u03c3', //      eMC
    '\u03c4', //      eMC
    '\u03c6', //      eMC
    '\u03c7', //      eMC
    '\u0430', //      eMC
    '\u2011', //      eMC
    '\u2013', // BNF
    '\u2014', // BNF, eMC
    '\u2018', // BNF
    '\u2019', // BNF, eMC
    '\u201c',
    '\u201d',
    '\u2020',
    '\u2021',
    '\u2022', // BNF, eMC
    '\u2026',
    '\u2030',
    '\u2032', //      eMC
    '\u2033', //      eMC
    '\u2070', //      eMC
    '\u2122', //      eMC
    '\u2153', //      eMC
    '\u2191', //      eMC
    '\u2192', //      eMC
    '\u2193', //      eMC
    '\u2194', //      eMC
    '\u2212', //      eMC
    '\u2215', //      eMC
    '\u2219', //      eMC
    '\u221a', //      eMC
    '\u222d', //      eMC
    '\u221e', //      eMC
    '\u223c', //      eMC
    '\u2248', //      eMC
    '\u2260', //      eMC
    '\u2261', // BNF, eMC
    '\u2264', //      eMC
    '\u2265', // BNF, eMC
    '\u22c5', //      eMC
    '\u2500', //      eMC
    '\u25aa', //      eMC
    '\u25bc', //      eMC
    '\u25cf', //      eMC
    '\u2640', 
    '\u2642', 
    '\u2666'  //      eMC
  };

  public static void main(String args[])
    throws Exception
  {
    System.out.println(" *");
    System.out.println(" * @(#)FontGen.java");
    System.out.println(" *");
    System.out.println(" * -- Aaron Ardiri  (mailto:aaron@ardiri.com)");
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
      byte         fw[] = new byte[MAX_CHARS + EXT_CHARS];
      int          tw, th, kerning, spaced, th_adj;
      int          size, depth, cnt;
      int          _xr;

      BufferedImage  img;
      Graphics2D     g;
      PixelGrabber   pg;
      byte           pixels[];

      byte[] r = new byte[4]; byte[] gr = new byte[4]; byte[] b = new byte[4];
      r[0] = gr[0] = b[0] = (byte)  0;
      r[1] = gr[1] = b[1] = (byte) 85;
      r[2] = gr[2] = b[2] = (byte)170;
      r[3] = gr[3] = b[3] = (byte)255;
          
      IndexColorModel icm = new IndexColorModel(2,4,r,gr,b);

      depth = Integer.parseInt(args[0]);
      if ((depth != 1) && (depth != 2)) 
        throw new Exception("depth = 1 or 2 accepted");
        
      // initialize our font
      size = Integer.parseInt(args[2]);
           if (args[3].equals("PLAIN"))      
        { spaced = 0; f = new Font(args[1], Font.PLAIN,              size); }
      else if (args[3].equals("BOLD"))       
        { spaced = 0; f = new Font(args[1], Font.BOLD,               size); }
      else if (args[3].equals("ITALIC"))     
        { spaced = 1; f = new Font(args[1], Font.ITALIC,             size); }
      else if (args[3].equals("BOLDITALIC")) 
        { spaced = 1; f = new Font(args[1], Font.ITALIC | Font.BOLD, size); }
      else 
        throw new Exception("type = PLAIN, BOLD, ITALIC, BOLDITALIC accepted");

      // create a value component
      c = new Frame();
      c.setFont(f);
      c.addNotify();              // need to do this to allow graphics

      // obtain the font metrics
      fm = c.getFontMetrics(f);

      // lets figure out the font metrics of each character
      tw = fm.getHeight() << 1;
      th = fm.getHeight();

      th_adj = 0; if ((size+3) > th) th_adj = 1;
      th += th_adj;

      img = new BufferedImage(tw, th, BufferedImage.TYPE_BYTE_BINARY, icm);      
      g   = img.createGraphics();
      g.setRenderingHint(RenderingHints.KEY_RENDERING, 
                         RenderingHints.VALUE_RENDER_QUALITY);
      g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, 
         (depth == 1) ? RenderingHints.VALUE_TEXT_ANTIALIAS_OFF 
                      : RenderingHints.VALUE_TEXT_ANTIALIAS_ON);

      // space between characters
      kerning = fm.charWidth(' ') - 1;

//
// 0..255
//

      for (int i=0; i<32; i++)
      {
        fc[i] = 0;
        fo[i] = 0;
        fw[i] = 0;
      }

      for (int i=0; i<MAX_CHARS; i++)
      {
        fc[i] = (char)i; 

// win-1252 codepage
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
//
// dos-XXXX codepage
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP437.TXT (latin us)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP737.TXT (greek)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP775.TXT (baltic)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP850.TXT (latin 1)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP852.TXT (latin 2)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP855.TXT (cyrillic)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP857.TXT (turkey)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP860.TXT (portugal)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP861.TXT (iceland)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP862.TXT (hebrew)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP863.TXT (canada)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP864.TXT (arabic)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP865.TXT (nordic)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP866.TXT (russia)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP869.TXT (greek)
// http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/PC/CP874.TXT

        // use escape ranges for win-1252 font page (grr microsoft!)
        if ((i >= 0x80) && (i < 0xA0))
        {
          char x[] = { 
                       '\u20ac', // euro
                       0,
                       '\u201a', // single low-9 quote mark
                       '\u0192', // latin small letter f with hook
                       '\u201e', // doule low-9 quote mark
                       '\u2026', // horizontal ellipsis
                       '\u2020', // dagger
                       '\u2021', // double dagger
                       '\u02c6', // modifer letter circumflex accent
                       '\u2030', // per mille sign
                       '\u0160', // latin capital letter s with caron
                       '\u2039', // single left-pointing angle quote mark
                       '\u0152', // latin capital ligature oe
                       0,
                       '\u017d', // latin capital letter z with caron
                       0,
                       0,
                       '\u2018', // left single quote mark
                       '\u2019', // right single quote mark
                       '\u201c', // left double quote mark
                       '\u201d', // right double quote mark
                       '\u2022', // bullet
                       '\u2013', // en dash
                       '\u2014', // em dash
                       '\u02dc', // small tilde
                       '\u2122', // trade mark sign
                       '\u0161', // latin small letter s with caron
                       '\u203a', // single right pointing angle quote mark
                       '\u0153', // latin small ligature oe
                       0,
                       '\u017e', // latin small letter z with caron
                       '\u0178'  // latin capital letter y with diaeresis
                     };
          fc[i] = x[i-0x80];
        }

        g.setPaint(new Color(0xFF,0xFF,0xFF));
        g.fillRect(0,0, img.getWidth(),img.getHeight());    
        g.setPaint(new Color(0x00,0x00,0x00));
                                        
        g.setFont(f);
        g.drawString(""+(char)fc[i], 0, fm.getMaxAscent() + th_adj);

        // lets look at the pixels
        pg  = new PixelGrabber(img, 0, 0, tw, th, false);
        try { pg.grabPixels(); } catch (InterruptedException ie) {}
        pixels = (byte[])pg.getPixels();

        _xr = 0;
        if ((i != 32) && (i != 160))
        {
          _xr = tw-1;

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

             if ((i == 32) || (i == 160)) fw[i] = (byte)kerning;
        else if (_xr < 0)                 fw[i] = fw[0];
        else                              fw[i] = (byte)(_xr + kerning);

        // magical skip!
        if (i == 0) i = 31;
      }

// 256+

      for (int i=0; i<EXT_CHARS; i++)
      {
        fc[MAX_CHARS + i] = ext_chars[i];

        g.setPaint(new Color(0xFF,0xFF,0xFF));
        g.fillRect(0,0, img.getWidth(),img.getHeight());    
        g.setPaint(new Color(0x00,0x00,0x00));
                                        
        g.setFont(f);
        g.drawString(""+(char)fc[MAX_CHARS + i], 0, fm.getAscent() + th_adj);

        // lets look at the pixels
        pg  = new PixelGrabber(img, 0, 0, tw, th, false);
        try { pg.grabPixels(); } catch (InterruptedException ie) {}
        pixels = (byte[])pg.getPixels();

        _xr = 0;
        if (true)
        {
          _xr = tw-1;

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

             if (_xr < 0) fw[MAX_CHARS+i] = fw[0];
        else              fw[MAX_CHARS+i] = (byte)(_xr + kerning);
      }

      tw = 0;

      // work with the basics of the font
      cnt = MAX_CHARS + EXT_CHARS;
      for (int i=0; i<cnt; i++)
      {
        fw[i] += kerning; // ensure there is no overlap
        if ((fc[i] >= '1') && (fc[i] <= '9')) fw[i] = fw['0'];  // digit width 

        fo[i]  = tw;
        tw    += fw[i];

        // magical skip!
        if (i == 0) i = 31;
      }

      // make sure width is byte aligned width
      switch (depth)
      {
        case 1: if ((tw % 8) != 0) tw = ((tw + 7) / 8) * 8; break;
        case 2: if ((tw % 4) != 0) tw = ((tw + 3) / 4) * 4; break;
        default: break;
      }

      // draw to the offscreen window
      img = new BufferedImage(tw, th, BufferedImage.TYPE_BYTE_BINARY, icm);      
      g   = img.createGraphics();

      // draw all the characters to the buffer (hope for best)
      g.setPaint(new Color(0xFF,0xFF,0xFF));
      g.fillRect(0,0, img.getWidth(),img.getHeight());    
      g.setRenderingHint(RenderingHints.KEY_RENDERING, 
                         RenderingHints.VALUE_RENDER_QUALITY);
      g.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, 
        (depth == 1) ? RenderingHints.VALUE_TEXT_ANTIALIAS_OFF 
                     : RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
      g.setPaint(new Color(0x00,0x00,0x00));
      
      g.setFont(f);
      for (int i=0; i<cnt; i++)
      {
        g.drawString(""+fc[i], fo[i] + kerning, fm.getAscent() + th_adj);

        // magical skip!
        if (i == 0) i = 31;
      }

      // lets look at the pixels
      pg  = new PixelGrabber(img, 0, 0, tw, th, false);
      try { pg.grabPixels(); } catch (InterruptedException ie) {}
      pixels = (byte [])pg.getPixels();
            
      //
      // display final font statistics
      //

      System.out.println("FONT SOURCE:");
      System.out.println("  " + f.getFontName() + " " + f.getSize());
      System.out.println();

      System.out.println();
      System.out.println("  pixels (w) = " + tw);
      System.out.println("  pixels (h) = " + th);
      System.out.println("  kerning    = " + kerning);
      System.out.println();

/**
 ** DEBUG
 **
      for (int x=0; x<cnt; x++)
      {
        int index = x;

        System.out.println("index:   " + index);
        System.out.println("unicode: " + (int)fc[index]);
        System.out.println();

        for (int j=0; j<th; j++)
        {
          for (int i=fo[index]; i<(fo[index]+fw[index]); i++)
          {
            char ch;

                 if (pixels[(j * tw) + i] == 0) ch = '#'; 
            else if (pixels[(j * tw) + i] == 1) ch = '*'; 
            else if (pixels[(j * tw) + i] == 2) ch = '"'; 
            else ch = '.';
                      
            System.out.print(ch);
          }
          System.out.println();
        }

        System.out.println();

        // magical skip!
        if (x == 0) x = 31;
      }
 **
 ** REAL
 **/
      // display the string passed as a parameter (if available)
      if (args.length > 5)
      {
        System.out.println();
                                 
        for (int j=0; j<th; j++)
        {
          for (int x=0; x<args[5].length(); x++)
          {
            int index = args[5].charAt(x);
            
            for (int i=fo[index]; i<(fo[index]+fw[index]); i++)
            {
              char ch;

                   if (pixels[(j * tw) + i] == 0) ch = '#'; 
              else if (pixels[(j * tw) + i] == 1) ch = '*'; 
              else if (pixels[(j * tw) + i] == 2) ch = '"'; 
              else ch = '.';
                          
              System.out.print(ch);
            }
            System.out.print(' ');
          }
          System.out.println();
        }
        System.out.println("");
      }
/**/          
      //
      // save the font to a file
      //

      String fName = args[4];
      try
      {
        BufferedOutputStream out =
          new BufferedOutputStream(new FileOutputStream(new File(fName)));

        // this is how we mark our spaced fonts
        if (spaced == 1) cnt |= 0x8000;
 
        // HEADER:
        out.write('F');
        out.write('N');
        out.write('T');                            // signature
        out.write((depth == 1) ? '1' : '2');       // depth
        out.write((tw & 0xff00) >> 8);
        out.write((tw & 0x00ff));                  // width
        out.write((th & 0xff00) >> 8);
        out.write((th & 0x00ff));                  // height
        out.write((cnt & 0xff00) >> 8);
        out.write((cnt & 0x00ff));                 // char count
        out.write(fw, 0, cnt & 0x7fff);            // font widths
        for (int j=0; j<EXT_CHARS; j++)
        {
          out.write((ext_chars[j] & 0xff00) >> 8);
          out.write((ext_chars[j] & 0x00ff));      // unicode value
          out.write(0x01);
          out.write((j & 0x00ff));                 // table offset
        }
        
        // DATA:
        for (int j=0; j<th; j++)
        {
          int data = 0;

          switch (depth)
          {
            case 1:
                 // dump each row of pixels, one by one
                 for (int i=0; i<tw; i += 8)
                 {
                   data = 0;
                   if (pixels[(j * tw) + i+0] == 0) data = data | 0x80;
                   if (pixels[(j * tw) + i+1] == 0) data = data | 0x40;
                   if (pixels[(j * tw) + i+2] == 0) data = data | 0x20;
                   if (pixels[(j * tw) + i+3] == 0) data = data | 0x10;
                   if (pixels[(j * tw) + i+4] == 0) data = data | 0x08;
                   if (pixels[(j * tw) + i+5] == 0) data = data | 0x04;
                   if (pixels[(j * tw) + i+6] == 0) data = data | 0x02;
                   if (pixels[(j * tw) + i+7] == 0) data = data | 0x01;

                   out.write(data);
                 }
                 break;
                 
            case 2:
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
                 break;

            default:
                 break;
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
      System.out.println("  java FontGen {depth} {typeface} {size} {type} {outfile} {sample string}");
      System.out.println("");
    }
  }
}

/********************************* EOF ***********************************/
