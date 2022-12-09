#ifndef Fl_Image_BrowserV_H
#  define Fl_Image_BrowserV_H

//
// Image browser widget header file for the Fast Light Tool Kit (FLTK).
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


#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Shared_Image.H>

#include "ItemList.h"

class FL_EXPORT Fl_Image_BrowserV : public Fl_Group
{
private:
  const static int SBWIDTH = 17;

  int		pushed_;
  int		selected_;
  uchar		textfont_;
  uchar		textsize_;
  Fl_Scrollbar	scrollbar_;

  int _numLines; // rows/columns of images
  bool _stackMode; // grid or stack
  int _maxExtent; // furthest end of the thumbnails
  
  static void	scrollbar_cb(Fl_Widget *w, void *d);
  void		set_scrollbar(int X);
  void		update_scrollbar();

  ItemList *_itemList;

  int thumbSize() 
  { 
      // TODO horizontal version
    //int H = h() - Fl::box_dh(box()) - SBWIDTH;
    //return H / _numLines; 
    
    // TODO vertical version
    int W = w() - Fl::box_dw(box()) - SBWIDTH;
    return W / _numLines;
  } 
  
protected:

  void draw();
  void drawGrid(int, int, int, int);
  void drawStack(int, int, int, int);
  void recalc();
  
public:

  Fl_Image_BrowserV(int X, int Y, int W, int H, const char *L = 0);
  ~Fl_Image_BrowserV();

  void		add(const char *filename, Fl_Shared_Image *img = 0);
  void		clear();
  int		find(const char *filename);
  int		handle(int event);
  void		load(const char *dirname);
  void		make_visible(int i);
  void		move(int from, int to) { _itemList->move_item(from, to); make_visible(to); }
  void		remove(int i);
  void		resize(int X, int Y, int W, int H);
  void		select(int i);
  int		selected() const { return selected_; }
  uchar		textfont() const { return textfont_; }
  void		textfont(uchar f) { textfont_ = f; }
  uchar		textsize() const { return textsize_; }
  void		textsize(uchar f) { textsize_ = f; }
  
  ItemList::ITEM *value(int i) { return _itemList->get(i); }
  
  void numLines(int val);
  void setStackMode(bool val);
  
};


#endif // !Fl_Image_BrowserV_H
