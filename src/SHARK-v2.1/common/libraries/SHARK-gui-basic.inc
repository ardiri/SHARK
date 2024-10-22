/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gui-basic.inc
 */

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define GUI_B_BACKGROUND _GfxGetPaletteIndex(192, 192, 192)
#define GUI_B_FOREGROUND _GfxGetPaletteIndex(  0,   0,   0)

#define GUI_B_SHADOW_HI  _GfxGetPaletteIndex(255, 255, 255)
#define GUI_B_SHADOW_LO  _GfxGetPaletteIndex(128, 128, 128)

#define GUI_BUTTON_PADDING 4

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct 
{
  void       *dummy;
} _GUI_B_Globals;

#define GUI_GLOBALS_B_ACCESS \
  _GUIGlobals     *g_gui; \
  _GUI_B_Globals  *g_b_gui; \
  g_gui   = (_GUIGlobals *)_LibGetGlobals(GUI_LIBRARY_ID); \
  g_b_gui = (_GUI_B_Globals *)g_gui -> extension;

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Generic _GUI routines
 *
 *************************************************************************/

static boolean
_GUI_B_Initialize()
{
  boolean         init_ok;
  _GUI_B_Globals *g_b_gui;
  GUI_GLOBALS_ACCESS;

  // assume everything is ok
  init_ok = true;

  // allocate globals memory for the library
  g_b_gui = (_GUI_B_Globals *)_MemPtrNew(sizeof(_GUI_B_Globals), false);
  init_ok &= (g_b_gui != NULL); if (!init_ok) goto GUI_B_INIT_ABORT;
  g_gui -> extension = (void *)g_b_gui;

GUI_B_INIT_ABORT:

  return init_ok;
}

static void
_GUI_B_Terminate()
{
  GUI_GLOBALS_B_ACCESS;

  // clean up
  if (g_b_gui != NULL) _MemPtrFree(g_b_gui);
  g_gui -> extension = NULL;
}

static void
_GUI_B_GetDefaultColors(color *fore, color *back)
{
  if (fore) *fore = GUI_B_FOREGROUND; 
  if (back) *back = GUI_B_BACKGROUND; 
}

/*************************************************************************
 *
 * _GUI :: panel implementation
 *
 *************************************************************************/

static void 
_GUI_B_panel_Draw(gui_element *e)
{
  rectangle rect;
  rectangle clip;
  int16     i;
//GUI_GLOBALS_B_ACCESS;

  // we need to get the bounds of this object
  _GUIGetElementPosition(e, &rect.x, &rect.y);
  rect.x      += e -> inset;
  rect.y      += e -> inset;
  rect.width   = e -> bounds.width;  
  rect.height  = e -> bounds.height;  

  // draw it
  _GfxGetClip(&clip); _GfxMergeClip(&rect);
  _GfxFillRegion(&rect, e -> background);

  // if there are any children - draw them now
  for (i=0; i < e -> children_count; i++)
    _GUIDraw(e -> children[i]);

  _GfxSetClip(&clip);
}

static void 
_GUI_B_panel_GetMinimumSize(gui_element *e, int16 *w, int16 *h)
{
  _GUI_container_GetMinimumSize(e, w, h);
}

/*************************************************************************
 *
 * _GUI :: label implementation
 *
 *************************************************************************/

static void 
_GUI_B_label_Draw(gui_element *e)
{
  gui_label_info *info;
  rectangle       rect;
  rectangle       clip;
  font            cur_font;
  coord           x, y;
//GUI_GLOBALS_B_ACCESS;

  // we need to get the bounds of this object
  _GUIGetElementPosition(e, &rect.x, &rect.y);
  rect.x      += e -> inset;
  rect.y      += e -> inset;
  rect.width   = e -> bounds.width;  
  rect.height  = e -> bounds.height;  

  // we need a reference to the "information" block
  info = (gui_label_info *)e -> extension;

  cur_font = _FntGetFont();
  _FntSetFont(info -> font_id);

  _GfxGetClip(&clip); _GfxMergeClip(&rect);

  // draw it
  switch (e -> justification)
  {
    case GUI_LEFT:
    case GUI_TOP:
         x = rect.x;
         y = rect.y;
         break;

    case GUI_RIGHT:
    case GUI_BOTTOM:
         x = rect.x + rect.width - 
             (int16)_FntGetCharsWidth(info -> label, _StrLen(info -> label));
         y = rect.y + rect.height - (int16)_FntGetFontHeight();
         break;

    default:
    case GUI_CENTER:
         x = rect.width - 
             (int16)_FntGetCharsWidth(info -> label, _StrLen(info -> label));
         y = rect.height - (int16)_FntGetFontHeight();

         x = rect.x + (x >> 1);
         y = rect.y + (y >> 1);
         break;
  }
  _GfxFillRegion(&rect, e -> background);
  _FntDrawString(info -> label, _StrLen(info -> label), x, y, gfxTransparent);

  _GfxSetClip(&clip);
  _FntSetFont(cur_font);
}

static void 
_GUI_B_label_GetMinimumSize(gui_element *e, int16 *w, int16 *h)
{
  gui_label_info *info;
  int16           width, height;
  font            cur_font;
//GUI_GLOBALS_B_ACCESS;

  // default values
  width  = 0;
  height = 0;

  // we need a reference to the "information" block
  info = (gui_label_info *)e -> extension;

  cur_font = _FntGetFont();
  _FntSetFont(info -> font_id);

  // get the boundaries of the label
  height = (int16)_FntGetFontHeight();
  width  = (int16)_FntGetCharsWidth(info -> label, _StrLen(info -> label));
  _FntSetFont(cur_font);

  // allow for "insets"
  width  += e -> inset << 1;
  height += e -> inset << 1;

  if (w != NULL) *w = width;
  if (h != NULL) *h = height;
}

/*************************************************************************
 *
 * _GUI :: image implementation
 *
 *************************************************************************/

static void 
_GUI_B_image_Draw(gui_element *e)
{
  gui_image_info *info;
  rectangle       rect;
  rectangle       rSrc;
  rectangle       clip;
  coord           x, y;
  int16           width, height;
//GUI_GLOBALS_B_ACCESS;

  // we need to get the bounds of this object
  _GUIGetElementPosition(e, &rect.x, &rect.y);
  rect.x      += e -> inset;
  rect.y      += e -> inset;
  rect.width   = e -> bounds.width;  
  rect.height  = e -> bounds.height;  

  // we need a reference to the "information" block
  info = (gui_image_info *)e -> extension;

  _GfxGetClip(&clip); _GfxMergeClip(&rect);

  // get the boundaries of the image
  _GfxGetWindowInfo(info -> image, NULL, &width, &height, NULL);

  rSrc.x      = 0;
  rSrc.y      = 0;
  rSrc.width  = width;
  rSrc.height = height;

  // draw it
  switch (e -> justification)
  {
    case GUI_LEFT:
    case GUI_TOP:
         x = rect.x;
         y = rect.y;
         break;

    case GUI_RIGHT:
    case GUI_BOTTOM:
         x = rect.x + rect.width  - width;
         y = rect.y + rect.height - height;
         break;

    default:
    case GUI_CENTER:
         x = rect.width  - width;
         y = rect.height - height;

         x = rect.x + (x >> 1);
         y = rect.y + (y >> 1);
         break;
  }
  _GfxCopyRegion(_GfxGetDrawWindow(), info -> image, &rSrc, x, y, gfxPaint);

  _GfxSetClip(&clip);
}

static void 
_GUI_B_image_GetMinimumSize(gui_element *e, int16 *w, int16 *h)
{
  gui_image_info *info;
  int16           width, height;
//GUI_GLOBALS_B_ACCESS;

  // default values
  width  = 0;
  height = 0;

  // we need a reference to the "information" block
  info = (gui_image_info *)e -> extension;

  // get the boundaries of the image
  _GfxGetWindowInfo(info -> image, NULL, &width, &height, NULL);

  // allow for "insets"
  width  += e -> inset << 1;
  height += e -> inset << 1;

  if (w != NULL) *w = width;
  if (h != NULL) *h = height;
}

/*************************************************************************
 *
 * _GUI :: button implementation
 *
 *************************************************************************/

static void 
_GUI_B_button_Draw(gui_element *e)
{
  gui_button_info *info;
  rectangle        rect;
  rectangle        clip;
  font             cur_font;
  coord            x, y;
  coord            x1, x2, y1, y2;
//GUI_GLOBALS_B_ACCESS;

  // we need to get the bounds of this object
  _GUIGetElementPosition(e, &rect.x, &rect.y);
  rect.x      += e -> inset;
  rect.y      += e -> inset;
  rect.width   = e -> bounds.width;  
  rect.height  = e -> bounds.height;  

  // we need a reference to the "information" block
  info = (gui_button_info *)e -> extension;

  cur_font = _FntGetFont();
  _FntSetFont(info -> font_id);

  _GfxGetClip(&clip); _GfxMergeClip(&rect);

  // draw it
  _GfxFillRegion(&rect, e -> background);
  x1 = rect.x; x2 = rect.x+rect.width - 1; y1 = rect.y; y2 = y1;
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_LO : GUI_B_SHADOW_HI);
  x1 = rect.x; x2 = x1; y1 = rect.y; y2 = rect.y+rect.height-1;
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_LO : GUI_B_SHADOW_HI);
  x1 = rect.x; x2 = rect.x+rect.width-1; y1 = rect.y+rect.height-1; y2 = y1;
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_HI : GUI_B_SHADOW_LO);
  x1 = rect.x+rect.width-1; x2 = x1; y1 = rect.y; y2 = rect.y+rect.height-1; 
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_HI : GUI_B_SHADOW_LO);

  // draw it
  switch (e -> justification)
  {
    case GUI_LEFT:
    case GUI_TOP:
         x = rect.x;
         y = rect.y;
         break;

    case GUI_RIGHT:
    case GUI_BOTTOM:
         x = rect.x + rect.width - 
             (int16)_FntGetCharsWidth(info -> label, _StrLen(info -> label));
         y = rect.y + rect.height - (int16)_FntGetFontHeight();
         break;

    default:
    case GUI_CENTER:
         x = rect.width - 
            (int16)_FntGetCharsWidth(info -> label, _StrLen(info -> label));
         y = rect.height - (int16)_FntGetFontHeight();

         x = rect.x + (x >> 1);
         y = rect.y + (y >> 1);
         break;
  }
  if (info -> pressed) { x++; y++; }
  _FntDrawString(info -> label, _StrLen(info -> label), x, y, gfxTransparent);

  _GfxSetClip(&clip);
  _FntSetFont(cur_font);
}

static void 
_GUI_B_button_GetMinimumSize(gui_element *e, int16 *w, int16 *h)
{
  gui_button_info *info;
  int16            width, height;
  font             cur_font;
//GUI_GLOBALS_B_ACCESS;

  // default values
  width  = 0;
  height = 0;

  // we need a reference to the "information" block
  info = (gui_button_info *)e -> extension;

  cur_font = _FntGetFont();
  _FntSetFont(info -> font_id);

  // get the boundaries of the label
  height = (int16)_FntGetFontHeight();
  width  = (int16)_FntGetCharsWidth(info -> label, _StrLen(info -> label));
  _FntSetFont(cur_font);

  width  += GUI_BUTTON_PADDING;
  height += GUI_BUTTON_PADDING;  // for shadows

  // allow for "insets"
  width  += e -> inset << 1;
  height += e -> inset << 1;

  if (w != NULL) *w = width;
  if (h != NULL) *h = height;
}

/*************************************************************************
 *
 * _GUI :: image button implementation
 *
 *************************************************************************/

static void 
_GUI_B_imagebutton_Draw(gui_element *e)
{
  gui_imagebutton_info *info;
  rectangle             rect;
  rectangle             rSrc;
  rectangle             clip;
  coord                 x, y;
  coord                 x1, x2, y1, y2;
  int16                 width, height;
  gfx_window           *image;
//GUI_GLOBALS_B_ACCESS;

  // we need to get the bounds of this object
  _GUIGetElementPosition(e, &rect.x, &rect.y);
  rect.x      += e -> inset;
  rect.y      += e -> inset;
  rect.width   = e -> bounds.width;  
  rect.height  = e -> bounds.height;  

  // we need a reference to the "information" block
  info = (gui_imagebutton_info *)e -> extension;

  _GfxGetClip(&clip); _GfxMergeClip(&rect);

  // draw it
  _GfxFillRegion(&rect, e -> background);
  x1 = rect.x; x2 = rect.x+rect.width - 1; y1 = rect.y; y2 = y1;
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_LO : GUI_B_SHADOW_HI);
  x1 = rect.x; x2 = x1; y1 = rect.y; y2 = rect.y+rect.height-1;
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_LO : GUI_B_SHADOW_HI);
  x1 = rect.x; x2 = rect.x+rect.width-1; y1 = rect.y+rect.height-1; y2 = y1;
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_HI : GUI_B_SHADOW_LO);
  x1 = rect.x+rect.width-1; x2 = x1; y1 = rect.y; y2 = rect.y+rect.height-1; 
  _GfxDrawLine(x1, y1, x2, y2, 
               (info -> pressed) ? GUI_B_SHADOW_HI : GUI_B_SHADOW_LO);

  image = info -> image; 
  if ((!e -> active) && (info -> image_inactive != NULL)) 
    image = info -> image_inactive; 

  // get the boundaries of the image on the button
  _GfxGetWindowInfo(image, NULL, &width, &height, NULL);

  rSrc.x      = 0;
  rSrc.y      = 0;
  rSrc.width  = width;
  rSrc.height = height;

  // draw it
  switch (e -> justification)
  {
    case GUI_LEFT:
    case GUI_TOP:
         x = rect.x;
         y = rect.y;
         break;

    case GUI_RIGHT:
    case GUI_BOTTOM:
         x = rect.x + rect.width  - width;
         y = rect.y + rect.height - height;
         break;

    default:
    case GUI_CENTER:
         x = rect.width  - width;
         y = rect.height - height;

         x = rect.x + (x >> 1);
         y = rect.y + (y >> 1);
         break;
  }
  if (info -> pressed) { x++; y++; }
  _GfxCopyRegion(_GfxGetDrawWindow(), image, &rSrc, x, y, gfxPaint);

  _GfxSetClip(&clip);
}

static void 
_GUI_B_imagebutton_GetMinimumSize(gui_element *e, int16 *w, int16 *h)
{
  gui_imagebutton_info *info;
  int16                 _w, _h, width, height;
//GUI_GLOBALS_B_ACCESS;

  // default values
  width  = 0;
  height = 0;

  // we need a reference to the "information" block
  info = (gui_imagebutton_info *)e -> extension;

  // get the boundaries of the image on the button
  _w = _h = 0;
  _GfxGetWindowInfo(info -> image, NULL, &width, &height, NULL);
  if (info -> image_inactive)
    _GfxGetWindowInfo(info -> image_inactive, NULL, &_w, &_h, NULL);
  width  = MAX(width,  _w);
  height = MAX(height, _h);

  width  += GUI_BUTTON_PADDING;
  height += GUI_BUTTON_PADDING;  // for shadows

  // allow for "insets"
  width  += e -> inset << 1;
  height += e -> inset << 1;

  if (w != NULL) *w = width;
  if (h != NULL) *h = height;
}

/********************************* EOF ***********************************/
