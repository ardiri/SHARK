/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gui-core.inc
 */

static void
_GUI_component_SetPosition(gui_element *e, coord x, coord y)
{
  // entry requirements
  if (e == NULL) return;

  e -> bounds.x = x;
  e -> bounds.y = y;
}

static void
_GUI_component_SetSize(gui_element *e, int16 width, int16 height)
{
  // entry requirements
  if (e == NULL) return;

  e -> bounds.width  = width;
  e -> bounds.height = height;
}

static void
_GUI_container_SetSize(gui_element *e, int16 width, int16 height)
{
  uint8 hgap, vgap;
  int16 i;

  // entry requirements
  if (e == NULL) return;

  e -> bounds.width  = width;
  e -> bounds.height = height;

  hgap = e -> layout.hgap;
  vgap = e -> layout.vgap;

  switch (e -> layout.type)
  {
    // FLOW LAYOUT
    // +-----+     +-----+
    // |     |     |     |
    // +-----+ ... +-----+ components flowing after each other, vert or horiz

    case gui_flow:
         {
           coord        x, y;
           int16        dx, dy, w, h;  
           int8         orientation, justification;
           gui_element *child;

           orientation   = e -> layout.data.flowlayout.orientation;
           justification = e -> layout.data.flowlayout.justification;
 
           x  = 0; y  = 0;
           dx = 0; dy = 0;

           switch (orientation)
           {
             case GUI_HORIZONTAL:
                  for (i=0; i < e -> children_count; i++)
                  {
                    _GUIGetMinimumSize(e -> children[i], &w, &h); dx += w;
                  }
                  dx += (e -> children_count - 1) * hgap;

                  switch (justification)
                  {
                    case GUI_LEFT:
                    case GUI_TOP:    dx = 0;                 break;
                    case GUI_CENTER: dx = (width - dx) >> 1; break;
                    case GUI_RIGHT:
                    case GUI_BOTTOM: dx = (width - dx);      break;
                  }

                  for (i=0; i < e -> children_count; i++)
                  {
                    child = e -> children[i];
                    _GUIGetMinimumSize(child, &w, &h); 

                    _GUISetSize(child, w, h);
                    _GUISetPosition(child, (coord)(dx + x), y);
                    x += (w + hgap);
                  }
                  break;

             case GUI_VERTICAL:
                  for (i=0; i < e -> children_count; i++)
                  {
                    _GUIGetMinimumSize(e -> children[i], &w, &h); dy += h;
                  }
                  dy += (e -> children_count - 1) * vgap; 

                  switch (justification)
                  {
                    case GUI_LEFT:
                    case GUI_TOP:    dy = 0;                  break;
                    case GUI_CENTER: dy = (height - dy) >> 1; break;
                    case GUI_RIGHT:
                    case GUI_BOTTOM: dy = (height - dy);      break;
                  }

                  for (i=0; i < e -> children_count; i++)
                  {
                    child = e -> children[i];
                    _GUIGetMinimumSize(child, &w, &h); 

                    _GUISetSize(child, w, h);
                    _GUISetPosition(child, x, (coord)(dy + y));
                    y += (h + vgap);
                  }
                  break;

             default:
                  break;
           }
         }
         break;

    // GRID LAYOUT
    // +-----+-----+
    // |     |     |
    // +-----+-----+
    // |     |     |
    // +-----+-----+ equi-sized panels with cols,rows definition
    
    case gui_grid:
         {
           int8         r, c, rows, cols;
           coord        x, y;
           int16        c_width, c_height;
#if (GUI_MINIMUMSIZE == 1)
           int16        w, h;
#endif
           gui_element *child;

           cols = e -> layout.data.gridlayout.cols;
           rows = e -> layout.data.gridlayout.rows;

           width   -= (cols - 1) * hgap;
           height  -= (rows - 1) * vgap;
           c_width  = width  / cols;
           c_height = height / rows;

#if (GUI_MINIMUMSIZE == 1)
           for (i=0; i < e -> children_count; i++)
           {
             child = e -> children[i];
             _GUIGetMinimumSize(child, &w, &h); 
             c_width  = MAX(c_width,  w);
             c_height = MAX(c_height, h);
           }
#endif

           for (i=0; i < e -> children_count; i++)
           {
             child = e -> children[i];

             c = child -> position.data.gridlayout.col;
             r = child -> position.data.gridlayout.row;

             x = c * (c_width  + hgap);
             y = r * (c_height + vgap);

             _GUISetPosition(child, x, y);
             _GUISetSize(child, c_width, c_height);
           }
         }
         break;

    // BORDER LAYOUT
    // +----------------------+
    // |        NORTH         |
    // +------+--------+------+
    // | WEST | CENTER | EAST |
    // +------+--------+------+
    // |        SOUTH         |
    // +----------------------+ extremity restrictions, central portion resizing

    case gui_border:
         {
           gui_element *panel[5] = { NULL };
           int16        h, h1, h2, h3;
           int16        w, w1, w2, w3;
           coord        x, y;
           gui_element *child;

           for (i=0; i < e -> children_count; i++)
           {
             child = e -> children[i];
             panel[child -> position.data.borderlayout.zone] = child;
           }

           h1 = h2 = h3 = 0;
           w1 = w2 = w3 = 0;

           // determine the extremity's (w1,w3,h1,h3)
           w = h = 0; _GUIGetMinimumSize(panel[GUI_NORTH],  &w, &h); h1 = h;
           w = h = 0; _GUIGetMinimumSize(panel[GUI_WEST],   &w, &h); w1 = w;
           w = h = 0; _GUIGetMinimumSize(panel[GUI_EAST],   &w, &h); w3 = w;
           w = h = 0; _GUIGetMinimumSize(panel[GUI_SOUTH],  &w, &h); h3 = h;

           // calculate inner region based on whats left over (w2,h2)
           w2 = width  - (((w1 != 0) ? (w1 + hgap) : 0) +
                          ((w3 != 0) ? (w3 + hgap) : 0));
           h2 = height - (((h1 != 0) ? (h1 + vgap) : 0) +
                          ((h3 != 0) ? (h3 + vgap) : 0));
#if (GUI_MINIMUMSIZE == 1)
           w = h = 0; _GUIGetMinimumSize(panel[GUI_WEST],   &w, &h); 
           h2 = MAX(h2, h);
           w = h = 0; _GUIGetMinimumSize(panel[GUI_EAST],   &w, &h); 
           h2 = MAX(h2, h);
           w = h = 0; _GUIGetMinimumSize(panel[GUI_CENTER], &w, &h); 
           h2 = MAX(h2, h);
           w2 = MAX(w2, w); 

           // override the width and height for minimum size setting
           width  = w1 + w2 + w3 + 
                    ((w1 != 0) ? hgap : 0) + ((w3 != 0) ? hgap : 0);
           height = h1 + h2 + h3 + 
                    ((h1 != 0) ? vgap : 0) + ((h3 != 0) ? vgap : 0);
#endif

           // :: northern pieces
           x = 0; y = 0;
           _GUISetPosition(panel[GUI_NORTH], x, y);
           _GUISetSize(panel[GUI_NORTH], width, h1);

           // :: central pieces
           x = 0; y = (h1 != 0) ? (h1 + hgap) : 0;
           _GUISetPosition(panel[GUI_WEST], x, y);
           _GUISetSize(panel[GUI_WEST], w1, h2);

           x += (w1 != 0) ? (w1 + vgap) : 0;
           _GUISetPosition(panel[GUI_CENTER], x, y);
           _GUISetSize(panel[GUI_CENTER], w2, h2);

           x += (w2 != 0) ? (w2 + vgap) : 0;
           _GUISetPosition(panel[GUI_EAST], x, y);
           _GUISetSize(panel[GUI_EAST], w3, h2);

           // :: southern pieces
           x = 0; y = height - h3;
           _GUISetPosition(panel[GUI_SOUTH], x, y);
           _GUISetSize(panel[GUI_SOUTH], width, h3);
         }
         break;

    // STRETCH LAYOUT
    //             +----+----+
    //           / |    |    |  *components inside are NOT managed
    //         /   +----+----+
    //       /     |    |    |
    // +-+-+       +----+----+
    // +-+-+       |    |    |
    // +-+-+ ----- +----+----+ scalable area with ABS co-ordinates

    case gui_stretch:
         {
           int16        w, h;
           coord        x, y;
           uint8        orientation;
           float        scale_x, scale_y;
           gui_element *child;

#if (GUI_MINIMUMSIZE == 1)
           width  = MAX(width, e -> layout.data.stretchlayout.width);
           height = MAX(width, e -> layout.data.stretchlayout.height);
#endif
           scale_x = (float)width  / e -> layout.data.stretchlayout.width;
           scale_y = (float)height / e -> layout.data.stretchlayout.height;

           orientation = e -> layout.data.stretchlayout.orientation;
           switch (orientation)
           {
             case GUI_HORIZONTAL: scale_y = 1.0; break;
             case GUI_VERTICAL:   scale_x = 1.0; break;
             default: break;
           }

           for (i=0; i < e -> children_count; i++)
           {
             child = e -> children[i];
 
             x = (coord)(child -> position.data.stretchlayout.x * scale_x);
             y = (coord)(child -> position.data.stretchlayout.y * scale_y);
             w = (coord)(child -> position.data.stretchlayout.width  * scale_x);
             h = (coord)(child -> position.data.stretchlayout.height * scale_y);

             _GUISetPosition(child, x, y);
             _GUISetSize(child, w, h);
           }
         }
         break;

    default:
    case gui_none:
         break;
  }
}

static void
_GUI_container_GetMinimumSize(gui_element *e, int16 *w, int16 *h)
{
  int16 i;
  uint8 hgap, vgap;
  int16 width, height;
//GUI_GLOBALS_B_ACCESS;

  // default values
  width  = 0;
  height = 0;

  // entry requirements
  if (e == NULL) return;

  hgap = e -> layout.hgap;
  vgap = e -> layout.vgap;

  switch (e -> layout.type)
  {
    // FLOW LAYOUT
    // +-----+     +-----+
    // |     |     |     |
    // +-----+ ... +-----+ components flowing after each other, vert or horiz

    case gui_flow:
         {
           int16        dx, dy, w, h;  
           int8         orientation;
           gui_element *child;

           orientation   = e -> layout.data.flowlayout.orientation;
 
           dx = 0; dy = 0;
           switch (orientation)
           {
             case GUI_HORIZONTAL:
                  for (i=0; i < e -> children_count; i++)
                  {
                    child = e -> children[i];
                    _GUIGetMinimumSize(child, &w, &h); dx += w;
                    height = MAX(height, h);
                  }
                  width = dx;
                  width += (e -> children_count - 1) * hgap;
                  break;

             case GUI_VERTICAL:
                  for (i=0; i < e -> children_count; i++)
                  {
                    _GUIGetMinimumSize(e -> children[i], &w, &h); dy += h;
                    width = MAX(width, w);
                  }
                  height = dy;
                  height += (e -> children_count - 1) * vgap; 
                  break;

             default:
                  break;
           }
         }
         break;

    // GRID LAYOUT
    // +-----+-----+
    // |     |     |
    // +-----+-----+
    // |     |     |
    // +-----+-----+ equi-sized panels with cols,rows definition
    
    case gui_grid:
         {
           int8         rows, cols;
           int16        c_width, c_height;
#if (GUI_MINIMUMSIZE == 1)
           int16        w, h;
#endif
           gui_element *child;

           cols = e -> layout.data.gridlayout.cols;
           rows = e -> layout.data.gridlayout.rows;

           c_width  = 0;
           c_height = 0;
           for (i=0; i < e -> children_count; i++)
           {
             child = e -> children[i];
             _GUIGetMinimumSize(child, &w, &h); 

             c_width  = MAX(c_width,  w);
             c_height = MAX(c_height, h);
           }

           width  = (cols * c_width)  + (cols - 1) * hgap;
           height = (rows * c_height) + (rows - 1) * vgap;
         }
         break;

    // BORDER LAYOUT
    // +----------------------+
    // |        NORTH         |
    // +------+--------+------+
    // | WEST | CENTER | EAST |
    // +------+--------+------+
    // |        SOUTH         |
    // +----------------------+ extremity restrictions, central portion resizing

    case gui_border:
         {
           gui_element *panel[5] = { NULL };
           int16        h, h1, h2, h3;
           int16        w, w1, w2, w3;
           gui_element *child;

           for (i=0; i < e -> children_count; i++)
           {
             child = e -> children[i];
             panel[child -> position.data.borderlayout.zone] = child;
           }

           h1 = h2 = h3 = 0;
           w1 = w2 = w3 = 0;

           // determine the minimum sizes
           w = h = 0; _GUIGetMinimumSize(panel[GUI_NORTH],  &w, &h); h1 = h;
           w = h = 0; _GUIGetMinimumSize(panel[GUI_WEST],   &w, &h); w1 = w;
           h2 = MAX(h2, h);
           w = h = 0; _GUIGetMinimumSize(panel[GUI_CENTER], &w, &h); w2 = w; 
           h2 = MAX(h2, h);
           w = h = 0; _GUIGetMinimumSize(panel[GUI_EAST],   &w, &h); w3 = w;
           h2 = MAX(h2, h);
           w = h = 0; _GUIGetMinimumSize(panel[GUI_SOUTH],  &w, &h); h3 = h;

           // override the width and height for minimum size setting
           width  = w1 + w2 + w3 + 
                    ((w1 != 0) ? hgap : 0) + ((w3 != 0) ? hgap : 0);
           height = h1 + h2 + h3 + 
                    ((h1 != 0) ? vgap : 0) + ((h3 != 0) ? vgap : 0);
         }
         break;

    // STRETCH LAYOUT
    //             +----+----+
    //           / |    |    |  *components inside are NOT managed
    //         /   +----+----+
    //       /     |    |    |
    // +-+-+       +----+----+
    // +-+-+       |    |    |
    // +-+-+ ----- +----+----+ scalable area with ABS co-ordinates

    case gui_stretch:
         {
           width  = e -> layout.data.stretchlayout.width;
           height = e -> layout.data.stretchlayout.height;
         }
         break;

    default:
    case gui_none:
         break;
  }

  // allow for "insets"
  width  += e -> inset << 1;
  height += e -> inset << 1;

  if (w != NULL) *w = width;
  if (h != NULL) *h = height;
}

static boolean 
_GUI_panel_HandleEvent(gui_element *e, event *ev)
{
  boolean result;
  int16   i;
//GUI_GLOBALS_ACCESS;

  // default return value
  result = false;

  switch (ev -> eType)
  {
    case _penEvent:
         if (_GUIElementAtPosition(e, ev -> data.pen.x, ev -> data.pen.y))
         {
           for (i=0; i < e -> children_count; i++)
           {
             result |= _GUIElementHandleEvent(e -> children[i], ev); 
             if (result) break;
           }
         }
         break;

    default:
         break;
  }

  return result;
}

static boolean 
_GUI_label_HandleEvent(gui_element *e, event *ev)
{
//GUI_GLOBALS_ACCESS;

  return false;
}

static boolean 
_GUI_image_HandleEvent(gui_element *e, event *ev)
{
//GUI_GLOBALS_ACCESS;

  return false;
}

static boolean 
_GUI_button_HandleEvent(gui_element *e, event *ev)
{
  gui_button_info *info;
  boolean          result;
  coord            penX, penY;
  boolean          penDown;
  GUI_GLOBALS_ACCESS;

  // default return value
  result = false;

  // we need a reference to the "information" block
  info = (gui_button_info *)e -> extension;

  switch (ev -> eType)
  {
    case _penEvent:
         penX = ev -> data.pen.x; 
         penY = ev -> data.pen.y;

         if (_GUIElementAtPosition(e, penX, penY)) g_gui -> focus = e;
         else                                      g_gui -> focus = NULL;
         info -> pressed = (g_gui -> focus == e);
         g_gui -> item[e->type].fnDraw(e);

         result = info -> pressed;
         break;

    case _nilEvent:
         if (_InputSupported() & INPUT_STYLUS)
         {
           // get the current pen state
           _PenState(&penX, &penY, &penDown);
           if (_GUIElementAtPosition(e, penX, penY)) g_gui -> focus = e;
           else                                      g_gui -> focus = NULL;
           info -> pressed = (g_gui -> focus == e);

           // has pen been released?
           if (!penDown)
             info -> pressed = false;

           g_gui -> item[e->type].fnDraw(e);
         }
         break;

    default:
         break;
  }

  return result;
}

static boolean 
_GUI_imagebutton_HandleEvent(gui_element *e, event *ev)
{
  gui_imagebutton_info *info;
  boolean               result;
  coord                 penX, penY;
  boolean               penDown;
  GUI_GLOBALS_ACCESS;

  // default return value
  result = false;

  // we need a reference to the "information" block
  info = (gui_imagebutton_info *)e -> extension;

  switch (ev -> eType)
  {
    case _penEvent:
         penX = ev -> data.pen.x; 
         penY = ev -> data.pen.y;

         if (_GUIElementAtPosition(e, penX, penY)) g_gui -> focus = e;
         else                                      g_gui -> focus = NULL;
         info -> pressed = (g_gui -> focus == e);
         g_gui -> item[e->type].fnDraw(e);

         result = info -> pressed;
         break;

    case _nilEvent:
         if (_InputSupported() & INPUT_STYLUS)
         {
           // get the current pen state
           _PenState(&penX, &penY, &penDown);
           if (_GUIElementAtPosition(e, penX, penY)) g_gui -> focus = e;
           else                                      g_gui -> focus = NULL;
           info -> pressed = (g_gui -> focus == e);

           // has pen been released?
           if (!penDown)
             info -> pressed = false;

           g_gui -> item[e->type].fnDraw(e);
         }
         break;

    default:
         break;
  }

  return result;
}

/********************************* EOF ***********************************/
