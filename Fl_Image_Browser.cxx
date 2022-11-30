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

#include "Fl_Image_Browser.H"
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <FL/filename.H>

// TODO why is fl_strlcpy not resolved via libfltk?

size_t                          /* O - Length of string */
fl_strlcpy(char       *dst,     /* O - Destination string */
           const char *src,     /* I - Source string */
           size_t      size) {  /* I - Size of destination string buffer */
  size_t        srclen;         /* Length of source string */


 /*
  * Figure out how much room is needed...
  */

  size --;

  srclen = strlen(src);

 /*
  * Copy the appropriate amount...
  */

  if (srclen > size) srclen = size;

  memcpy(dst, src, srclen);
  dst[srclen] = '\0';

  return (srclen);
}


//extern size_t fl_strlcpy(char *, const char *, size_t);
#define strlcpy fl_strlcpy





#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  include <io.h>
#  define fl_mkdir(p)	mkdir(p)
#else
#  include <unistd.h>
#  define fl_mkdir(p)	mkdir(p, 0777)
#endif // WIN32 && !__CYGWIN__


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

  box(FL_DOWN_BOX);
  selection_color(FL_SELECTION_COLOR);

  items_       = 0;
  num_items_   = 0;
  alloc_items_ = 0;
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
  clear();

  if (items_)
    delete[] items_;
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
    
  int ts = thumbSize();
  
  for (int i = 0; i < num_items_; i ++)
  {
    int row = i / _numLines;
    
    int yoff = row * ts - scrollbar_.value();
    int xoff = i % _numLines ? ts : 0;
    
    if (yoff < -ts || yoff >= H)
      continue;

    ITEM *item = items_[i];

    if (!item)
      continue;

    Fl_Color bg;

    if (item->selected)
      bg = item->changed ?
               fl_color_average(FL_RED, selection_color(), 0.5) :
               selection_color();
    else
      bg = item->changed ? FL_RED : FL_WHITE; // TODO chosen background color?
    
    int ts = thumbSize();
    if (bg != FL_WHITE) // TODO chosen background color
    {
      fl_color(bg);
      fl_rectf(X + xoff + 2, Y + yoff + 2, ts - 4, ts+4); // H - 4);
    }

    if (item->thumbnail)
    {
        
        
        // TODO drawing double margin horizontally [once on right of image x, once on left of image x+1]
        // TODO margins setting
        // selected item is drawn smaller so selection color will show. otherwise, no margin.
        
        //int margin = 20; // TODO original margins
        //int drawsize = item->selected ? ts - margin : ts;
        //int delta    = item->selected ? 10 : 0;
        
        int drawsize = item->selected ?  ts - 10 : ts;
        int delta = item->selected ? 5 : 0;

        // TODO grid mode should be centered+cropped: draw anti-proportional then take center(drawsize,drawsize)
        // Proportional sizing
        int tW = drawsize;
        int tH = tW * item->thumbnail->h() / item->thumbnail->w();

        //if (item->thumbnail->h() > item->thumbnail->w())
        if (item->thumbnail->h() < item->thumbnail->w())
        {
            tH = drawsize;
            tW = tH * item->thumbnail->w() / item->thumbnail->h();
        }        
        
        auto tmpImage = item->thumbnail->copy(tW,tH);

        tmpImage->draw(X + xoff + delta, Y+yoff+delta,
                       drawsize, drawsize, (tW - drawsize) / 2, (tH - drawsize) / 2);
#if 0        
        // Proportional drawing
        tmpImage->draw(X + xoff + delta + (ts - tmpImage->w())/2, 
                       Y + yoff + delta + (ts - tmpImage->h())/2);
        //tmpImage->draw(X + xoff + (_thumbSize - item->thumbnail->w()) / 2, Y);
//                            Y + (H - textsize() - item->thumbnail->h()) / 2);
#endif
        tmpImage->release();
    }

#if 0 // KBR draw no label      
    fl_color(fl_contrast(FL_BLACK, bg));
    fl_font(textfont(), textsize());
    fl_draw(item->label, X + xoff + 2, Y, ITEMWIDTH - 4, H - 2,
            (Fl_Align)(FL_ALIGN_INSIDE | FL_ALIGN_BOTTOM |
	               FL_ALIGN_CLIP | FL_ALIGN_WRAP));
#endif	               
  }

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
  int	sel;				// Currently selected item
  int	X, Y;				// Mouse position


  X = Fl::event_x() - x(); // - Fl::box_dx(box());
  Y = Fl::event_y() - y();
  
  int ts = thumbSize();
  if (ts < 1) // KBR early event
      sel = -1;
  else
      sel = (Y + scrollbar_.value()) / ts;
    //sel = (X + scrollbar_.value()) / ts;

  if (sel < 0 || sel >= num_items_)
    sel = -1;

  switch (event)
  {
    case FL_PUSH :
      //  if (Y >= (h() - SBWIDTH))
	  //break;
      if (X >= w() - SBWIDTH)
          break;
      
      pushed_ = sel;

	if (sel >= 0)
	{
	  if (Fl::event_state() & FL_CTRL)
	    items_[sel]->selected = !items_[sel]->selected;
	  else if (Fl::event_state() & FL_SHIFT)
	  {
	    if (selected_ < 0)
	      selected_ = 0;

	    if (sel < selected_)
	    {
	      for (int i = sel; i <= selected_; i ++)
	        items_[i]->selected = 1;
	    }
	    else
	    {
	      for (int i = selected_; i <= sel; i ++)
	        items_[i]->selected = 1;
	    }
	  }
	  else if (!items_[sel]->selected)
	  {
	    // Select item...
	    for (int i = 0; i < num_items_; i ++)
	      items_[i]->selected = (i == sel);
	  }

  	  damage(FL_DAMAGE_SCROLL);
	}
	return (1);

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

    case FL_RELEASE :
	if (sel < 0)
	{
	  if (!(Fl::event_state() & (FL_CTRL | FL_SHIFT)))
	  {
	    for (int i = 0; i < num_items_; i ++)
	      items_[i]->selected = 0;
	  }
	}
	else if (!(Fl::event_state() & (FL_CTRL | FL_SHIFT)) && pushed_ == sel)
	{
	  // Select item...
	  for (int i = 0; i < num_items_; i ++)
	    items_[i]->selected = (i == sel);
	}

        selected_ = sel;

	damage(FL_DAMAGE_SCROLL);

        do_callback();
	clear_changed();
	return (1);

    case FL_SHORTCUT :
    case FL_KEYDOWN :
      if (Fl::event_key() == FL_Left && selected_ > 0)
        selected_ --;
      else if (Fl::event_key() == FL_Right && selected_ < (num_items_ - 1))
	    selected_ ++;
	  else
	    break;

      if (Fl::event_state() & FL_SHIFT)
	    items_[selected_]->selected = 1;
	  else
	  {
	  for (int i = 0; i < num_items_; i ++)
	    items_[i]->selected = (i == selected_);
	  }

      make_visible(selected_);

      do_callback();
	  return (1);

    case FL_FOCUS :
    case FL_UNFOCUS :
        return (1);
  }

  return (Fl_Group::handle(event));
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

  //scrollbar_.resize(X, Y + H - SBWIDTH, W, SBWIDTH);
  scrollbar_.resize(X + W - SBWIDTH, Y, SBWIDTH, H);

  update_scrollbar();

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
Fl_Image_BrowserV::set_scrollbar(int X)	// I - New scroll position
{

  int displayWidth = w() - Fl::box_dw(box());
  int currentSize = thumbSize();
  int numColumns = (int)((float)num_items_ / _numLines + 0.5);
  
  if (numColumns)
  {
    if ((numColumns * currentSize) <= displayWidth)
      X = 0;
    else if (X > (numColumns * currentSize - displayWidth))
      X = numColumns * currentSize - displayWidth;
    else if (X < 0)
      X = 0;

    scrollbar_.value(X, displayWidth, 0, numColumns * currentSize);
    scrollbar_.linesize(displayWidth / 2);
  }
  else
    scrollbar_.value(0, 1, 0, 1);

  if ((numColumns * currentSize) <= displayWidth)
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


  if (img || (!stat(filename, &fileinfo) && fileinfo.st_size))
  {
    // Only add non-empty or cached files!
    insert_item(filename, img, num_items_);
    update_scrollbar();
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
  while (num_items_ > 0)
    delete_item(0);

  update_scrollbar();
  clear_changed();
  damage(FL_DAMAGE_SCROLL);
}


//
// 'Fl_Image_BrowserV::delete_item()' - Delete an item from the browser.
//

void
Fl_Image_BrowserV::delete_item(int i)	// I - Item to delete
{

  if (outOfRange(i))
    return;

  ITEM *item = items_[i];

  if (item->filename)
    delete[] item->filename;

  if (item->thumbname)
    delete[] item->thumbname;

  if (item->image)
    item->image->release();

  if (item->thumbnail)
    item->thumbnail->release();

  if (item->comments)
    delete[] item->comments;

  delete item;

  num_items_ --;
  if (i < num_items_)
    memmove(items_ + i, items_ + i + 1, (num_items_ - i) * sizeof(ITEM *));
}


//
// 'Fl_Image_BrowserV::find()' - Find an item in the browser.
//

int					// O - Item number or -1 if none
Fl_Image_BrowserV::find(
    const char *filename)		// I - File to find
{

  for (int i = 0; i < num_items_; i ++)
    if (strcmp(filename, items_[i]->filename) == 0)
      return (i);

  return (-1);
}


//
// 'Fl_Image_BrowserV::insert_item()' - Insert an item in the browser.
//

Fl_Image_BrowserV::ITEM *		// O - New item
Fl_Image_BrowserV::insert_item(
    const char      *f,			// I - Filename
    Fl_Shared_Image *img,		// I - Image
    int             i)			// I - Index
{
  ITEM	*item,				// New item
	**temp;				// New item array
  char	thumbdir[512],			// Thumbnail directory
	thumbname[1024],		// Thumbnail filename
	*ptr;				// Pointer into filename


  // Verify that the file exists...
  if (access(f, 0))
    return (0);

  // Create a new item...
  item = new ITEM;

  item->filename = new char[strlen(f) + 1];
  strcpy(item->filename, f);

  // TODO 'label' should be renamed as 'tracker': the label used to lookup the cached thumbnail
  if ((item->label = strrchr(item->filename, '/')) != NULL)
    item->label ++;
  else
    item->label = item->filename;
  
  item->image     = img;
  item->thumbnail = 0;
  item->comments  = 0;
  item->changed   = 0;
  item->selected  = 0;

  // Load/create the thumbnail image...
  strlcpy(thumbdir, f, sizeof(thumbdir));

  if ((ptr = strrchr(thumbdir, '/')) != NULL)
    ptr ++;
  else
    ptr = thumbdir;

  strlcpy(ptr, ".xvpics", sizeof(thumbdir) - (ptr - thumbdir));

//#ifdef DEBUG
//  puts(thumbdir);
//#endif // DEBUG

  snprintf(thumbname, sizeof(thumbname), "%s/%s", thumbdir, item->label);

  item->thumbname = new char[strlen(thumbname) + 1];
  strcpy(item->thumbname, thumbname);

#ifdef DEBUG
  puts(thumbname);
#endif // DEBUG

  if (access(thumbname, 0) ||
      (item->thumbnail = Fl_Shared_Image::get(thumbname)) == NULL)
  {
    fl_mkdir(thumbdir);
    item->save_thumbnail();
  }

  // Add to the item array...
  if (i < 0)
    i = 0;
  else if (i > num_items_)
    i = num_items_;

  if (num_items_ >= alloc_items_)
  {
    temp = new ITEM *[alloc_items_ + 10];

    if (items_)
    {
      memcpy(temp, items_, num_items_ * sizeof(ITEM *));

      delete[] items_;
    }

    items_       = temp;
    alloc_items_ += 10;
  }

  if (i < num_items_)
    memmove(items_ + i + 1, items_ + i, (num_items_ - i) * sizeof(ITEM *));

  items_[i] = item;
  num_items_ ++;

  return (item);
}


//
// 'Fl_Image_BrowserV::move_item()' - Move an image in the browser.
//

void
Fl_Image_BrowserV::move_item(int from,	// I - From item
                            int to)	// I - To item
{

#ifdef DEBUG
  printf("Fl_Image_BrowserV::move_item(from=%d, to=%d)\n", from, to);
#endif // DEBUG

  if (from < 0 || from >= num_items_ || to < 0 || to > num_items_ ||
      from == to)
    return;

#ifdef DEBUG
  printf("    num_items_=%d\n", num_items_);
#endif // DEBUG

  ITEM *temp = items_[from];

  if (to < from)
    memmove(items_ + to + 1, items_ + to, (from - to) * sizeof(ITEM *));
  else
  {
    memmove(items_ + from, items_ + from + 1, (to - from) * sizeof(ITEM *));

    to --;
  }

  items_[to] = temp;
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

    int j;
    for (int i = 0; i < num_files; i ++)
    {
      snprintf(filename, sizeof(filename), "%s/%s", absdir, files[i]->d_name);

      for (j = 0; j < num_items_; j ++)
      
#if defined(WIN32) || defined(__EMX__) || defined(__APPLE__)
        if (!strcasecmp(items_[j]->filename, filename))
	  break;
#else
        if (!strcmp(items_[j]->filename, filename))  // KBR TODO possible to have conflicting filenames?
	  break;
#endif // WIN32 || __EMX__ || __APPLE__

      // Import all supported file formats *except* PPM to avoid cached
      // raw image files...
      if (!fl_filename_isdir(filename) && j >= num_items_ &&
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
// 'Fl_Image_BrowserV::load_item()' - Load the image for an item.
//

Fl_Shared_Image *			// O - Image
Fl_Image_BrowserV::load_item(int i)	// I - Index
{

  if (outOfRange(i))
    return nullptr;

  ITEM *item = items_[i];

  if (!item->image)
    item->image = Fl_Shared_Image::get(item->filename);

  return item->image;
}


//
// 'Fl_Image_BrowserV::remove()' - Remove an item.
//

void
Fl_Image_BrowserV::remove(int i)		// I - Index to remove
{
  delete_item(i);
  update_scrollbar();
  redraw();
}


//
// 'Fl_Image_BrowserV::ITEM::make_thumbnail()' - Make the thumbnail image.
//

// TODO what if image is smaller than THUMBSIZE?
// KBR create a decent sized thumbnail in the first place
//#define THUMBSIZE (ITEMWIDTH-20)
#define THUMBSIZE 500

void
Fl_Image_BrowserV::ITEM::make_thumbnail()
{

  // Clear the thumbnail image as needed...
  if (thumbnail)
  {
    thumbnail->release();
    thumbnail = nullptr;
  }

  // Create the thumbnail image...
  bool releaseit = false; // need to release the image?
  if (!image)
  {
    releaseit = true;
    image     = Fl_Shared_Image::get(filename);
  }

  if (image && image->w() && image->h())
  {
    // Size the thumbnail within an 80x80 box...
    int W = THUMBSIZE;
    int H = W * image->h() / image->w();

    if (image->h() > image->w()) //(H > THUMBSIZE)
    {
      H = THUMBSIZE;
      W = H * image->w() / image->h();
    }

    thumbnail = (Fl_Shared_Image *)image->copy(W, H);
    
  }

  if (image && releaseit)
  {
    image->release();
    image = 0;
  }
}


//
// 'Fl_Image_BrowserV::ITEM::save_thumbnail()' - Save the thumbnail image.
//

void
Fl_Image_BrowserV::ITEM::save_thumbnail(
    int createit)			// I - 1 = create thumbnail image
{
  FILE		*thumbfile;		// Thumbnail file


  // Create the thumbnail image as needed...
  if (createit || !thumbnail)
    make_thumbnail();

  if (!thumbnail)
    return;

  int W = thumbnail->w();
  int H = thumbnail->h();

  // Save the thumbnail image...
  if ((thumbfile = fopen(thumbname, "wb")) != NULL)
  {
    fprintf(thumbfile, "P7 332\n%d %d 255\n", W, H);

    // ptr to image data
    uchar *rgb = (uchar *)thumbnail->data()[0];
    for (int Y = 0; Y < H; Y ++)
      for (int X = 0; X < W; X ++)
      {
        int r = *rgb++ >> 5;
        int g = *rgb++ >> 5;
        int b = *rgb++ >> 6;

        putc((((r << 3) | g) << 2) | b, thumbfile);
      }

    fclose(thumbfile);
  }
}


//
// 'Fl_Image_BrowserV::select()' - Select an image.
//

void
Fl_Image_BrowserV::select(int i)		// I - Index
{

  if (outOfRange(i))
    return;

  selected_ = i;

  for (int j = 0; j < num_items_; j ++)
    items_[j]->selected = 0;

  items_[i]->selected = 1;

  make_visible(i);
}


//
// 'Fl_Image_BrowserV::make_visible()' - Make an image visible.
//

void
Fl_Image_BrowserV::make_visible(int i)	// I - Index
{

  if (outOfRange(i))
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

