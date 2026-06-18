/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       spinst.cpp - application installer                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

#include <sptk5/OsProcess.h>
#include <sptk5/cgui>
#include <sptk5/cutils>
#include <sptk5/gui/CCheckButtons.h>
#include <sptk5/gui/CListView.h>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/gui/CProgressBar.h>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/net/SSLSocket.h>
#include <sptk5/net/URL.h>
#include <sptk5/xdoc/Document.h>

#include <filesystem>
#include <thread>

using namespace std;
using namespace sptk;

// ── Configuration ───────────────────────────────────────────────────────────

struct InstallOption
{
    String name;
    String value;
};

struct InstallerConfig
{

    String                application {"Application"};
    String                version {"1.0.0"};
    String                description;
    String                installDirectory {"/opt/app"};
    String                sidebarImage;
    vector<InstallOption> options;
    map<String, String>   packages;

    void load(const filesystem::path& configFile)
    {
        Buffer buf;
        buf.loadFromFile(configFile);

        xdoc::Document doc;
        doc.load(buf);
        const auto& root = doc.root();

        application = root->getString("application");
        version = root->getString("version");
        description = root->getString("description");
        installDirectory = root->getString("install_directory");
        sidebarImage = root->getString("sidebar_image");

        for (const auto& optNode: root->nodes("options"))
        {
            InstallOption opt;
            opt.name = optNode->getString("name");
            opt.value = optNode->getString("value");
            options.push_back(opt);
        }

        auto pkgNode = root->findFirst("packages");
        if (pkgNode)
        {
            for (const auto& child: pkgNode->nodes())
                packages[string(child->getName())] = child->getString();
        }
    }
};

// ── Page indices ────────────────────────────────────────────────────────────

enum Page : uint32_t
{
    PAGE_WELCOME = 0,
    PAGE_DIRECTORY = 1,
    PAGE_OPTIONS = 2,
    PAGE_CONFIRMATION = 3,
    PAGE_PROGRESS = 4,
    PAGE_COMPLETED = 5,
    PAGE_COUNT = 6
};

// ── Message from install thread to GUI ──────────────────────────────────────

struct GuiMessage
{
    enum Type
    {
        LOG,
        PROGRESS,
        DONE
    };
    Type   type;
    String text;
    float  progress {0};
};

// Forward declaration
class InstallerWizard;

struct AwakeData
{
    InstallerWizard* wizard;
    GuiMessage*      message;
};

// ── InstallerWizard ─────────────────────────────────────────────────────────

class InstallerWizard
{
public:
    InstallerWizard(InstallerConfig& config, int argc, char* argv[]);
    int run();

    // Called from awake callback
    void handleMessage(GuiMessage* msg);

private:
    void createWelcomePage();
    void createDirectoryPage();
    void createOptionsPage();
    void createConfirmationPage();
    void createProgressPage();
    void createCompletedPage();

    void updateButtons();
    void goNext();
    void goBack();
    void doCancel();
    void startInstallation();
    void installThread();

    void postLog(const String& text);
    void postProgress(float value);
    void postDone(bool success);

    String detectPackageType() const;
    String getDownloadUrl() const;
    String buildConfirmationHtml() const;

    static void cb_next(Fl_Widget*, void* data);
    static void cb_back(Fl_Widget*, void* data);
    static void cb_cancel(Fl_Widget*, void* data);
    static void cb_awake(void* data);

    InstallerConfig& m_config;
    CWindow*         m_window {nullptr};
    CTabs*           m_tabs {nullptr};
    CButton*         m_backButton {nullptr};
    CButton*         m_nextButton {nullptr};
    CButton*         m_cancelButton {nullptr};

    CHtmlBox*      m_welcomeHtml {nullptr};
    CInput*        m_dirInput {nullptr};
    CCheckButtons* m_checkButtons {nullptr};
    CHtmlBox*      m_confirmHtml {nullptr};
    CProgressBar*  m_progressBar {nullptr};
    CListView*     m_logView {nullptr};
    CHtmlBox*      m_completedHtml {nullptr};

    bool    m_installing {false};
    bool    m_installDone {false};
    bool    m_installSuccess {false};
    jthread m_installWorker;
};

// ── Static callbacks ────────────────────────────────────────────────────────

void InstallerWizard::cb_next(Fl_Widget*, void* data)
{
    static_cast<InstallerWizard*>(data)->goNext();
}

void InstallerWizard::cb_back(Fl_Widget*, void* data)
{
    static_cast<InstallerWizard*>(data)->goBack();
}

void InstallerWizard::cb_cancel(Fl_Widget*, void* data)
{
    static_cast<InstallerWizard*>(data)->doCancel();
}

void InstallerWizard::cb_awake(void* data)
{
    auto* ad = static_cast<AwakeData*>(data);
    ad->wizard->handleMessage(ad->message);
    delete ad->message;
    delete ad;
}

void InstallerWizard::handleMessage(GuiMessage* msg)
{
    switch (msg->type)
    {
        case GuiMessage::LOG:
            m_logView->addRow(0, Strings {msg->text});
            m_logView->redraw();
            break;

        case GuiMessage::PROGRESS:
            m_progressBar->data(Variant(static_cast<double>(msg->progress)));
            m_progressBar->redraw();
            break;

        case GuiMessage::DONE:
            m_installing = false;
            m_installDone = true;
            m_installSuccess = (msg->text == "success");
            updateButtons();
            break;
    }
}

// ── Constructor ─────────────────────────────────────────────────────────────

InstallerWizard::InstallerWizard(InstallerConfig& config, int argc, char* argv[])
    : m_config(config)
{
    CThemes allThemes;

    String title = m_config.application + " " + m_config.version + " Setup";
    m_window = new CWindow(640, 480, title.c_str());

    int X, Y, W, H;
    Fl::screen_xywh(X, Y, W, H);
    m_window->position(X + (W - 640) / 2, Y + (H - 480) / 2);

    // Sidebar image (LEFT-aligned group with Fl_Box inside)
    if (!m_config.sidebarImage.empty() && filesystem::exists(m_config.sidebarImage.c_str()))
    {
        auto* sidebarGroup = new CGroup("", 164, CLayoutAlign::LEFT);
        auto* imgBox = new Fl_Box(0, 0, 164, 400);
        auto* img = new CPngImage(m_config.sidebarImage);
        imgBox->image(img->copy(164, 400));
        imgBox->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP);
        sidebarGroup->end();
    }

    // Wizard pages (tabs with hidden tab bar)
    m_tabs = new CTabs("", 10, CLayoutAlign::CLIENT);
    m_tabs->showTabs(false);

    createWelcomePage();
    createDirectoryPage();
    createOptionsPage();
    createConfirmationPage();
    createProgressPage();
    createCompletedPage();

    m_tabs->end();

    // Navigation buttons
    auto* buttonGroup = new CGroup("", 10, CLayoutAlign::BOTTOM);

    m_cancelButton = new CButton(CButtonKind::CANCEL_BUTTON, CLayoutAlign::RIGHT);
    m_cancelButton->callback(cb_cancel, this);

    m_nextButton = new CButton(CButtonKind::NEXT_BUTTON, CLayoutAlign::RIGHT);
    m_nextButton->callback(cb_next, this);

    m_backButton = new CButton(CButtonKind::PRIOR_BUTTON, CLayoutAlign::RIGHT);
    m_backButton->callback(cb_back, this);

    buttonGroup->end();

    m_window->end();
    m_window->resizable(m_window);

    updateButtons();
    m_window->show(argc, argv);
}

// ── Page creation ───────────────────────────────────────────────────────────

void InstallerWizard::createWelcomePage()
{
    Fl_Group* page = m_tabs->newScroll("Welcome", false);

    m_welcomeHtml = new CHtmlBox("", 10, CLayoutAlign::CLIENT);
    String html = "<h2>Welcome to " + m_config.application + " Setup</h2>"
                                                             "<p>" +
                  m_config.description + "</p>"
                                         "<p>This wizard will guide you through the installation of <b>" +
                  m_config.application + " " + m_config.version + "</b>.</p>"
                                                                  "<p>Click Next to continue.</p>";
    m_welcomeHtml->data(html);

    page->end();
}

void InstallerWizard::createDirectoryPage()
{
    Fl_Group* page = m_tabs->newScroll("Directory", false);

    auto* label = new CHtmlBox("", 50, CLayoutAlign::TOP);
    label->data(Variant("<h3>Installation Directory</h3>"
                        "<p>Choose the directory where " +
                        m_config.application + " will be installed.</p>"));

    m_dirInput = new CInput("Install to:", 30, CLayoutAlign::TOP);
    m_dirInput->data(m_config.installDirectory);

    page->end();
}

void InstallerWizard::createOptionsPage()
{
    Fl_Group* page = m_tabs->newScroll("Options", false);

    auto* label = new CHtmlBox("", 50, CLayoutAlign::TOP);
    label->data("<h3>Installation Options</h3>"
                "<p>Select the components you want to install.</p>");

    m_checkButtons = new CCheckButtons("Options:", 20, CLayoutAlign::CLIENT);
    Strings buttonLabels;
    for (const auto& opt: m_config.options)
        buttonLabels.push_back(opt.name);
    m_checkButtons->buttons(buttonLabels);

    // Select all by default
    String allSelected;
    for (size_t i = 0; i < m_config.options.size(); i++)
    {
        if (i > 0)
            allSelected += "|";
        allSelected += m_config.options[i].name;
    }
    m_checkButtons->data(Variant(allSelected));

    page->end();
}

void InstallerWizard::createConfirmationPage()
{
    Fl_Group* page = m_tabs->newScroll("Confirm", false);

    m_confirmHtml = new CHtmlBox("", 10, CLayoutAlign::CLIENT);
    m_confirmHtml->data("<h3>Ready to Install</h3>"
                        "<p>Click <b>Install</b> to begin the installation.</p>");

    page->end();
}

void InstallerWizard::createProgressPage()
{
    Fl_Group* page = m_tabs->newPage("Progress", false);

    auto* label = new CHtmlBox("", 30, CLayoutAlign::TOP);
    label->data("<h3>Installing...</h3>");

    m_progressBar = new CProgressBar("Progress:", 25, CLayoutAlign::TOP);
    m_progressBar->minimum(0);
    m_progressBar->maximum(100);
    m_progressBar->data(Variant(0.0f));

    m_logView = new CListView("Installation Log:", 10, CLayoutAlign::CLIENT);
    m_logView->addColumn(CColumn("Message", VariantDataType::VAR_STRING, 500));
    m_logView->showGrid(true);

    page->end();
}

void InstallerWizard::createCompletedPage()
{
    Fl_Group* page = m_tabs->newScroll("Completed", false);

    m_completedHtml = new CHtmlBox("", 10, CLayoutAlign::CLIENT);
    m_completedHtml->data("<h3>Installation Complete</h3>"
                          "<p>Click <b>Finish</b> to exit the wizard.</p>");

    page->end();
}

// ── Navigation ──────────────────────────────────────────────────────────────

void InstallerWizard::updateButtons()
{
    uint32_t page = m_tabs->pageNumber();

    // Back button
    if (page == PAGE_WELCOME || page == PAGE_PROGRESS || page == PAGE_COMPLETED)
        m_backButton->deactivate();
    else
        m_backButton->activate();

    // Next button label and state
    if (page == PAGE_CONFIRMATION)
        m_nextButton->label("Install");
    else if (page == PAGE_COMPLETED)
        m_nextButton->label("Finish");
    else if (page == PAGE_PROGRESS)
        m_nextButton->label("Next");
    else
        m_nextButton->label("Next");

    if (m_installing)
        m_nextButton->deactivate();
    else if (page == PAGE_PROGRESS && !m_installDone)
        m_nextButton->deactivate();
    else
        m_nextButton->activate();

    // Cancel button
    if (page == PAGE_COMPLETED || m_installing)
        m_cancelButton->deactivate();
    else
        m_cancelButton->activate();
}

void InstallerWizard::goNext()
{
    uint32_t page = m_tabs->pageNumber();

    if (page == PAGE_COMPLETED)
    {
        m_window->hide();
        return;
    }

    if (page == PAGE_CONFIRMATION)
    {
        m_tabs->pageNumber(PAGE_PROGRESS);
        updateButtons();
        startInstallation();
        return;
    }

    if (page == PAGE_PROGRESS && m_installDone)
    {
        if (m_installSuccess)
            m_completedHtml->data(Variant("<h3>Installation Complete</h3>"
                                          "<p><b>" +
                                          m_config.application + " " + m_config.version + "</b> has been successfully installed.</p>"
                                                                                          "<p>Click <b>Finish</b> to exit the wizard.</p>"));
        else
            m_completedHtml->data(Variant("<h3>Installation Failed</h3>"
                                          "<p>There were errors during installation. "
                                          "Please check the installation log for details.</p>"
                                          "<p>Click <b>Finish</b> to exit the wizard.</p>"));

        m_tabs->pageNumber(PAGE_COMPLETED);
        updateButtons();
        return;
    }

    // Update confirmation summary when entering confirmation page
    if (page + 1 == PAGE_CONFIRMATION)
        m_confirmHtml->data(buildConfirmationHtml());

    if (page < PAGE_COUNT - 1)
    {
        m_tabs->pageNumber(page + 1);
        updateButtons();
    }
}

void InstallerWizard::goBack()
{
    uint32_t page = m_tabs->pageNumber();
    if (page > 0)
    {
        m_tabs->pageNumber(page - 1);
        updateButtons();
    }
}

void InstallerWizard::doCancel()
{
    if (fl_choice("Are you sure you want to cancel the installation?", "No", "Yes", nullptr) == 1)
        m_window->hide();
}

String InstallerWizard::buildConfirmationHtml() const
{
    String html = "<h3>Ready to Install</h3>";
    html += "<p><b>Application:</b> " + m_config.application + " " + m_config.version + "</p>";
    html += "<p><b>Install to:</b> " + String(m_dirInput->data().getString()) + "</p>";

    String selectedOpts = m_checkButtons->data().getString();
    if (!selectedOpts.empty())
    {
        html += "<p><b>Options:</b></p><ul>";
        Strings opts(selectedOpts, "|");
        for (const auto& opt: opts)
            html += "<li>" + String(opt) + "</li>";
        html += "</ul>";
    }

    html += "<p>Click <b>Install</b> to begin.</p>";
    return html;
}

// ── Thread-safe message posting ─────────────────────────────────────────────

void InstallerWizard::postLog(const String& text)
{
    Fl::awake(cb_awake, new AwakeData {this, new GuiMessage {GuiMessage::LOG, text, 0}});
}

void InstallerWizard::postProgress(float value)
{
    Fl::awake(cb_awake, new AwakeData {this, new GuiMessage {GuiMessage::PROGRESS, "", value}});
}

void InstallerWizard::postDone(bool success)
{
    Fl::awake(cb_awake, new AwakeData {this, new GuiMessage {GuiMessage::DONE, success ? "success" : "failure", 0}});
}

// ── Installation ────────────────────────────────────────────────────────────

String InstallerWizard::detectPackageType() const
{
    if (filesystem::exists("/usr/bin/apt") || filesystem::exists("/usr/bin/apt-get"))
        return "linux_deb";
    if (filesystem::exists("/usr/bin/rpm") || filesystem::exists("/usr/bin/dnf") || filesystem::exists("/usr/bin/yum"))
        return "linux_rpm";
    return "linux_tar";
}

String InstallerWizard::getDownloadUrl() const
{
    String pkgType = detectPackageType();
    auto   it = m_config.packages.find(pkgType);
    if (it != m_config.packages.end())
        return it->second;

    it = m_config.packages.find("linux_tar");
    if (it != m_config.packages.end())
        return it->second;

    return {};
}

void InstallerWizard::startInstallation()
{
    m_installing = true;
    m_installDone = false;
    updateButtons();

    m_installWorker = jthread([this](stop_token)
                              {
                                  installThread();
                              });
}

void InstallerWizard::installThread()
{
    postLog("Starting installation...");
    postProgress(5);

    // Determine package type and URL
    String downloadUrl = getDownloadUrl();
    if (downloadUrl.empty())
    {
        postLog("ERROR: No download URL configured for this OS.");
        postDone(false);
        return;
    }

    String pkgType = detectPackageType();
    postLog("Detected package type: " + pkgType);
    postLog("Download URL: " + downloadUrl);
    postProgress(10);

    // Download the file
    postLog("Downloading package...");

    try
    {
        URL url(downloadUrl);
        auto [hostname, port] = url.hostAndPort();
        String urlPath = url.path();

        shared_ptr<TCPSocket> socket;
        if (url.protocol() == "https")
        {
            auto sslSocket = make_shared<SSLSocket>();
            sslSocket->host(Host(hostname, port == 0 ? 443 : port));
            socket = sslSocket;
        }
        else
        {
            socket = make_shared<TCPSocket>();
            socket->host(Host(hostname, port == 0 ? 80 : port));
        }

        socket->open();
        HttpConnect http(socket);
        Buffer      fileData;
        int         statusCode = http.cmd_get(urlPath, HttpParams(), fileData);

        if (statusCode != 200)
        {
            postLog("ERROR: Download failed with HTTP " + to_string(statusCode));
            postDone(false);
            return;
        }

        postLog("Downloaded " + to_string(fileData.bytes()) + " bytes.");
        postProgress(50);

        // Save to temp file
        String extension;
        if (pkgType == "linux_deb")
            extension = ".deb";
        else if (pkgType == "linux_rpm")
            extension = ".rpm";
        else
            extension = ".tar.gz";

        filesystem::path tmpFile = filesystem::temp_directory_path() / ("spinst_package" + extension);
        fileData.saveToFile(tmpFile);
        postLog("Saved to " + tmpFile.string());
        postProgress(60);

        // Install using package manager
        String installCmd;
        String installDir = m_dirInput->data().getString();

        if (pkgType == "linux_deb")
            installCmd = "sudo apt install -y " + tmpFile.string();
        else if (pkgType == "linux_rpm")
            installCmd = "sudo rpm -i " + tmpFile.string();
        else
        {
            filesystem::create_directories(installDir.c_str());
            installCmd = "tar xzf " + tmpFile.string() + " -C " + installDir;
        }

        postLog("Running: " + installCmd);
        postProgress(70);

        OsProcess process(installCmd, [this](const string& output)
                          {
                              if (!output.empty())
                                  postLog(String(output));
                          });

        process.start();
        int exitCode = process.wait();

        postProgress(95);

        // Clean up temp file
        filesystem::remove(tmpFile);

        if (exitCode == 0)
        {
            postLog("Installation completed successfully.");
            postProgress(100);
            postDone(true);
        }
        else
        {
            postLog("ERROR: Installation failed with exit code " + to_string(exitCode));
            postProgress(100);
            postDone(false);
        }
    }
    catch (const exception& e)
    {
        postLog(String("ERROR: ") + e.what());
        postDone(false);
    }
}

int InstallerWizard::run()
{
    return Fl::run();
}

// ── Main ────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 2)
        {
            cerr << "Usage: spinst <config.json>" << endl;
            return 1;
        }

        InstallerConfig config;
        config.load(argv[1]);

        InstallerWizard wizard(config, argc, argv);
        return wizard.run();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
        return 1;
    }
}
