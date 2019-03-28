// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <thread>

class NoopRenderHandler : public CefRenderHandler
{
public:
    NoopRenderHandler() = default;

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override
    {
        rect = CefRect(0, 0, 200, 200);
    }

    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height) override
    {}

    IMPLEMENT_REFCOUNTING(NoopRenderHandler);
    DISALLOW_COPY_AND_ASSIGN(NoopRenderHandler);
};

class HeadlessClient : public CefClient
{
public:
    HeadlessClient() = default;

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
    {
        if (!m_renderHandler) m_renderHandler = new NoopRenderHandler;
        return m_renderHandler;
    }

    CefRefPtr<CefRenderHandler> m_renderHandler;

    IMPLEMENT_REFCOUNTING(HeadlessClient);
    DISALLOW_COPY_AND_ASSIGN(HeadlessClient);
};


int main(int argc, char* argv[])
{
#if defined(_WIN32)
    CefEnableHighDPISupport();
    CefMainArgs args(GetModuleHandle(NULL));
#else
    CefMainArgs args(argc, argv);
#endif

    void* windowsSandboxInfo = NULL;

#if defined(CEF_USE_SANDBOX) && defined(_WIN32)
    // Manage the life span of the sandbox information object. This is necessary
    // for sandbox support on Windows. See cef_sandbox_win.h for complete details.
    CefScopedSandboxInfo scopedSandbox;
    windowsSandboxInfo = scopedSandbox.sandbox_info();
#endif

    CefSettings settings;
    settings.windowless_rendering_enabled = 1;
#if !defined(CEF_USE_SANDBOX)
    settings.no_sandbox = true;
#endif

    CefInitialize(args, settings, nullptr, windowsSandboxInfo);

    CefWindowInfo windowInfo;
    windowInfo.SetAsWindowless(0);

    CefBrowserSettings browserSettings;
    auto browser = CefBrowserHost::CreateBrowserSync(windowInfo, new HeadlessClient,
        "https://ibob.github.io/cef-demos/html/hello-demos.html", browserSettings, nullptr);

    // just do some message loop work for 2.25 seconds
    for (int i=0; i<150; ++i) {
        CefDoMessageLoopWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }

    browser = nullptr;
    CefShutdown();

    return 0;
}