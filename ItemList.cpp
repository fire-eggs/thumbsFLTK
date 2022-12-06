#include <algorithm> // min, max
#include "ItemList.h"


#if defined(WIN32) && !defined(__CYGWIN__)
#  include <direct.h>
#  include <io.h>
#  define fl_mkdir(p)	mkdir(p)
#else
#  include <unistd.h> // access
#  define fl_mkdir(p)	mkdir(p, 0777)
#endif // WIN32 && !__CYGWIN__


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



ItemList::ItemList()
{
  items_       = nullptr;
  num_items_   = 0;
  alloc_items_ = 0;
}

ItemList::~ItemList()
{
  if (items_) // TODO unnecessary check?
    delete[] items_;
}

void ItemList::clear()
{
  while (num_items_ > 0)
    delete_item(0);
}


//
// 'Fl_Image_BrowserV::delete_item()' - Delete an item from the browser.
//

void ItemList::delete_item(int i)	// I - Item to delete
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
ItemList::find(
    const char *filename)		// I - File to find
{

    for (int j=0; j < num_items_; j++)
#if defined(WIN32) || defined(__EMX__) || defined(__APPLE__)
        if (!strcasecmp(items_[j]->filename, filename))
            return j;
#else
        if (!strcmp(items_[j]->filename, filename))
            return j;
#endif // WIN32 || __EMX__ || __APPLE__

    return -1;
}

//
// 'Fl_Image_BrowserV::insert_item()' - Insert an item in the browser.
//

ItemList::ITEM *		// O - New item
ItemList::insert_item(
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
// 'Fl_Image_BrowserV::load_item()' - Load the image for an item.
//

Fl_Shared_Image *			// O - Image
ItemList::load_item(int i)	// I - Index
{

  if (outOfRange(i))
    return nullptr;

  ITEM *item = items_[i];

  if (!item->image)
    item->image = Fl_Shared_Image::get(item->filename);

  return item->image;
}

//
// 'Fl_Image_BrowserV::move_item()' - Move an image in the browser.
//

void
ItemList::move_item(int from,	// I - From item
                            int to)	// I - To item
{

#ifdef DEBUG
  printf("ItemList::move_item(from=%d, to=%d)\n", from, to);
#endif // DEBUG

  if (outOfRange(from) || outOfRange(to) || from == to)
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

void ItemList::clearSelect()
{
    for (int i = 0; i < num_items_; i ++)
        items_[i]->selected = 0;
}

void ItemList::forceSelect(int i)
{
    items_[i]->selected = 1;
}

void ItemList::select(int i)
{
  if (outOfRange(i))
    return;

  // select only the specified, clear all others
  for (int j = 0; j < num_items_; j ++)
    items_[j]->selected = j == i;
}

void ItemList::selectRange(int from, int to)
{
    for (int i = std::min(from, to); i <= std::max(from, to); i++)
        items_[i]->selected = 1;
}

void ItemList::toggleSelect(int sel)
{
    if (outOfRange(sel))
        return;
    items_[sel]->selected = !items_[sel]->selected;
}

bool ItemList::isSelected(int idx)
{
    return outOfRange(idx) ? false : items_[idx]->selected;
}


//
// 'Fl_Image_BrowserV::ITEM::make_thumbnail()' - Make the thumbnail image.
//

// TODO what if image is smaller than THUMBSIZE?
// KBR create a decent sized thumbnail in the first place
//#define THUMBSIZE (ITEMWIDTH-20)
#define THUMBSIZE 500

void
ItemList::ITEM::make_thumbnail()
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
ItemList::ITEM::save_thumbnail(
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

int ItemList::find(int x, int y)
{
    for (int i=0; i < count(); i++)
    {
        ITEM *tem = items_[i];
        if (x < tem->_x || y < tem->_y)
            continue;
        if (x < (tem->_x + tem->_w) &&
            y < (tem->_y + tem->_h))
            return i;
    }
    return -1;
}
