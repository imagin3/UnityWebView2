#pragma once
#include <Windows.h>
#include <WebView2.h>
#include <vector>
#include <string>
#include <tchar.h>
#include <wil/com.h>

#include "common.h"

class MyWebview {
    LPCWSTR objectName;
    wil::com_ptr<ICoreWebView2Controller> webviewController;
    wil::com_ptr<ICoreWebView2> webviewWindow;


    EventCallBack navigationCompletedCallbackInstance = nullptr;
    EventRegistrationToken navigationCompletedCallbackToken;

    RECT bounds;

public:
    MyWebview(LPCWSTR objectName, LPCWSTR url, LPCWSTR browserPath = NULL, LPCWSTR dataPath = NULL, LPCWSTR windowName = NULL);

    void navigate(LPCWSTR url);
    void navigateToHTML(LPCWSTR htmlContent);
    void closeWebView();
    void setBounds(RECT rect);
    bool isActive();
    bool isVisible();

    void registerNavCallback(EventCallBack cb);

private:
    ~MyWebview() {
        webviewController = nullptr;
        webviewWindow = nullptr;
    }
};