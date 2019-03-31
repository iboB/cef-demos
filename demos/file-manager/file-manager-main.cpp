// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/wrapper/cef_message_router.h>

#include <common/PlatformUtil.hpp>
#include <common/Resources.hpp>

#include "FileManager.hpp"

#include <memory>
#include <sstream>

#define URI_ROOT "https://cefdemos"
const char* URL = URI_ROOT "/file-manager.html";

class MessageHandler : public CefMessageRouterBrowserSide::Handler
{
public:
    MessageHandler(fileman::FileManager& f)
        : m_fileManager(f)
    {}

    bool OnQuery(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int64 query_id,
        const CefString& request,
        bool persistent,
        CefRefPtr<Callback> callback) override
    {
        std::string r = request;
        auto f = r.find(':');
        if (f == std::string::npos) return false;
        auto rtype = r.substr(0, f);
        if (rtype != "contents") return false;
        auto rarg = r.substr(f+1);
        auto contents = m_fileManager.getDirectoryContents(rarg.c_str());

        std::ostringstream json;
        json << R"json({"dirs":[)json";
        for(size_t i=0; i<contents.dirs.size(); ++i) {
            if (i != 0) json << ',';
            json << '"' << contents.dirs[i] << '"';
        }
        json << R"json(],"files":[)json";
        for (size_t i = 0; i < contents.files.size(); ++i) {
            if (i != 0) json << ',';
            json << '"' << contents.files[i] << '"';
        }
        json << R"json(]})json";

        //puts(json.str().c_str());

        callback->Success(json.str());
        return true;
    }

private:
    fileman::FileManager& m_fileManager;
};

class FileManagerClient : public CefClient, public CefLifeSpanHandler, public CefRequestHandler
{
public:
    FileManagerClient()
        : m_resourceManager(new CefResourceManager)
        , m_fileManager(new fileman::FileManager)
    {
        auto exePath = cefdemos::PlatformUtil::getCurrentExecutablePath();
        auto assetPath = cefdemos::PlatformUtil::getAssetPath(exePath, "html");
        cefdemos::Resources::setupResourceManagerDirectoryProvider(m_resourceManager, URI_ROOT, assetPath);
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process,
        CefRefPtr<CefProcessMessage> message) override
    {
        return m_messageRouter->OnProcessMessageReceived(browser, source_process, message);
    }

    /////////////////////////////////////
    // lifespan handler
    virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
    {
        CefMessageRouterConfig mrconfig;
        m_messageRouter = CefMessageRouterBrowserSide::Create(mrconfig);
        m_messageHandler.reset(new MessageHandler(*m_fileManager));
        m_messageRouter->AddHandler(m_messageHandler.get(), false);
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
        m_messageRouter->RemoveHandler(m_messageHandler.get());
        m_messageHandler.reset();
        m_messageRouter = nullptr;

        CefQuitMessageLoop();
    }


    /////////////////////////////////////
    // request handler

    virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        bool user_gesture,
        bool is_redirect) override
    {
        m_messageRouter->OnBeforeBrowse(browser, frame);
        return false;
    }

    virtual void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) override
    {
        m_messageRouter->OnRenderProcessTerminated(browser);
    }

    virtual cef_return_value_t OnBeforeResourceLoad(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request,
        CefRefPtr<CefRequestCallback> callback) override
    {
        return m_resourceManager->OnBeforeResourceLoad(browser, frame, request, callback);
    }

    virtual CefRefPtr<CefResourceHandler> GetResourceHandler(
        CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request) override
    {
        return m_resourceManager->GetResourceHandler(browser, frame, request);
    }

private:
    std::unique_ptr<fileman::FileManager> m_fileManager;

    CefRefPtr<CefResourceManager> m_resourceManager;

    CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
    scoped_ptr<CefMessageRouterBrowserSide::Handler> m_messageHandler;

    IMPLEMENT_REFCOUNTING(FileManagerClient);
    DISALLOW_COPY_AND_ASSIGN(FileManagerClient);
};

////////////////////////////////////////
// renderer process handling
class RendererApp : public CefApp, public CefRenderProcessHandler
{
public:
    RendererApp() = default;

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
        return this;
    }

    void OnWebKitInitialized() override
    {
        CefMessageRouterConfig config;
        m_messageRouter = CefMessageRouterRendererSide::Create(config);
    }

    virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override
    {
        m_messageRouter->OnContextCreated(browser, frame, context);
    }

    virtual void OnContextReleased(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override
    {
        m_messageRouter->OnContextReleased(browser, frame, context);
    }

    virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override
    {
        return m_messageRouter->OnProcessMessageReceived(browser, source_process, message);
    }

private:
    CefRefPtr<CefMessageRouterRendererSide> m_messageRouter;

    IMPLEMENT_REFCOUNTING(RendererApp);
    DISALLOW_COPY_AND_ASSIGN(RendererApp);
};
////////////////////////////////////////

int main(int argc, char* argv[])
{
    CefRefPtr<CefCommandLine> commandLine = CefCommandLine::CreateCommandLine();
#if defined(_WIN32)
    CefEnableHighDPISupport();
    CefMainArgs args(GetModuleHandle(NULL));
    commandLine->InitFromString(GetCommandLineW());
#else
    CefMainArgs args(argc, argv);
    commandLine->InitFromArgv(argc, argv);
#endif

    void* windowsSandboxInfo = NULL;

#if defined(CEF_USE_SANDBOX) && defined(_WIN32)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scopedSandbox;
    windowsSandboxInfo = scopedSandbox.sandbox_info();
#endif

    CefRefPtr<CefApp> app = nullptr;
    if (commandLine->GetSwitchValue("type") == "renderer") {
        app = new RendererApp;
    }
    int result = CefExecuteProcess(args, app, windowsSandboxInfo);
    if (result >= 0)
    {
        // child process completed
        return result;
    }

    CefSettings settings;
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    CefInitialize(args, settings, nullptr, windowsSandboxInfo);

    CefWindowInfo windowInfo;

#if defined(_WIN32)
    // On Windows we need to specify certain flags that will be passed to CreateWindowEx().
    windowInfo.SetAsPopup(NULL, "Resource");
#endif
    CefBrowserSettings browserSettings;
    CefBrowserHost::CreateBrowser(windowInfo, new FileManagerClient, URL, browserSettings, nullptr);

    CefRunMessageLoop();

    CefShutdown();

    return 0;
}