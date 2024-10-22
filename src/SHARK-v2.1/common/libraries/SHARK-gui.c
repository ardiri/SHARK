/*************************************************************************
 *
 * Copyright 2002+ MobileWizardry
 * All rights reserved.
 *
 *************************************************************************/

/*
 * @(#)SHARK-gui.c
 */

#include "../../SHARK.h"

/*************************************************************************
 *
 * Configuration
 *
 *************************************************************************/

#define GUI_LIBRARY_ID  0x5f677569 // '_gui' = 0x5f677569

// -----------------------------------------------------------------------
// compile time options
// -----------------------------------------------------------------------

// GUI_MINIMUMSIZE
//
// _GUI_component_SetSize
//   -> this routine performs basic layout management to re-position the
//      components in the most appopriate manner based on a set of layout
//      constraints. setting this define allows control over if the size
//      of a component can ever drop below its minimum size
//
//      = 0 - resize purely on rules
//      = 1 - never allow a component to drop below GetMinimumSize()
//
// NOTE: setting to 1 can causes truncation in rendering of the display

   #define GUI_MINIMUMSIZE          1 // bounds never < GetMinimumSize()

/*************************************************************************
 *
 * Global Variables
 *
 *************************************************************************/

typedef struct _GUIGlobals
{
  void           *extension;

  boolean       (*fnGUIInitialize)(void);
  void          (*fnGUITerminate)(void);

  gui_face        face;
  void          (*fnGUIGetDefaultColors)(color *, color *);

  gui_element    *container;
  gui_element    *focus;

  struct
  {
    void        (*fnDraw)(gui_element *e);
    void        (*fnSetPosition)(gui_element *e, coord x, coord y);
    void        (*fnSetSize)(gui_element *e, int16 w, int16 h);
    void        (*fnGetMinimumSize)(gui_element *e, int16 *w, int16 *h);
    boolean     (*fnHandleEvent)(gui_element *e, event *ev);
  } item[GUI_TYPE_COUNT];

} _GUIGlobals;

#define GUI_GLOBALS_ACCESS \
  _GUIGlobals *g_gui = (_GUIGlobals *)_LibGetGlobals(GUI_LIBRARY_ID);

//------------------------------------------------------------------------
//                    --== GLOBALS ARE FORBIDDEN! ==--
//
// DAL may support the use of globals on some platforms, however, its not
// guaranteed that all the destination platforms allow the use of globals
// (variable or static data). use the GlobalsType / _LibGetGlobals() API.
//------------------------------------------------------------------------

/*************************************************************************
 *
 * Implementation
 *
 *************************************************************************/

static void    _GUIInitialize_RESIZE();
static void    _GUITerminate_RESIZE();

static boolean _GUIDriverConfiguration(gui_face face);
static boolean _GUIHandleEvent(event *e);
static void    _GUIGetDefaultColors(color *f, color *b);

static void    _GUIDraw(gui_element *e);
static boolean _GUIElementHandleEvent(gui_element *e, event *ev);
static boolean _GUIElementAtPosition(gui_element *e, coord x, coord y);
static void    _GUIGetElementPosition(gui_element *e, coord *x, coord *y);

boolean
_GUIInitialize(gui_face face)
{
  boolean      init_ok;
  _GUIGlobals *g_gui;

  // assume everything is ok
  init_ok = true;

  // lets see if we have already been initialized
  g_gui = (_GUIGlobals *)_LibGetGlobals(GUI_LIBRARY_ID);

  // entry condition
  if (g_gui != NULL) return false;

  // register the library
  if (_LibRegister(GUI_LIBRARY_ID, "_GUILibrary", (void *)_GUIHandleEvent))
  {
    // allocate globals memory for the library
    g_gui = (_GUIGlobals *)_MemPtrNew(sizeof(_GUIGlobals), false);
    init_ok &= (g_gui != NULL); if (!init_ok) goto GUI_INIT_ABORT;
    _LibSetGlobals(GUI_LIBRARY_ID, (void *)g_gui);

    // initialize the _GUI driver
    init_ok = _GUIDriverConfiguration(face);
    if (!init_ok) goto GUI_INIT_ABORT;
   
    // create the "global" container
    g_gui -> container = _GUINew(gui_panel, NULL);
    init_ok = (g_gui -> container != NULL); if (!init_ok) goto GUI_INIT_ABORT;
    g_gui -> focus = NULL;

    // initialize _Gui internally
    _GUIInitialize_RESIZE();
  }
  else
    init_ok = false;

GUI_INIT_ABORT:

  return init_ok;
}

void
_GUITerminate()
{
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if (g_gui == NULL) return;

  // shutdown the sub-GUI layer
  if (g_gui -> fnGUITerminate != NULL)
    g_gui -> fnGUITerminate();

  // dispose of the "global" container (this terminates everything)
  _GUIFree(g_gui -> container);
  g_gui -> container = NULL;

  // terminate _GUI internally
  _GUITerminate_RESIZE();

  // free the globals used
  _MemPtrFree(g_gui); g_gui = NULL;
  _LibSetGlobals(GUI_LIBRARY_ID, (void *)g_gui);

  // shutdown the library
  _LibShutdown(GUI_LIBRARY_ID);
}

gui_element *
_GUINew(gui_type type, void *info)
{
  gui_element *result;
  boolean      valid;
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if (g_gui == NULL) return NULL;

  // entry requirement #2 - must have valid info block
  if ((type != gui_panel) && (info == NULL)) return NULL;

  // default return value
  result = NULL;

  // lets allocate the gui_element structure
  result = (gui_element *)_MemPtrNew(sizeof(gui_element), false);
  if (result != NULL)
  {
    // lets assume it is valid
    valid = true;

    // what type of gui_element to create?
    _MemSet(result, sizeof(gui_element), 0);
    switch (type)
    {
      case gui_panel:
           break;

      case gui_label:
           valid &= (((gui_label_info *)info) -> label != NULL);
           break;

      case gui_image:
           valid &= (((gui_image_info *)info) -> image != NULL);
           break;

      case gui_button:
           valid &= (((gui_button_info *)info) -> label != NULL);
           break;

      case gui_imagebutton:
           valid &= (((gui_imagebutton_info *)info) -> image != NULL);
           break;

      case gui_checkbox:
           valid &= (((gui_checkbox_info *)info) -> label != NULL);
           break;

      case gui_list:
           valid &= (((gui_list_info *)info) -> list_items != NULL);
           break;

      case gui_scrollbar:
           valid &= (((gui_scrollbar_info *)info) -> val > 
                     ((gui_scrollbar_info *)info) -> min);
           valid &= (((gui_scrollbar_info *)info) -> val < 
                     ((gui_scrollbar_info *)info) -> max);
           valid &= (((gui_scrollbar_info *)info) -> page < 
                     ((gui_scrollbar_info *)info) -> max);
           break;

      case gui_textfield:
           valid &= (((gui_textfield_info *)info) -> buffer != NULL);
           valid &= (((gui_textfield_info *)info) -> size   >  0);
           break;

      case gui_textarea:
           valid &= (((gui_textarea_info *)info) -> buffer != NULL);
           valid &= (((gui_textarea_info *)info) -> size   >  0);
           valid &= (((gui_textarea_info *)info) -> lines  >  1);
           break;

      default:
           valid = false;
           break;
    }

    if (valid)
    {
      result -> type       = type;
      result -> extension  = (void *)info;
      _GUIGetDefaultColors(&result -> foreground, &result -> background);

      // now we created it, lets give it the default size
      _GUIGetMinimumSize(result, 
                         &result -> bounds.width, &result -> bounds.height);
    }
    else
    {
      _MemPtrFree(result); 
      result = NULL;    
    }
  }
 
  return result;
}

void
_GUIFree(gui_element *e)
{
  uint16 i;
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if ((g_gui == NULL) || (e == NULL)) return;

  // lets ensure all the children are removed
  if ((e -> children_count > 0) && (e -> children != NULL))
  {
    for (i=0; i < e -> children_count; i++)
      _GUIFree(e -> children[i]);

    _MemPtrFree(e -> children);
    e -> children_count = 0;
    e -> children = NULL;
  }

  _MemPtrFree(e);
}

void
_GUISetPosition(gui_element *e, coord x, coord y)
{
  GUI_GLOBALS_ACCESS;

  if ((g_gui == NULL) || (e == NULL)) return;

  // entry requirements #2 - ensure we dont have extra types
  if (e -> type >= GUI_TYPE_COUNT) return;
 
  // call the appropriate function
  g_gui -> item[e -> type].fnSetPosition(e, x, y);
}

void
_GUISetSize(gui_element *e, int16 width, int16 height)
{
  GUI_GLOBALS_ACCESS;

  if ((g_gui == NULL) || (e == NULL)) return;

  // entry requirements #2 - ensure we dont have extra types
  if (e -> type >= GUI_TYPE_COUNT) return;
 
  // call the appropriate function
  g_gui -> item[e -> type].fnSetSize(e, width, height);
}

void 
_GUISetLayout(gui_element *e, gui_layout *layout)
{
  GUI_GLOBALS_ACCESS;

  if ((g_gui == NULL) || (layout == NULL)) return;

  // are we defining the "top container"
  if (e == NULL) e = g_gui -> container;

  // we can only do this to containers
  if (e -> type == gui_panel)
  {
    _MemMove(&e -> layout, layout, sizeof(gui_layout));

    // lets ensure that the item is at least drawn
    _GUISetSize(e, e -> bounds.width, e -> bounds.height);
  }
}

void
_GUIGetMinimumSize(gui_element *e, int16 *width, int16 *height)
{
  GUI_GLOBALS_ACCESS;

  if ((g_gui == NULL) || (e == NULL)) return;

  // entry requirements #2 - ensure we dont have extra types
  if (e -> type >= GUI_TYPE_COUNT) return;
 
  // call the appropriate function
  g_gui -> item[e -> type].fnGetMinimumSize(e, width, height);
}

boolean
_GUIInsert(gui_element *e, gui_element *item)
{
  boolean       result;
  gui_element **children_old;
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if ((g_gui == NULL) || (item == NULL)) return false;

  // default return value
  result = false;

  // are we defining the "top container"
  if (e == NULL) e = g_gui -> container;

  // we can only add items to containers
  if (e -> type == gui_panel)
  {
    // insert the gui_element into the list of children
    children_old = e -> children;
    e -> children = (gui_element **)
      _MemPtrNew((e -> children_count+1) * sizeof(gui_element *), false);
    if (e -> children != NULL)
    {
      item -> parent = e;

      if (children_old != NULL)
      {
        // we need to copy the old list on top of the new one
        _MemMove(e -> children, children_old, 
                 (e -> children_count * sizeof(gui_element *)));
        _MemPtrFree(children_old);
        children_old = NULL;
      }
      e -> children[e -> children_count] = item; 
      e -> children_count++;

      result = true;
    }
  }

  return result;
}

boolean
_GUIInsertAtPosition(gui_element *e, gui_element *item, gui_position *position)
{
  boolean result;
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if ((g_gui == NULL) || (item == NULL) || (position == NULL)) return false;

  // default return value
  result = false;

  // set the parameter and insert like usual
  _MemMove(&item -> position, position, sizeof(gui_position));
  result = _GUIInsert(e, item);

  return result;
}

void
_GUIRemove(gui_element *e, gui_element *item)
{
  uint16        i, cnt;
  boolean       found;
  gui_element **children_old;
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if ((g_gui == NULL) || (item == NULL)) return;

  // are we defining the "top container"
  if (e == NULL) e = g_gui -> container;
 
  // we can only remove items from containers
  if (e -> type == gui_panel)
  {
    // lets find the item within the list
    if ((e -> children_count > 0) && (e -> children != NULL))
    {
      found = false;
      for (i=0; i < e -> children_count; i++)
      {
        found = (e -> children[i] == item); 
        if (found) break;
      }  

      if (found)
      {
        if (e -> children_count < 2)
        {
          _MemPtrFree(e -> children);
          e -> children = NULL;
          e -> children_count = 0;
        }
        else
        {
          children_old = e -> children;
          e -> children = (gui_element **)
            _MemPtrNew((e -> children_count-1) * sizeof(gui_element *), false);
          if (e -> children != NULL)
          {
            _MemMove(e -> children, children_old, (i * sizeof(gui_element)));
            if (i != (e -> children_count-1)) 
            {
               cnt = e -> children_count - i - 1;
               _MemMove(e -> children + i, children_old + i + 1, 
                        (cnt * sizeof(gui_element)));
            }
  
            _MemPtrFree(children_old);
            e -> children_count = e -> children_count - 1;
          }
        }
      } 
    } 
  } 
}

void
_GUILayout()
{
  coord width, height;
  GUI_GLOBALS_ACCESS;

  // get the display boundaries
  _GfxGetWindowInfo(_GfxGetDisplayWindow(), NULL, &width, &height, NULL);
 
  // reset the container values
  _GUISetPosition(g_gui -> container, 0, 0);
  _GUISetSize(g_gui -> container, width, height);

  // repaint the "gui"
  _GUIRepaint();
}

void 
_GUIRepaint()
{
  GUI_GLOBALS_ACCESS;

  _GUIDraw(g_gui -> container);
  _GfxRepaint();
}

static void
_GUIInitialize_RESIZE()
{
//GUI_GLOBALS_ACCESS;

  // layout the display
  _GUILayout();
}

static void
_GUITerminate_RESIZE()
{
//GUI_GLOBALS_ACCESS;
}

#include "SHARK-gui-core.inc"
#include "SHARK-gui-basic.inc"

static boolean
_GUIDriverConfiguration(gui_face face)
{
  boolean result;
  GUI_GLOBALS_ACCESS;

  // default return value
  result = false;

  // we need to store this information for later use
  g_gui -> face = face;

  // we need to confirm that we can actually handle this configuration
  switch (face)
  {
    case gui_basic:
 
         // initialize the _GUI routines
         g_gui -> fnGUIInitialize       = (void *)_GUI_B_Initialize;
         g_gui -> fnGUITerminate        = (void *)_GUI_B_Terminate;

         g_gui -> fnGUIGetDefaultColors = (void *)_GUI_B_GetDefaultColors;

#define DEFINE_GUI_FUNCTION_HANDLEEVENT(o)        \
  g_gui -> item[gui_ ## o ].fnHandleEvent    = (void *)_GUI_ ## o ## _HandleEvent
#define DEFINE_GUI_FUNCTION_DRAW(o)               \
  g_gui -> item[gui_ ## o ].fnDraw           = (void *)_GUI_B_ ## o ## _Draw
#define DEFINE_GUI_FUNCTION_SETPOSITION(o)        \
  g_gui -> item[gui_ ## o ].fnSetPosition    = (void *)_GUI_component_SetPosition
#define DEFINE_GUI_FUNCTION_SETSIZE(o, a)         \
  g_gui -> item[gui_ ## o ].fnSetSize        = (void *)_GUI_ ## a ## _SetSize
#define DEFINE_GUI_FUNCTION_GETMINIMUMSIZE(o)     \
  g_gui -> item[gui_ ## o ].fnGetMinimumSize = (void *)_GUI_B_ ## o ## _GetMinimumSize

         DEFINE_GUI_FUNCTION_HANDLEEVENT(panel);
         DEFINE_GUI_FUNCTION_DRAW(panel);
         DEFINE_GUI_FUNCTION_SETPOSITION(panel);
         DEFINE_GUI_FUNCTION_SETSIZE(panel, container);
         DEFINE_GUI_FUNCTION_GETMINIMUMSIZE(panel);

         DEFINE_GUI_FUNCTION_HANDLEEVENT(label);
         DEFINE_GUI_FUNCTION_DRAW(label);
         DEFINE_GUI_FUNCTION_SETPOSITION(label);
         DEFINE_GUI_FUNCTION_SETSIZE(label, component);
         DEFINE_GUI_FUNCTION_GETMINIMUMSIZE(label);

         DEFINE_GUI_FUNCTION_HANDLEEVENT(image);
         DEFINE_GUI_FUNCTION_DRAW(image);
         DEFINE_GUI_FUNCTION_SETPOSITION(image);
         DEFINE_GUI_FUNCTION_SETSIZE(image, component);
         DEFINE_GUI_FUNCTION_GETMINIMUMSIZE(image);

         DEFINE_GUI_FUNCTION_HANDLEEVENT(button);
         DEFINE_GUI_FUNCTION_DRAW(button);
         DEFINE_GUI_FUNCTION_SETPOSITION(button);
         DEFINE_GUI_FUNCTION_SETSIZE(button, component);
         DEFINE_GUI_FUNCTION_GETMINIMUMSIZE(button);

         DEFINE_GUI_FUNCTION_HANDLEEVENT(imagebutton);
         DEFINE_GUI_FUNCTION_DRAW(imagebutton);
         DEFINE_GUI_FUNCTION_SETPOSITION(imagebutton);
         DEFINE_GUI_FUNCTION_SETSIZE(imagebutton, component);
         DEFINE_GUI_FUNCTION_GETMINIMUMSIZE(imagebutton);

#undef DEFINE_GUI_FUNCTION_SETPOSITION
#undef DEFINE_GUI_FUNCTION_SETSIZE
#undef DEFINE_GUI_FUNCTION_GETMINIMUMSIZE
#undef DEFINE_GUI_FUNCTION_DRAW
#undef DEFINE_GUI_FUNCTION_HANDLEEVENT

         break;

    default:
         break;
  }

  // initialize the sub-gui layer
  if (g_gui -> fnGUIInitialize != NULL)
    result = g_gui -> fnGUIInitialize();

  return result;
}

static boolean
_GUIHandleEvent(event *e)
{
  boolean processed;
  GUI_GLOBALS_ACCESS;

  // default return value
  processed = false;

  // entry requirement (must call _GUIInitialize())
  if (g_gui == NULL) return processed;

  switch (e -> eType)
  {
    case _penEvent:
         if (!_GUIElementHandleEvent(g_gui -> focus, e))
           processed = _GUIElementHandleEvent(g_gui -> container, e);
         break;

    case _keyEvent:
         switch (e -> data.key.chr)
         {
           case _key_gfx_change:
                _GUITerminate_RESIZE();
                _GUIInitialize_RESIZE();
                break;

           default:
                processed = _GUIElementHandleEvent(g_gui -> focus, e);
                break;
         }
         break;

    case _nilEvent:
         processed = _GUIElementHandleEvent(g_gui -> focus, e);
         break;

    default:
         break;
  }

  return processed;
}

static void
_GUIGetDefaultColors(color *fore, color *back)
{
  GUI_GLOBALS_ACCESS;

  // entry requirement (must call _GUIInitialize())
  if (g_gui == NULL) return;

  // initialize the sub-gui layer
  if (g_gui -> fnGUIGetDefaultColors != NULL)
    g_gui -> fnGUIGetDefaultColors(fore, back);
}

static void
_GUIDraw(gui_element *e)
{
  GUI_GLOBALS_ACCESS;

  // entry requirement 
  if (e == NULL) return;

  // entry requirements #2 - ensure we dont have extra types
  if (e -> type >= GUI_TYPE_COUNT) return;

  // call the appropriate function
  g_gui -> item[e -> type].fnDraw(e);
}

static boolean
_GUIElementHandleEvent(gui_element *e, event *ev)
{
  boolean result;
  GUI_GLOBALS_ACCESS;

  // entry requirements
  if ((e == NULL) || (ev == NULL)) return false;

  // entry requirements #2 - ensure we dont have extra types
  if (e -> type >= GUI_TYPE_COUNT) return false;
  
  // call the appropriate function
  result = g_gui -> item[e -> type].fnHandleEvent(e, ev);

  return result;
}

static boolean
_GUIElementAtPosition(gui_element *e, coord x, coord y)
{
  boolean result;
  coord   _x, _y;
//GUI_GLOBALS_ACCESS;
  
  // default return value
  result = false;

  _GUIGetElementPosition(e, &_x, &_y);
  result = ((x >= _x) && (x <= (_x + e -> bounds.width)) &&
            (y >= _y) && (y <= (_y + e -> bounds.height)));

  return result;
}

static void
_GUIGetElementPosition(gui_element *e, coord *x, coord *y)
{
  coord        _x, _y;
  gui_element *p;
//GUI_GLOBALS_ACCESS;

  // default values
  _x = 0;
  _y = 0;

  // do we have an element (we should)
  if (e != NULL)
  {
    // we can start by the offset within the parent
    _x = e -> bounds.x;
    _y = e -> bounds.y;

    // go through all the parents, adding appropriate offsets
    p = e -> parent;
    while (p != NULL) 
    {
      _x += p -> bounds.x;
      _y += p -> bounds.y;
 
      p = p -> parent;
    }
  }

  if (x != NULL) *x = _x;
  if (y != NULL) *y = _y;
}

/********************************* EOF ***********************************/
