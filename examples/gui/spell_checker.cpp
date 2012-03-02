#include <stdio.h>

#include <FL/fl_ask.h>
#include <sptk5/cgui>
#include <sptk5/gui/CEditorSpellChecker.h>
#include <sptk5/CException.h>

using namespace std;
using namespace sptk;

CEditor  *editor;

void cb_spellCheck(Fl_Widget *,void *) {
   CEditorSpellChecker sc(editor);
   try {
      sc.spellCheck();
   }
   catch (exception& e) {
      fl_alert("%s",e.what());
   }
}

int main(int argc,char *argv[]) {

   CWindow  window(400,300,"CSpellChecker test");
   editor = new CEditor(10,SP_ALIGN_CLIENT);

   editor->textBuffer()->text("Mary has a little lemb, big botl of whiskie, and cucomber");

   CToolBar toolBar;
   CButton  spellCheckButton("Spell Check",SP_ALIGN_LEFT);
   spellCheckButton.callback(cb_spellCheck);

   window.show();

   CThemes::set("OSX");

   window.relayout();

   Fl::run();

   return 0;
}
