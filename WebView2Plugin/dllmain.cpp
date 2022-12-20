#include <map>
#include "MyWebview.h"

#define PLUGIN_API extern "C" __declspec(dllexport)
//-------------------------------------------------------------------


//Create a callback delegate
void RegisterDebugCallback(FuncCallBack cb) {
    dbgCallbackInstance = cb;
}

static std::map<LPCWSTR, MyWebview*> webviews;
static std::map<LPCWSTR, MyWebview*>::iterator webviewsIt;

MyWebview* getWebView(LPCWSTR objectName) {
    if (webviews.size() > 0)
    {
        webviewsIt = webviews.find(objectName);
        if (webviewsIt != webviews.end())
        {
            return webviewsIt->second;
        }
    }
    return NULL;
}

//-------------------------------------------------------------------
void Log(LPCWSTR message, Color color) {
    if (dbgCallbackInstance != nullptr)
        dbgCallbackInstance(message, (int)color, (int)wcslen(message));
}
//-------------------------------------------------------------------

/// <summary>
/// Register Navigation Completed Callback
/// </summary>
/// <param name="cb">function called back</param>
void RegisterNavigationCompletedCallback(LPCWSTR objectName, EventCallBack cb)
{
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->registerNavCallback(cb);
    }
}
//-------------------------------------------------------------------

PLUGIN_API void createWebView(LPCWSTR objectName, LPCWSTR url, LPCWSTR browserPath = NULL, LPCWSTR dataPath = NULL, LPCWSTR windowName = NULL) {
    if (getWebView(objectName) == NULL)
    {
        MyWebview* webview = new MyWebview(objectName, url, browserPath, dataPath, windowName);
        webviews[objectName] = webview;
    }
}

PLUGIN_API void navigate(LPCWSTR objectName, LPCWSTR url) {
    Log(L"navigate");

    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->navigate(url);
    }
}

PLUGIN_API void navigateToHTML(LPCWSTR objectName, LPCWSTR htmlContent) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->navigateToHTML(htmlContent);
    }
}

PLUGIN_API void updateWebViewBound(LPCWSTR objectName, int x, int y, int width, int height) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {

        RECT bounds;
        bounds.left = x;
        bounds.top = y;
        bounds.right = x + width;
        bounds.bottom = y + height;
        webView->setBounds(bounds);
    }
}

PLUGIN_API void closeWebView(LPCWSTR objectName) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->closeWebView();
    }
}

PLUGIN_API bool isActive(LPCWSTR objectName) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        return webView->isActive();
    }
    return false;
}

PLUGIN_API bool isVisible(LPCWSTR objectName) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        return webView->isVisible();
    }
    return false;
}