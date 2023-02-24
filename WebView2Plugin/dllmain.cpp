#include <map>
#include "MyWebview.h"

#define PLUGIN_API extern "C" __declspec(dllexport)
//-------------------------------------------------------------------


//Create a callback delegate
void RegisterDebugCallback(FuncCallBack cb) {
    dbgCallbackInstance = cb;
}

static std::vector<MyWebview*> webviews;

MyWebview* getWebView(LPCWSTR objectName) {
    if (webviews.size() > 0)
    {
        for (std::vector<MyWebview*>::iterator it = webviews.begin(); it != webviews.end(); ++it)
        {
            if ((*it)->isBoundToObjectWithName(objectName))
                return (*it);
        }
    }
    return NULL;
}

//-------------------------------------------------------------------
void Log(LPCWSTR message, Color color) {
    if (dbgCallbackInstance != nullptr)
        dbgCallbackInstance(message, (int)color, (int)wcslen(message));
}
void Log(LPCWSTR message, Color color, ...) {
    va_list vl; 
    va_start(vl, message);
    if (dbgCallbackInstance != nullptr)
    {
        wchar_t szBuff[1024];
        swprintf_s(szBuff, 1024, message, vl);
        dbgCallbackInstance(szBuff, (int)color, (int)wcslen(szBuff));
    }
    va_end(vl);
}


std::wstring fromChar(std::string s) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(s);
}

std::string fromWideChar(std::wstring ws) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(ws);
}
//-------------------------------------------------------------------

/// <summary>
/// Register Navigation Completed Callback
/// </summary>
/// <param name="cb">function called back</param>
void RegisterNavigationCompletedCallback(LPCWSTR objectName, EventCallBack navDoneCb, EventCallBack responseReceiveCb)
{
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->registerNavCallback(navDoneCb);
        webView->registerResponseReceivedCallback(responseReceiveCb);
    }
}
//-------------------------------------------------------------------

PLUGIN_API int newWebView(LPCWSTR objectName, EventCallBack navCb = nullptr, EventCallBack responseReceivedCb = nullptr) {
    if (getWebView(objectName) == NULL)
    {
        LPCWSTR webviewName = objectName;
        webviews.push_back(new MyWebview(webviewName, navCb, responseReceivedCb));
        //webviews.insert(std::pair<LPCWSTR, MyWebview*>(objectName, webview));

        return 0;
    }
    else {
        Log(L"webview already created");
    }
    return -1;
}

PLUGIN_API int createWebView(LPCWSTR objectName, LPCWSTR url, bool startVisible = true, LPCWSTR browserPath = nullptr, LPCWSTR dataPath = nullptr, LPCWSTR windowName = nullptr) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        return webView->create(url, startVisible, browserPath, dataPath, windowName);
    }
    else
    {
        int nbWebview = (int)webviews.size();
        wchar_t szbuf[256];
        swprintf_s(szbuf, 256, L"Webview not available over %d : ", nbWebview);
        Log(szbuf);
    }
    return -1;
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
    if (webviews.size() > 0)
    {
        webviews.erase(
            std::remove_if(
                webviews.begin(),
                webviews.end(),
                [objectName](MyWebview* const& w) { if (w->isBoundToObjectWithName(objectName)) {
                        w->closeWebView();
                        return true;
                    }
                    return false;
                }
            ),
            webviews.end()
        );
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

PLUGIN_API void setVisibility(LPCWSTR objectName, bool visibility) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->setVisibility(visibility);
    }
}

PLUGIN_API void RunJavaScript(LPCWSTR objectName, LPCWSTR script, JSCallBack callback)
{
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->runJavascript(script, callback);
    }
}

PLUGIN_API void setAuthUrl(LPCWSTR objectName, LPCWSTR url)
{
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->setAuthUrl(url);
    }
}

PLUGIN_API void getCookies(LPCWSTR objectName, LPCWSTR url, EventCallBack callback) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->getCookies(url, callback);
    }
}

PLUGIN_API bool saveCookies(LPCWSTR objectName, LPCWSTR url) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        return webView->saveCookies(url);
    }
    return false;
}

PLUGIN_API void clearCache(LPCWSTR objectName) {
    MyWebview* webView = getWebView(objectName);
    if (webView != NULL) {
        webView->clearCache();
    }
}