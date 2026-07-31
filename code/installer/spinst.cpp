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

#include "CompletedPage.h"
#include "ConfirmationPage.h"
#include "DirectoryPage.h"
#include "InstallerConfig.h"
#include "OptionsPage.h"
#include "ProgressPage.h"
#include "WelcomePage.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

#include <sptk5/OsProcess.h>
#include <sptk5/cgui>
#include <sptk5/cutils>
#include <sptk5/gui/CPngImage.h>
#include <sptk5/net/HttpConnect.h>
#include <sptk5/net/SSLSocket.h>
#include <sptk5/net/URL.h>

#include <filesystem>
#include <memory>
#include <thread>

using namespace std;
using namespace sptk;

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
    void createPages();

    void updateButtons();
    void showPage(uint32_t page);
    void goNext();
    void goBack();
    void doCancel();
    void startInstallation();
    void installThread(stop_token token);

    void postLog(const String& text);
    void postProgress(float value);
    void postDone(bool success);

    String detectPackageType() const;
    String getDownloadUrl() const;

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

    // Wizard pages, indexed by the Page enum
    vector<unique_ptr<WizardPage>> m_pages;

    // Pages the wizard talks to directly, owned by m_pages
    ProgressPage*  m_progressPage {nullptr};
    CompletedPage* m_completedPage {nullptr};

    bool    m_installing {false};
    bool    m_installDone {false};
    bool    m_installSuccess {false};
    bool    m_cancelRequested {false};
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
            m_progressPage->addLogLine(msg->text);
            break;

        case GuiMessage::PROGRESS:
            m_progressPage->progress(msg->progress);
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

    createPages();

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

void InstallerWizard::createPages()
{
    // The pages are added in the order of the Page enum
    m_pages.push_back(make_unique<WelcomePage>(m_config));
    m_pages.push_back(make_unique<DirectoryPage>(m_config));
    m_pages.push_back(make_unique<OptionsPage>(m_config));
    m_pages.push_back(make_unique<ConfirmationPage>(m_config));

    auto progressPage = make_unique<ProgressPage>(m_config);
    m_progressPage = progressPage.get();
    m_pages.push_back(move(progressPage));

    auto completedPage = make_unique<CompletedPage>(m_config);
    m_completedPage = completedPage.get();
    m_pages.push_back(move(completedPage));

    if (m_pages.size() != PAGE_COUNT)
        throw Exception("The wizard page list doesn't match the Page enum");

    for (const auto& page: m_pages)
        page->create(*m_tabs);
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
    else
        m_nextButton->label("Next");

    if (m_installing)
        m_nextButton->deactivate();
    else if (page == PAGE_PROGRESS && !m_installDone)
        m_nextButton->deactivate();
    else
        m_nextButton->activate();

    // Cancel button. It stays enabled while the installation runs, so a slow
    // download or a stuck package manager can still be interrupted
    if (page == PAGE_COMPLETED || m_cancelRequested)
        m_cancelButton->deactivate();
    else
        m_cancelButton->activate();
}

void InstallerWizard::showPage(uint32_t page)
{
    m_pages[page]->onEnter();
    m_tabs->pageNumber(page);
    updateButtons();
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
        showPage(PAGE_PROGRESS);
        startInstallation();
        return;
    }

    if (page == PAGE_PROGRESS)
    {
        if (!m_installDone)
            return;

        m_completedPage->showResult(m_installSuccess);
        showPage(PAGE_COMPLETED);
        return;
    }

    // The page data must be valid before the wizard moves on
    if (!m_pages[page]->onLeave())
        return;

    if (page < PAGE_COUNT - 1)
        showPage(page + 1);
}

void InstallerWizard::goBack()
{
    uint32_t page = m_tabs->pageNumber();
    if (page > 0)
        showPage(page - 1);
}

void InstallerWizard::doCancel()
{
    if (m_installing)
    {
        if (fl_choice("The installation is in progress.\n"
                      "Stop it after the current step completes?",
                      "No", "Yes", nullptr)
            != 1)
            return;

        m_cancelRequested = true;
        m_installWorker.request_stop();
        m_progressPage->addLogLine("Cancelling, waiting for the current step to finish...");
        updateButtons();
        return;
    }

    if (fl_choice("Are you sure you want to cancel the installation?", "No", "Yes", nullptr) == 1)
        m_window->hide();
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
    m_cancelRequested = false;
    updateButtons();

    m_installWorker = jthread([this](stop_token token)
                              {
                                  installThread(token);
                              });
}

void InstallerWizard::installThread(stop_token token)
{
    // Cancellation is honoured between the installation steps. Killing a running
    // package manager could leave a partially installed package behind
    const auto postCancelled = [this]
    {
        postLog("Installation cancelled.");
        postProgress(100);
        postDone(false);
    };

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
        String        installCmd;
        const String& installDir = m_config.installDirectory;

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

        // FLTK threading support must be initialised before any window is created.
        // Without it Fl::awake() from the installation thread is silently dropped,
        // and the progress page never receives a log, progress or completion message.
        Fl::lock();

        InstallerWizard wizard(config, argc, argv);
        int             exitCode = wizard.run();

        Fl::unlock();
        return exitCode;
    }
    catch (const Exception& e)
    {
        CERR(e.what());
        return 1;
    }
}
