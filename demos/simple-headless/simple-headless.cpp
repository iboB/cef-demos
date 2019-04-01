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
#include <include/wrapper/cef_resource_manager.h>

#include <thread> // so we can sleep in the main loop
#include <atomic>

const char* HTML = R"demohtml(
<!DOCTYPE html>
<html>
<script>
    console.log('Hello from the html!');
    let req = 'done';
    window.cefQuery({
        request: req,
    });
</script>
</html>
)demohtml";

const char* URL = "https://cef-demos/headless";

std::atomic_bool HTML_LoadingDone;

class MessageHandler : public CefMessageRouterBrowserSide::Handler
{
public:
    MessageHandler() = default;

    bool OnQuery(CefRefPtr<CefBrowser> browser,
        CefRefPtr<CefFrame> frame,
        int64 query_id,
        const CefString& request,
        bool persistent,
        CefRefPtr<Callback> callback) override
    {
        if (request == "done") {
            HTML_LoadingDone = true;
            return true;
        }
        return false;
    }
};

void SetupResourceManagerOnIOThread(CefRefPtr<CefResourceManager> resourceManager)
{
    if (!CefCurrentlyOn(TID_IO))
    {
        CefPostTask(TID_IO, base::Bind(SetupResourceManagerOnIOThread, resourceManager));
        return;
    }

    resourceManager->AddContentProvider(URL, HTML, "text/html", 10, std::string());
}

class HeadlessClient : public CefClient, public CefLifeSpanHandler, public CefRequestHandler, public CefRenderHandler
{
public:
    HeadlessClient()
        : m_resourceManager(new CefResourceManager)
    {
        SetupResourceManagerOnIOThread(m_resourceManager);
    }

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
    virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override { return this; }

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
        m_messageHandler.reset(new MessageHandler);
        m_messageRouter->AddHandler(m_messageHandler.get(), false);
    }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
        m_messageRouter->RemoveHandler(m_messageHandler.get());
        m_messageHandler.reset();
        m_messageRouter = nullptr;
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

    /////////////////////////////////////
    // render handler
    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override
    {
        rect = CefRect(0, 0, 200, 200); // some random rect
    }

    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override
    {} // noop. nothing to do


private:
    CefRefPtr<CefResourceManager> m_resourceManager;

    CefRefPtr<CefMessageRouterBrowserSide> m_messageRouter;
    scoped_ptr<CefMessageRouterBrowserSide::Handler> m_messageHandler;

    IMPLEMENT_REFCOUNTING(HeadlessClient);
    DISALLOW_COPY_AND_ASSIGN(HeadlessClient);
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
        puts("WEBKIG!!!");
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
    std::string appType = commandLine->GetSwitchValue("type");
    if (appType == "renderer" || appType == "zygote") {
        app = new RendererApp;
    }
    int result = CefExecuteProcess(args, app, windowsSandboxInfo);
    if (result >= 0)
    {
        // child process completed
        return result;
    }

    CefSettings settings;
    settings.windowless_rendering_enabled = 1;
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    CefInitialize(args, settings, nullptr, windowsSandboxInfo);

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(0);

    CefBrowserSettings browserSettings;
    auto browser = CefBrowserHost::CreateBrowserSync(windowInfo, new HeadlessClient, URL, browserSettings, nullptr);

    // main loop
    while (!HTML_LoadingDone)
    {
        CefDoMessageLoopWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    browser = nullptr;
    CefShutdown();

    return 0;
}