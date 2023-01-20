#pragma once
#include <Windows.h>
#include <WebView2.h>
#include <vector>
#include <string>
#include <tchar.h>
#include <wil/com.h>

#include "common.h"

class MyWebview {
    /// <summary>
    /// linked gameobject name
    /// </summary>
    std::wstring objectName;

    /// <summary>
    /// path where data are saved
    /// </summary>
    std::wstring userDataPath;

    /// <summary>
    /// path where cookies are saved
    /// </summary>
    std::wstring cookiesDataPath;

    /// <summary>
    /// url for auth detection, retrieve response content and parse result
    /// </summary>
    std::wstring authUrl;

    /// <summary>
    /// Display rect of webview
    /// </summary>
    RECT bounds;

    /* Managers and environments */
    wil::com_ptr<ICoreWebView2Controller> webviewController;
    wil::com_ptr<ICoreWebView2> webviewWindow;
    wil::com_ptr<ICoreWebView2CookieManager> cookieManager;

    /* Events */
    EventCallBack navigationCompletedCallbackInstance = nullptr;
    EventRegistrationToken navigationCompletedCallbackToken;
    EventCallBack receiveResponseCallbackInstance = nullptr;
    EventRegistrationToken webResponseRquestedCallbackToken;
    EventRegistrationToken webResponseReceivedCallbackToken;

    EventCallBack cookieRetrievedCallbackInstance = nullptr;


public:
    MyWebview(LPCWSTR objectName, EventCallBack navigationEvent, EventCallBack responseReceivedEvent);

    /// <summary>
    /// Call webview and environment creation of webview
    /// </summary>
    /// <param name="url">initial url</param>
    /// <param name="startVisible">makes the webview visible at creation if TRUE</param>
    /// <param name="browserPath">path to the webview2 executable</param>
    /// <param name="dataPath">path to user navigation data</param>
    /// <param name="windowName">(optional)name of the window to overlay the webview</param>
    /// <returns></returns>
    int create(LPCWSTR url, bool startVisible = false, LPCWSTR browserPath = NULL, LPCWSTR dataPath = NULL, LPCWSTR windowName = NULL);

    /// <summary>
    /// Navigates to given url
    /// </summary>
    /// <param name="url">url to navigate to</param>
    void navigate(LPCWSTR url);
    /// <summary>
    /// Displays html given in htmlContent parameter
    /// </summary>
    /// <param name="htmlContent">html to displays</param>
    void navigateToHTML(LPCWSTR htmlContent);

    /// <summary>
    /// Close this webview
    /// </summary>
    void closeWebView();

    /// <summary>
    /// Change rect of webview (position and size)
    /// </summary>
    /// <param name="rect">Future rect of webview</param>
    void setBounds(RECT rect);

    /// <summary>
    /// Gets the current activity of webview
    /// </summary>
    /// <returns>TRUE if active, FALSE instead</returns>
    bool isActive();

    /// <summary>
    /// Gets the current visibility of the webview
    /// </summary>
    /// <returns>TRUE if visible, FALSE instead</returns>
    bool isVisible();
    /// <summary>
    /// Hides or displays webview
    /// </summary>
    /// <param name="visible">if TRUE display webview, hides it if FALSE</param>
    void setVisibility(bool visible);

    /// <summary>
    /// Call given js javascript given as first parameter.
    /// When script ends, call given cb if not null
    /// </summary>
    /// <param name="js">javascript script to be run</param>
    /// <param name="cb">callback with result of script</param>
    void runJavascript(LPCWSTR js, JSCallBack cb);

    /// <summary>
    /// Set auth url to be able to get content of response if needed
    /// </summary>
    /// <param name="url"></param>
    void setAuthUrl(LPCWSTR url);

    /// <summary>
    /// Get cookies depending given url
    /// </summary>
    /// <param name="url">url to retrieve cookies about</param>
    /// <param name="cookiesCallback">Callback called when cookies will be reterieved</param>
    void getCookies(LPCWSTR url, EventCallBack cookiesCallback);
    /// <summary>
    /// Save cookies to a file which will be loaded at start if exists
    /// </summary>
    /// <param name="url">select cookies on url</param>
    bool saveCookies(LPCWSTR url);

    /// <summary>
    /// register navigation callback
    /// [OBSOLETE] now moved to constructor of the webview
    /// </summary>
    /// <param name="cb"></param>
    void registerNavCallback(EventCallBack cb);

    /// <summary>
    /// register response received callbakc
    /// </summary>
    /// <param name="cb"></param>
    void registerResponseReceivedCallback(EventCallBack cb);

    ~MyWebview() {
        webviewController = nullptr;
        webviewWindow = nullptr;
    }

private:
    std::wstring boolToString(BOOL value);
    std::wstring encodeQuote(std::wstring raw);

    /// <summary>
    /// Converts a list of webview2 cookie to json string
    /// </summary>
    /// <param name="cookie">webview2 cookie list to convert</param>
    /// <returns>json representation of webview2 cookie list as string</returns>
    std::wstring cookieListToString(ICoreWebView2CookieList* list);

    /// <summary>
    /// Converts webview2 cookie (ICoreWebView2Cookie) to json string
    /// </summary>
    /// <param name="cookie">webview2 cookie to convert</param>
    /// <returns>json representation of webview2 cookie as string</returns>
    std::wstring cookieToString(ICoreWebView2Cookie* cookie);

    std::wstring responseToJsonString(
        ICoreWebView2WebResourceResponseView* response, std::wstring url, IStream* content);

    std::wstring getResponseContent(IStream* content, int contentLength);

    std::wstring responseHeadersToJsonString(ICoreWebView2HttpResponseHeaders* responseHeaders);

    /// <summary>
    /// Converts back webview from string
    /// </summary>
    /// <param name="json">json string to convert to cookie object</param>
    /// <returns>parsed cookie</returns>
    /// <returns>true if cookies has been loaded successfully</returns>
    bool loadCookies();
};