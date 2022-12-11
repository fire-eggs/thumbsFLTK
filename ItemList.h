#ifndef _ITEMLIST_H_
#define _ITEMLIST_H_

#include <FL/Fl_Shared_Image.H>

class ItemList
{
public:
    
  struct ITEM
  {
    char            *filename;
    char            *thumbname;
    char            *label;
    char            *comments;
    Fl_Shared_Image *image;
    Fl_Shared_Image *thumbnail;
    int             changed;
    int             selected;

    void make_thumbnail();
    void save_thumbnail(int createit = 0);
    
    int _x;
    int _y;
    int _w;
    int _h;
  };

private:  
  ITEM **items_;
  int    num_items_;
  int    alloc_items_;
  
public:
    ItemList();
    ~ItemList();
    
  void  delete_item(int i);
  ITEM *insert_item(const char *f, Fl_Shared_Image *img, int i = __INT_MAX__);
  void  move_item(int from, int to);

  bool outOfRange(int val) { return val < 0 || val >= num_items_; }

  int find(int x, int y);
  int find(const char *filename);
  Fl_Shared_Image *load_item(int i);

  int		count() const { return num_items_; }

  int		selected(int i) { return outOfRange(i) ? 0 : items_[i]->selected; }
  
  ITEM *get(int i) { return outOfRange(i) ? nullptr : items_[i]; }
  ITEM *getUnsafe(int i) { return items_[i]; }
  
  void clear();
  
  void clearSelect();
  void select(int);
  void toggleSelect(int);
  void selectRange(int, int);
  void forceSelect(int);
  bool isSelected(int);
};

#endif // _ITEMLIST_H_
