#include <FL/Fl.h>
#include <FL/fl_ask.h>
#include <sptk5/cnet>
#include <sptk5/cgui>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace sptk;

// Buffer size for read buffer
// The smaller it is, more responsible interface will be.
// Though, you may loose in speed lil bit.
//#define BUFFER_SIZE 16384
//#define BUFFER_SIZE 4096
#define BUFFER_SIZE 1024

CHttpConnect   *sock;
CInput         *urlInput;
CMemoInput     *paramsInput;
CComboBox      *paramsCombo;
CEditor        *textdisp;

void go_callback(Fl_Widget *,void *) 
{
   textdisp->textBuffer()->text("");    
   textdisp->redraw();

   std::string  pageName = urlInput->data();
   string hostName = pageName;

   try {
      //sock->host(hostName);
      sock->host("192.168.1.14");
      sock->port(8080);
      sock->open();

	  CBuffer response;
	  sock->write("CONNECT "+hostName+":443 HTTP/1.0\r\n\r\n\r\n");
	  do {
			sock->readLine(response);
	  }
	  while (response.bytes() > 2);

      CStrings text(paramsInput->data(),"\n");
      CHttpParams httpFields;
      if (paramsCombo->data() == "HTTP Get") {

         for (unsigned i = 0; i < text.size(); i++) {
            CStrings data(text[i],"=");
            if (data.size() == 2) {
               httpFields["first_name"] = text[0];
               httpFields["last_name"] = text[1];
            }
         }

         sock->cmd_get(pageName,httpFields);
      } else {
         for (unsigned i = 0; i < text.size(); i++) {
            CStrings data(text[i],"=");
            if (data.size() == 2) {
               httpFields["first_name"] = text[0];
               httpFields["last_name"] = text[1];
            }
         }
      }
      sock->close();
      textdisp->textBuffer()->text(sock->htmlData().data());
   }
   catch(CException &e) {
      spError(e.what());
      return;
   }
}

int main(int argc,char *argv[])
{
    // Initialize themes
    CThemes themes;

    CHttpConnect socket;
    sock = &socket;

    CWindow main_window(600,400,"CHttpConnect test");

    CToolBar urlToolBar;
    urlInput = new CInput("http://",300,SP_ALIGN_LEFT);
    urlInput->labelWidth(40);
    urlInput->data("www.tts-sf.com/index.html");
    CButton go_button(SP_EXEC_BUTTON,SP_ALIGN_LEFT,"Go");
    go_button.callback(go_callback);
    urlToolBar.end();

    CToolBar paramsToolBar;
    paramsToolBar.layoutSize(150);
    paramsCombo = new CComboBox("Mode",10,SP_ALIGN_TOP);
    paramsCombo->labelWidth(40);
    paramsCombo->addRows("http mode",CStrings("HTTP Get|HTTP Post","|"));
    paramsCombo->columns()[0].width(100);
    paramsCombo->data("HTTP Post");
    paramsInput = new CMemoInput("",100,SP_ALIGN_CLIENT);
    paramsToolBar.end();

    CEditor  editor(10,SP_ALIGN_CLIENT);

    textdisp = &editor;

    CGroup status_line("",25,SP_ALIGN_BOTTOM);
    status_line.layoutSpacing(1);

    status_line.end();

    main_window.end();
    main_window.resizable(main_window);
    main_window.show(argc,argv);

    return Fl::run();
}
