/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <fstream>
#include <gtest/gtest.h>
#include <sptk5/net/BaseMailConnect.h>

using namespace std;
using namespace sptk;

namespace sptk {

// Minimal concrete subclass for testing the abstract BaseMailConnect.
class TestMailConnect : public BaseMailConnect
{
protected:
    void sendMessage() override {}
};

// Helper: call mimeMessage() and return the result as std::string.
static string buildMime(TestMailConnect& mail)
{
    Buffer buf;
    mail.mimeMessage(buf);
    return {buf.c_str(), buf.bytes()};
}

// ── Accessor tests ───────────────────────────────────────────────────────────

TEST(BaseMailConnectTests, accessors)
{
    TestMailConnect mail;

    mail.from("John Doe <john@example.com>");
    mail.to("Jane Doe <jane@example.com>");
    mail.cc("Bob <bob@example.com>");
    mail.bcc("Alice <alice@example.com>");
    mail.subject("Test subject");
    mail.body("Hello, World!", false);
    mail.attachments("file1.txt;file2.pdf");

    EXPECT_EQ(mail.from(), "John Doe <john@example.com>");
    EXPECT_EQ(mail.to(), "Jane Doe <jane@example.com>");
    EXPECT_EQ(mail.cc(), "Bob <bob@example.com>");
    EXPECT_EQ(mail.bcc(), "Alice <alice@example.com>");
    EXPECT_EQ(mail.subject(), "Test subject");
    EXPECT_EQ(mail.body(), "Hello, World!");
    EXPECT_EQ(mail.attachments(), "file1.txt;file2.pdf");
}

// ── mimeMessage() structural tests ──────────────────────────────────────────

TEST(BaseMailConnectTests, mimeMessage_requiredHeaders)
{
    TestMailConnect mail;
    mail.from("John Doe <john@example.com>");
    mail.to("Jane Doe <jane@example.com>");
    mail.subject("Hello");
    mail.body("Test body", false);

    const string mime = buildMime(mail);

    EXPECT_NE(mime.find("From: John Doe <john@example.com>"), string::npos);
    EXPECT_NE(mime.find("To: Jane Doe <jane@example.com>"), string::npos);
    EXPECT_NE(mime.find("Subject: Hello"), string::npos);
    EXPECT_NE(mime.find("Date: "), string::npos);
    EXPECT_NE(mime.find("MIME-Version: 1.0"), string::npos);
    EXPECT_NE(mime.find("Content-Type: multipart/mixed"), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_plainTextBody)
{
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("recipient@example.com");
    mail.subject("Plain text test");
    mail.body("This is the message body.", false);

    const string mime = buildMime(mail);

    EXPECT_NE(mime.find("Content-Type: text/plain"), string::npos);
    EXPECT_NE(mime.find("This is the message body."), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_htmlBody)
{
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("recipient@example.com");
    mail.subject("HTML test");
    mail.body("<html><body><b>Bold</b></body></html>", false);

    const string mime = buildMime(mail);

    EXPECT_NE(mime.find("Content-Type: multipart/alternative"), string::npos);
    EXPECT_NE(mime.find("Content-Type: text/plain"), string::npos);
    EXPECT_NE(mime.find("Content-Type: text/html"), string::npos);
    // HTML content must appear in the message
    EXPECT_NE(mime.find("<html>"), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_ccAppearsInHeader)
{
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("recipient@example.com");
    mail.cc("cc@example.com");
    mail.subject("CC test");
    mail.body("Body", false);

    const string mime = buildMime(mail);

    EXPECT_NE(mime.find("CC: cc@example.com"), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_bccNotInMimeOutput)
{
    // BCC recipients must never appear in the MIME message headers.
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("recipient@example.com");
    mail.bcc("secret@example.com");
    mail.subject("BCC test");
    mail.body("Body", false);

    const string mime = buildMime(mail);

    EXPECT_EQ(mime.find("BCC:"), string::npos);
    EXPECT_EQ(mime.find("secret@example.com"), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_defaultFromWhenEmpty)
{
    TestMailConnect mail;
    // from() is intentionally not set
    mail.to("recipient@example.com");
    mail.subject("Default from test");
    mail.body("Body", false);

    const string mime = buildMime(mail);

    EXPECT_NE(mime.find("From: postmaster"), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_semicolonRecipientsConvertedToCommas)
{
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("a@example.com;b@example.com;c@example.com");
    mail.cc("x@example.com;y@example.com");
    mail.subject("Multi-recipient test");
    mail.body("Body", false);

    const string mime = buildMime(mail);

    EXPECT_NE(mime.find("To: a@example.com, b@example.com, c@example.com"), string::npos);
    EXPECT_NE(mime.find("CC: x@example.com, y@example.com"), string::npos);
}

TEST(BaseMailConnectTests, mimeMessage_doesNotMutateToAndCc)
{
    // mimeMessage() must not permanently modify the to() and cc() fields.
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("a@example.com;b@example.com");
    mail.cc("x@example.com;y@example.com");
    mail.subject("Mutation test");
    mail.body("Body", false);

    buildMime(mail); // first call — this triggers the semicolon replacement

    EXPECT_EQ(mail.to(), "a@example.com;b@example.com");
    EXPECT_EQ(mail.cc(), "x@example.com;y@example.com");
}

TEST(BaseMailConnectTests, mimeMessage_calledTwiceProducesSameOutput)
{
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("a@example.com;b@example.com");
    mail.subject("Idempotent test");
    mail.body("Body", false);

    const string first  = buildMime(mail);
    const string second = buildMime(mail);

    // The To: header must be the same in both calls.
    const auto pos1 = first.find("To: ");
    const auto pos2 = second.find("To: ");
    ASSERT_NE(pos1, string::npos);
    ASSERT_NE(pos2, string::npos);
    const auto end1 = first.find('\n', pos1);
    const auto end2 = second.find('\n', pos2);
    EXPECT_EQ(first.substr(pos1, end1 - pos1), second.substr(pos2, end2 - pos2));
}

TEST(BaseMailConnectTests, mimeMessage_withFileAttachment)
{
    // Write a small temp file and attach it.
    const filesystem::path tmpFile = filesystem::temp_directory_path() / "sptk_test_attachment.txt";
    {
        ofstream ofs(tmpFile);
        ASSERT_TRUE(ofs.is_open()) << "Could not create temp file: " << tmpFile;
        ofs << "attachment content";
    }

    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("recipient@example.com");
    mail.subject("Attachment test");
    mail.body("See attached file.", false);
    mail.attachments(tmpFile.string());

    string mime;
    EXPECT_NO_THROW(mime = buildMime(mail));

    EXPECT_NE(mime.find("Content-Disposition: attachment"), string::npos);
    EXPECT_NE(mime.find("Content-Transfer-Encoding: base64"), string::npos);
    EXPECT_NE(mime.find("sptk_test_attachment.txt"), string::npos);

    filesystem::remove(tmpFile);
}

TEST(BaseMailConnectTests, mimeMessage_missingAttachmentFileThrows)
{
    TestMailConnect mail;
    mail.from("sender@example.com");
    mail.to("recipient@example.com");
    mail.subject("Missing attachment test");
    mail.body("Body", false);
    mail.attachments("/nonexistent/path/no_such_file.txt");

    Buffer buf;
    EXPECT_THROW(mail.mimeMessage(buf), Exception);
}

} // namespace sptk
