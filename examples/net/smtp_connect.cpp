/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          smtp_connect.cpp  -  description
                             -------------------
    begin                : 26 June 2003
    copyright            : (C) 2000-2012 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <stdio.h>
#include <sptk5/CSmtpConnect.h>
#include <sptk5/CStrings.h>

using namespace std;
using namespace sptk;

void printResonse(const CStrings& response) {
   for (unsigned i = 0; i < response.size(); i++) {
      puts(response[i].c_str());
   }
   puts("---------------------------------");
}

int main( int argc, char *argv[] ) {
   char           buffer[128];
   CSmtpConnect   SMTP;
   std::string    user, password, email;
   
   puts("Testing SMTP connectivity.\n");
   printf("SMTP server name: ");
   scanf("%s", buffer);
   SMTP.host(buffer);
   
   printf("SMTP user name (or N/A if not required): ");
   scanf("%s", buffer);
   user = buffer;
   
   if (trim(lowerCase(user)) != "n/a") {
      printf("SMTP user password: ");
      scanf("%s", buffer);
      password = buffer;
   } else {
      user = "";
   }
   
   printf("E-mail address to test: ");
   scanf("%s", buffer);
   email = buffer;
   
   puts("\nTrying to connect to SMTP server..");
   
   try {
      SMTP.cmd_login(user, password);
      printResonse(SMTP.response());
      
      SMTP.subject("Test e-mail");
      SMTP.from("Yourself <"+email+">");
      SMTP.to(email);
      SMTP.body("<HTML><BODY>Hello, <b>my friend!</b><br><br>\n\nIf you received this e-mail it means the SMTP module works just fine.<br><br>\n\nSincerely, SPTK.<br>\n</BODY></HTML>", true);
      //SMTP.attachments("test.html");
      
      puts("\nSending test message..");
      SMTP.cmd_send();
      printResonse(SMTP.response());
      
      puts("\nClosing SMTP connection..");
      SMTP.cmd_quit();
      printResonse(SMTP.response());
      
      puts(("\nMessage send. Please, check your mail in "+email).c_str());
   }
   catch (std::exception& e) {
      puts(e.what());
   }
   return 0;
}
