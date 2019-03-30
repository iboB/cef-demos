// CEF-Demos
// Copyright (c) 2019 Borislav Stanimirov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include <iostream>

#include <common/PlatformUtil.hpp>
#include <common/Resources.hpp>

#include <include/cef_app.h>
#include <include/cef_client.h>

#define URI_ROOT "https://cefdemos"
const char* URL = URI_ROOT "/hello-demos.html";

class ResourceClient : public CefClient, public CefLifeSpanHandler, public CefRequestHandler
{
public:
    ResourceClient()
        : m_resourceManager(new CefResourceManager)
    {
        auto exePath = cefdemos::PlatformUtil::getCurrentExecutablePath();
        auto assetPath = cefdemos::PlatformUtil::getAssetPath(exePath, "html");
        cefdemos::Resources::setupResourceManagerDirectoryProvider(m_resourceManager, URI_ROOT, assetPath);
    }

    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override
    {
        CefQuitMessageLoop();
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
    CefRefPtr<CefResourceManager> m_resourceManager;

    IMPLEMENT_REFCOUNTING(ResourceClient);
    DISALLOW_COPY_AND_ASSIGN(ResourceClient);
};

int main()
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

    int result = CefExecuteProcess(args, nullptr, windowsSandboxInfo);
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
    CefBrowserHost::CreateBrowser(windowInfo, new ResourceClient, URL, browserSettings, nullptr);

    CefRunMessageLoop();

    CefShutdown();

    return 0;
}
