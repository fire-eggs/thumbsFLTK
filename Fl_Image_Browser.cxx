//
// Image browser widget methods for the Fast Light Tool Kit (FLTK).
//
// Copyright 2002-2006 by Michael Sweet.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Contents:
//
//   Fl_Image_BrowserV::Fl_Image_BrowserV()     - Create a new image display widget.
//   Fl_Image_BrowserV::~Fl_Image_BrowserV()    - Destroy an image display widget.
//   Fl_Image_BrowserV::draw()                 - Draw the image display widget.
//   Fl_Image_BrowserV::handle()               - Handle events in the widget.
//   Fl_Image_BrowserV::resize()               - Resize the image display widget.
//   Fl_Image_BrowserV::scrollbar_cb()         - Update the display based on the scrollbar position.
//   Fl_Image_BrowserV::update_scrollbar()     - Update the scrollbar.
//   Fl_Image_BrowserV::add()                  - Add an image to the browser.
//   Fl_Image_BrowserV::clear()                - Remove all items from the browser.
//   Fl_Image_BrowserV::delete_item()          - Delete an item from the browser.
//   Fl_Image_BrowserV::insert_item()          - Insert an item in the browser.
//   Fl_Image_BrowserV::move_item()            - Move an image in the browser.
//   Fl_Image_BrowserV::load()                 - Load all images in a directory.
//   Fl_Image_BrowserV::load_item()            - Load the image for an item.
//   Fl_Image_BrowserV::remove()               - Remove an item.
//   Fl_Image_BrowserV::ITEM::save_thumbnail() - Save the thumbnail image.
//   Fl_Image_BrowserV::select()               - Select an image.
//

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <FL/filename.H>

#include "Fl_Image_Browser.H"


//
// 'Fl_Image_BrowserV::Fl_Image_BrowserV()' - Create a new image display widget.
//

Fl_Image_BrowserV::Fl_Image_BrowserV(
    int        X,			// I - X position
    int        Y,			// I - Y position
    int        W,			// I - Width
    int        H,			// I - Height
    const char *L)			// I - Label string
  : Fl_Group(X, Y, W, H, L),
    //scrollbar_(X, Y + H - SBWIDTH, W, SBWIDTH)
    scrollbar_(X + W - SBWIDTH, Y, SBWIDTH, H)
{
  end();

  _itemList = new ItemList();
  
  box(FL_DOWN_BOX);
  selection_color(FL_SELECTION_COLOR);

  selected_    = -1;
  textfont_    = FL_HELVETICA;
  textsize_    = FL_NORMAL_SIZE;

  //scrollbar_.type(FL_HORIZONTAL);
  scrollbar_.type(FL_VERTICAL);
  scrollbar_.callback(scrollbar_cb, this);

  _numLines = 2; // KBR NOTE *must* be set before resize
  
  resize(X, Y, W, H);
}


//
// 'Fl_Image_BrowserV::~Fl_Image_BrowserV()' - Destroy an image display widget.
//

Fl_Image_BrowserV::~Fl_Image_BrowserV()
{
  _itemList->clear();
  // unnecessary widget update cause we're shutting down clear();
  delete _itemList;
}


void Fl_Image_BrowserV::drawGrid(int X, int Y, int W, int H)
{
  int ts = thumbSize();
  
  for (int i = 0; i < _itemList->count(); i ++)
  {
    ItemList::ITEM *item = _itemList->get(i);

    if (!item || !item->thumbnail)
      continue; // TODO label drawing, placeholder drawing
      
    int yoff = item->_y - scrollbar_.value();
            
//    int row = i / _numLines;
    
//    int yoff = row * ts - scrollbar_.value();
//    int xoff = i % _numLines * ts; // : 0;
    
    if (yoff < -ts || yoff >= H)
      continue;

    Fl_Color bg;

    if (item->selected)
      bg = item->changed ?
               fl_color_average(FL_RED, selection_color(), 0.5) :
               selection_color();
    else
      bg = item->changed ? FL_RED : FL_WHITE; // TODO chosen background color?
    
//    int ts = thumbSize();

    // TODO drawing double margin horizontally [once on right of image x, once on left of image x+1]
    // TODO margins setting
    // selected item is drawn smaller so selection color will show. otherwise, no margin.
    
    //int margin = 20; // TODO original margins
    //int drawsize = item->selected ? ts - margin : ts;
    //int delta    = item->selected ? 10 : 0;
    
    int drawsize = item->selected ?  ts - 10 : ts;
    int delta = item->selected ? 5 : 0;

    int tW = drawsize;
    int tH = tW * item->thumbnail->h() / item->thumbnail->w();
    
    int xoff = item->_x;
    
    if (bg != FL_WHITE) // TODO chosen background color?
    {
        fl_color(bg);
        fl_rectf(X + xoff + 1, Y + yoff + 1, ts - 4, ts+4); // H - 4);
    }
    
    //if (item->thumbnail->h() > item->thumbnail->w())
    if (item->thumbnail->h() < item->thumbnail->w())
    {
        tH = drawsize;
        tW = tH * item->thumbnail->w() / item->thumbnail->h();
    }        
    
    auto tmpImage = item->thumbnail->copy(tW,tH);
        
    // grid mode is centered+cropped: draw anti-proportional then take center(drawsize,drawsize)

    tmpImage->draw(X + xoff + delta, Y+yoff+delta,
                drawsize, drawsize, (tW - drawsize) / 2, (tH - drawsize) / 2);
    tmpImage->release();

#if 0 // KBR draw no label      
    fl_color(fl_contrast(FL_BLACK, bg));
    fl_font(textfont(), textsize());
    fl_draw(item->label, X + xoff + 2, Y, ITEMWIDTH - 4, H - 2,
            (Fl_Align)(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM |
	               FL_ALIGN_CLIP | FL_ALIGN_WRAP));
#endif	               
  }
}

void Fl_Image_BrowserV::drawStack(int X, int Y, int W, int H)
{
  int ts = thumbSize();
  
  for (int i = 0; i < _itemList->count(); i ++)
  {
    ItemList::ITEM *item = _itemList->get(i);

    if (!item || !item->thumbnail)
      continue;

    int xoff = item->_x;
    int yoff = item->_y - scrollbar_.value();
    
//    int row = i / _numLines;
    
//    int yoff = row * ts - scrollbar_.value();
//    int xoff = i % _numLines * ts; // : 0;
    
    //if (yoff < -ts || yoff >= H)
    //  continue;
    if (yoff >= H)
      continue;

    Fl_Color bg;

    if (item->selected)
      bg = item->changed ?
               fl_color_average(FL_RED, selection_color(), 0.5) :
               selection_color();
    else
      bg = item->changed ? FL_RED : FL_WHITE; // TODO chosen background color?
    
        // TODO drawing double margin horizontally [once on right of image x, once on left of image x+1]
        // TODO margins setting
        // selected item is drawn smaller so selection color will show. otherwise, no margin.
        
        //int margin = 20; // TODO original margins
        //int drawsize = item->selected ? ts - margin : ts;
        //int delta    = item->selected ? 10 : 0;
        
        int drawsize = item->selected ?  ts - 10 : ts;
        int delta = item->selected ? 5 : 0;

        int tW = drawsize;
        int tH = tW * item->thumbnail->h() / item->thumbnail->w();

#if 0        
        if (i >= _numLines)
        {
            ItemList::ITEM *temAbove = _itemList->get(i-_numLines);
            item->_y = temAbove->_y + temAbove->_h;
            yoff = item->_y;
        }
#endif

        if (bg != FL_WHITE) // TODO chosen background color?
        {
            fl_color(bg);
            fl_rectf(X + xoff, Y + yoff, ts + 2, tH + 2); // H - 4);
        }
        
        auto tmpImage = item->thumbnail->copy(tW,tH);
        
        // TODO yoff depends on thumbnails above me
        tmpImage->draw(X + xoff + delta, Y+yoff+delta, tW, tH);
        
        tmpImage->release();

#if 0 // KBR draw no label      
    fl_color(fl_contrast(FL_BLACK, bg));
    fl_font(textfont(), textsize());
    fl_draw(item->label, X + xoff + 2, Y, ITEMWIDTH - 4, H - 2,
            (Fl_Align)(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM |
	               FL_ALIGN_CLIP | FL_ALIGN_WRAP));
#endif	               
  }
    
}



//
// 'Fl_Image_BrowserV::draw()' - Draw the image display widget.
//

void
Fl_Image_BrowserV::draw()
{

  int X = x() + Fl::box_dx(box());
  int Y = y() + Fl::box_dy(box());
  //int W = w() - Fl::box_dw(box());
  int W = w() - Fl::box_dw(box()) - SBWIDTH;
  //int H = h() - Fl::box_dh(box()) - SBWIDTH;
  int H = h() - Fl::box_dh(box());

  if (damage() & FL_DAMAGE_SCROLL)
    fl_push_clip(X, Y, W, H);

  //draw_box(box(), x(), y(), w(), h() - SBWIDTH, color());
  draw_box(box(), x(), y(), w() - SBWIDTH, h(), color());

  if (!(damage() & FL_DAMAGE_SCROLL))
    fl_push_clip(X, Y, W, H);

#ifdef DEBUG
  printf("scrollbar_.value() = %d\n", scrollbar_.value());
#endif // DEBUG

    if (_stackMode)
        drawStack(X, Y, W, H);
    else
        drawGrid(X, Y, W, H);

  fl_pop_clip();

  if (damage() & FL_DAMAGE_SCROLL)
    update_child(scrollbar_);
  else
    draw_child(scrollbar_);
}


//
// 'Fl_Image_BrowserV::handle()' - Handle events in the widget.
//

int					// O - 1 if event handled, 0 otherwise
Fl_Image_BrowserV::handle(int event)	// I - Event
{
  
  switch (event)
  {
    case FL_PUSH :
    {
        // get mouse position
        int X = Fl::event_x() - x(); // - Fl::box_dx(box());
        int Y = Fl::event_y() - y();

        // thumb values don't take scroll position into account
        Y += scrollbar_.value(); // vertical
        int sel = _itemList->find(X, Y); // currently selected item
        
        if (X >= w() - SBWIDTH)
            break;

        pushed_ = sel; // TODO for drag?

        if (sel >= 0)
        {
            if (Fl::event_state() & FL_CTRL)
                _itemList->toggleSelect(sel); // TODO selected_ state?
            else if (Fl::event_state() & FL_SHIFT)
            {
                if (selected_ < 0)
                    selected_ = 0;
                _itemList->selectRange(sel, selected_);
            }
            else if (!_itemList->isSelected(sel))
            {
                _itemList->select(sel); // select only said item
                selected_ = sel;
            }

            damage(FL_DAMAGE_SCROLL);
            take_focus();

        }
        return (1);
    }
/*    
    case FL_DRAG :
        if (X < 0)
	{
	  // Scroll left
	  set_scrollbar(scrollbar_.value() - ts / 10);
	}
	else if (X > w())
	{
	  // Scroll right
	  set_scrollbar(scrollbar_.value() + ts / 10);
	}

	if (!(Fl::event_state() & (FL_CTRL | FL_SHIFT)) &&
            pushed_ >= 0 && sel != pushed_)
	{
          if (sel > pushed_)
	  {
	    // Move items right...
	    if (items_[sel] && items_[sel]->selected)
	    {
	      // Find the end of the selection...
	      while (sel < num_items_ && items_[sel]->selected)
		sel ++;

              sel ++;
	    }

	    if (sel < num_items_)
	    {
	      for (int i = num_items_ - 1; i >= 0; i --)
            if (items_[i] && items_[i]->selected)
              move_item(i, sel);
	    }
	  }
	  else
	  {
	    // Move items left...
	    for (int i = 0; i < num_items_; i ++)
	      if (items_[i] && items_[i]->selected)
	      {
	        move_item(i, sel);
            sel ++;
	      }
	  }

      pushed_ = sel;
	  set_changed();
	  do_callback();
	  clear_changed();
	}

	damage(FL_DAMAGE_SCROLL);
        return (1);
*/

/* TODO selection happening on push, why is this necessary? drag-and-release?
    case FL_RELEASE :
    {
  int X = Fl::event_x() - x(); // - Fl::box_dx(box());
  int Y = Fl::event_y() - y();

  int sel = _itemList->find(X, Y); // currently selected item
        
        if (sel < 0)
        {
            if (!(Fl::event_state() & (FL_CTRL | FL_SHIFT)))
            {
                _itemList->clearSelect();
            }
        }
        else if (!(Fl::event_state() & (FL_CTRL | FL_SHIFT)) && pushed_ == sel)
        {
            _itemList->select(sel);
        }

        selected_ = sel;

        damage(FL_DAMAGE_SCROLL);

        do_callback();
        clear_changed();
        return (1);
    }
*/
    case FL_SHORTCUT :
    case FL_KEYDOWN :
      if (Fl::event_key() == FL_Left && selected_ > 0)
        selected_ --;
      else if (Fl::event_key() == FL_Right && selected_ < (_itemList->count() - 1))
	    selected_ ++;
	  else
        return 1; // do NOT pass the keystroke to Fl_Group
	    //break;

      if (Fl::event_state() & FL_SHIFT)
      {
          _itemList->forceSelect(selected_); // add to selected
      }
	  else
	  {
          _itemList->select(selected_); // only the one selected
	  }

      make_visible(selected_);

      do_callback();
	  return (1);

    case FL_FOCUS :
    case FL_UNFOCUS :
        return (1);
  }

  return Fl_Group::handle(event);
}


//
// 'Fl_Image_BrowserV::resize()' - Resize the image display widget.
//

void
Fl_Image_BrowserV::resize(int X,		// I - X position
                        int Y,		// I - Y position
            int W,		// I - Width
            int H)		// I - Height
{
  Fl_Widget::resize(X, Y, W, H);

  //scrollbar_.resize(X, Y + H - SBWIDTH, W, SBWIDTH);  // horizontal
  scrollbar_.resize(X + W - SBWIDTH, Y, SBWIDTH, H);    // vertical

  recalc();
  //update_scrollbar();

  redraw();
}


//
// 'Fl_Image_BrowserV::scrollbar_cb()' - Update the display based on the scrollbar position.
//

void
Fl_Image_BrowserV::scrollbar_cb(
    Fl_Widget *,			// I - Widget (not used)
    void      *d)			// I - Image browser
{
  Fl_Image_BrowserV	*widget = (Fl_Image_BrowserV *)d;

  widget->damage(FL_DAMAGE_SCROLL);
}


//
// 'Fl_Image_BrowserV::set_scrollbar()' - Set the scrollbar position.
//

void
Fl_Image_BrowserV::set_scrollbar(int scrollPos)	// I - New scroll position
{
  int numItems = _itemList->count();
  
  if (numItems < 1)
  {
    scrollbar_.value(0, 1, 0, 1); // empty
    scrollbar_.deactivate();
    return;
  }
  
  //ItemList::ITEM *lastTem = _itemList->get(numItems-1);
  
  //int maxPos = w() - Fl::box_dw(box()); // horizontal version
  // int fullExtent = lastTem->_x + _lastTem->_w;
  
  // Vertical version
  //int fullExtent = lastTem->_y + lastTem->_h;
  int fullExtent = _maxExtent;
  int maxPos  = h() - Fl::box_dh(box());
  int numMarches = 1;
    
/*  
  int currentSize = thumbSize();
  int numItems = _itemList->count();
  //int numMarches = (int)((float)numItems / _numLines + 0.5);
  int numMarches = (numItems + _numLines - 1) / _numLines; // round up : # rows or # columns
  int fullExtent = numMarches * currentSize;
*/  
  
  
  if (numMarches)
  {
    if (scrollPos < 0 || fullExtent <= maxPos)
      scrollPos = 0;
    else if (scrollPos > (fullExtent - maxPos))
      scrollPos = fullExtent - maxPos;

    scrollbar_.value(scrollPos, maxPos, 0, fullExtent);
    scrollbar_.linesize(maxPos / 2);
  }
  else
    scrollbar_.value(0, 1, 0, 1); // empty

  if (fullExtent <= maxPos) // everything fully visible
    scrollbar_.deactivate();
  else
    scrollbar_.activate();
}


//
// 'Fl_Image_BrowserV::update_scrollbar()' - Update the scrollbar.
//

void
Fl_Image_BrowserV::update_scrollbar()
{
  set_scrollbar(scrollbar_.value());
}


//
// 'Fl_Image_BrowserV::add()' - Add an image to the browser.
//

void
Fl_Image_BrowserV::add(
    const char      *filename,		// I - File to add
    Fl_Shared_Image *img)		// I - Cached image data
{
  struct stat	fileinfo;		// Information about file


  // Only add non-empty or cached files!  
  if (img || (!stat(filename, &fileinfo) && fileinfo.st_size))
  {
    _itemList->insert_item(filename, img); // add to end of list
    
    recalc();
    //update_scrollbar();
    
    set_changed();
    do_callback();
    clear_changed();
    damage(FL_DAMAGE_SCROLL);
  }
}


//
// 'Fl_Image_BrowserV::clear()' - Remove all items from the browser.
//

void
Fl_Image_BrowserV::clear()
{
  _itemList->clear();
  update_scrollbar();
  clear_changed();
  damage(FL_DAMAGE_SCROLL);
}


//
// 'Fl_Image_BrowserV::load()' - Load all images in a directory.
//

void
Fl_Image_BrowserV::load(
    const char *dirname)		// I - Directory to load
{
  int		num_files;		// Number of files in directory
  dirent	**files;		// Files in directory
  char		absdir[512],		// Absolute directory path
		filename[1024];		// Absolute filename path


  fl_filename_absolute(absdir, sizeof(absdir), dirname);

  num_files = fl_filename_list(dirname, &files);

  if (num_files > 0)
  {
    window()->cursor(FL_CURSOR_WAIT);

    for (int i = 0; i < num_files; i ++)
    {
      snprintf(filename, sizeof(filename), "%s/%s", absdir, files[i]->d_name);

      bool isNotFound = _itemList->find(filename) == -1;
      
      // Import all supported file formats *except* PPM to avoid cached
      // raw image files...
      if (isNotFound && !fl_filename_isdir(filename) && 
          fl_filename_match(files[i]->d_name,
	                    "*.{arw,avi,bay,bmp,bmq,cr2,crw,cs1,dc2,dcr,dng,"
			    "erf,fff,hdr,jpg,k25,kdc,mdc,mos,nef,orf,pcd,pef,"
			    "png,pxn,raf,raw,rdc,sr2,srf,sti,tif,x3f}"))
      {
        if (window()->shown())
	{
          int xx, yy;
	  int ww, hh;

	  window()->make_current();

	  fl_font(textfont(), textsize());

	  ww = w() / 2;
	  hh = textsize() + 20;
	  xx = x() + w() / 4;
	  yy = y() + (h() - SBWIDTH - hh) / 2;

          if (i > 0)
	  {
            fl_push_clip(xx, yy, ww * i / (num_files - 1), hh);
	    draw_box(FL_UP_BOX, xx, yy, ww, hh, selection_color());
	    fl_color(fl_contrast(FL_BLACK, selection_color()));
	    fl_draw(files[i]->d_name, xx, yy, ww, hh, FL_ALIGN_CENTER);
	    fl_pop_clip();
	  }

          if (i < (num_files - 1))
	  {
            fl_push_clip(xx + ww * i / (num_files - 1), yy,
	        	 ww - ww * i / (num_files - 1), hh);
	    draw_box(FL_UP_BOX, xx, yy, ww, hh, color());
	    fl_color(fl_contrast(FL_BLACK, color()));
	    fl_draw(files[i]->d_name, xx, yy, ww, hh, FL_ALIGN_CENTER);
	    fl_pop_clip();
	  }

          Fl::flush();
	}

	add(filename);

#if 0 // KBR don't move scrollbar to end during insert    
    int W = w() - Fl::box_dw(box());
    int X = num_items_ * ITEMWIDTH - W;
	if (X < 0)
	  X = 0;

        scrollbar_.value(X, W, 0, num_items_ * ITEMWIDTH);
#endif

        Fl::check();
      
      }
      free(files[i]);
    }

    free(files);

    window()->cursor(FL_CURSOR_DEFAULT);
    
    set_scrollbar(0);
    
//    int zoom = 1; // TODO more than one thumbnail row
//    int tSize = (h() - SBWIDTH) / zoom;
//    scrollbar_.value(0, w() - Fl::box_dw(box()), 0, num_items_ * tSize); // TODO don't have _thumbSize at this time!
    
  }
}



//
// 'Fl_Image_BrowserV::remove()' - Remove an item.
//

void
Fl_Image_BrowserV::remove(int i)		// I - Index to remove
{
  _itemList->delete_item(i);
  update_scrollbar();
  redraw();
}



//
// 'Fl_Image_BrowserV::select()' - Select an image.
//

void
Fl_Image_BrowserV::select(int i)		// I - Index
{

  if (_itemList->outOfRange(i))
      return;
  
  _itemList->select(i);
  selected_ = i;
  make_visible(i);
}


//
// 'Fl_Image_BrowserV::make_visible()' - Make an image visible.
//

void
Fl_Image_BrowserV::make_visible(int i)	// I - Index
{

  if (_itemList->outOfRange(i))
    return;

  int column = i / _numLines;
  int X = column * thumbSize();

  // The current scroll position of the right side of widget
  int currScrollMax = scrollbar_.value() + w() - Fl::box_dw(box());
  
  if (X < scrollbar_.value() || X >= currScrollMax)
    set_scrollbar(X);
  
  // Positioning for a partially visible thumb: only move scrollbar
  // a bit, not force all the way [jarring jump].
  // TODO works only when scrolling right, needs scroll left version?
  if (X + thumbSize() >= currScrollMax)
      set_scrollbar(X - w()/2);

  damage(FL_DAMAGE_SCROLL);
}

// Stack/grid mode has been changed. Or number of lines has been changed.
// Recalculate each thumb's
// dimensions. This sets the scrollbar max extent.
//
void Fl_Image_BrowserV::recalc()
{
    _maxExtent = 0;
    
    int ts = thumbSize();

    for (int i = 0; i < _itemList->count(); i++)
    {
        ItemList::ITEM *tem = _itemList->get(i);
        if (!tem || !tem->thumbnail)
            continue; 

        int xoff = i % _numLines * ts; // vertical
        
        if (_stackMode)
        {
            int tW = ts;
            int tH = tW * tem->thumbnail->h() / tem->thumbnail->w();
                    
            tem->_x = xoff;
            tem->_w = tW;
            tem->_h = tH;
            tem->_y = 0;

            if (i >= _numLines)
            {
                ItemList::ITEM *temAbove = _itemList->get(i-_numLines);
                tem->_y = temAbove->_y + temAbove->_h;
            }
        }
        else
        {
            // Each thumb is the same size
            int row = i / _numLines;
            
            int yoff = row * ts;
            int xoff = i % _numLines * ts;
            
            tem->_x = xoff;
            tem->_y = yoff;
            tem->_w = ts;
            tem->_h = ts;
        }
        
        printf("%d: %d\n", _stackMode, tem->_y + tem->_h);
        int newval = tem->_y + tem->_h;
        // In stack mode, the "last" thumb might not be the tallest
        _maxExtent = newval > _maxExtent ? newval : _maxExtent;
    }
    
    printf("-%d: %d-\n", _stackMode, _maxExtent);
    update_scrollbar();
}

void Fl_Image_BrowserV::numLines(int val) 
{ 
    if (_numLines == val) 
        return; 
    _numLines = val; 
    recalc(); 
    redraw();
    //resize(x(),y(),w(),h()); 
}

void Fl_Image_BrowserV::setStackMode(bool val) 
{ 
    if (_stackMode == val) 
        return; 
    _stackMode = val; 
    recalc(); 
    redraw(); 
}
