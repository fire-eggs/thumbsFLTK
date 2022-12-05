#include <FL/Fl_Native_File_Chooser.H>

#include "Fl_Image_Browser.H"

char *fl_native_file_chooser(const char *message,const char *pat,const char *fname,int relative=0) {

    static char   retname[FL_PATH_MAX];           // Returned filename
    char *tmpstr = NULL;

    Fl_Native_File_Chooser fnfc; // = new Fl_Native_File_Chooser;
    fnfc.title(message);
    fnfc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fnfc.filter(pat);

    if (!fname || !*fname)
        fname = ".";

    fnfc.preset_file( fname ); // !fname || !*fname ? "." : fname);

    // Show native chooser
    switch ( fnfc.show() ) {
        case -1:
            break;  // ERROR

        case  1: 
            break;  // CANCEL

        default: 
            tmpstr = (char *)fnfc.filename();
            break;  // FILE CHOSEN
    }

    if (tmpstr && relative) {
        fl_filename_relative(retname, sizeof(retname), (char *)fnfc.filename());
        return retname;
    }

    return tmpstr;
}

#if 0
void dobtn(Fl_Widget *, void *)
{
    char *res = fl_native_file_chooser("Massage","",nullptr);
}
#endif

Fl_Image_BrowserV *browser_;

void cb_browser_(Fl_Image_BrowserV* o, void* v) 
{
  //((flphoto*)(o->parent()->user_data()))->cb_browser__i(o,v);
  //puts("Browser callback\n");
}

void cb_numlines(Fl_Widget *o, void *d)
{
    Fl_Choice *w = dynamic_cast<Fl_Choice *>(o);
    int newval = w->value() + 1;
    browser_->numLines(newval);
}

void cb_stack(Fl_Widget *o, void *d)
{
    Fl_Check_Button *w = dynamic_cast<Fl_Check_Button*>(o);
    browser_->setStackMode(w->value() == 1);
}

int main(int argc, char** argv) {

    fl_register_images();
    
    Fl_Double_Window window(50, 50, 500, 750);
    
    Fl_Choice *choice = new Fl_Choice(100, 10, 100, 25, "Num Lines:");
    choice->add("1");
    choice->add("2");
    choice->add("3");
    choice->add("4");
    choice->add("5");
    choice->value(1); // make "2" the default
    choice->callback(cb_numlines);
    
    Fl_Check_Button *stack = new Fl_Check_Button(210, 10, 100, 25, "Stack");
    stack->callback(cb_stack);
    
    browser_ = new Fl_Image_BrowserV(10, 40, 400, 600);
    browser_->box(FL_DOWN_BOX);
    browser_->color(FL_LIGHT3);
    browser_->selection_color(FL_SELECTION_COLOR);
    browser_->labeltype(FL_NORMAL_LABEL);
    browser_->labelfont(1);
    browser_->labelsize(14);
    browser_->labelcolor(FL_FOREGROUND_COLOR);
    browser_->callback((Fl_Callback*)cb_browser_);
    browser_->align(FL_ALIGN_TOP_LEFT);
    browser_->when(FL_WHEN_RELEASE);
    browser_->numLines(2);
    
    browser_->end();

	// TODO use chooser to open folder
    browser_->load("/mnt/brix1/temp/testImages");
    
    window.resizable(browser_);
        
    window.show();
    return Fl::run();
}
