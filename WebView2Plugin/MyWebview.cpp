#include "MyWebview.h"
#include <comdef.h>
#include <wrl.h>
#include <WebView2EnvironmentOptions.h>

using namespace Microsoft::WRL;

MyWebview::MyWebview(LPCWSTR name, LPCWSTR url, LPCWSTR browserPath, LPCWSTR dataPath, LPCWSTR windowName) {
    objectName = name;

    HWND parenthWnd = GetActiveWindow();
    HWND hWnd = parenthWnd;
    if (windowName != NULL)
    {
        hWnd = FindWindowEx(parenthWnd, NULL, NULL, windowName);
        if (hWnd != NULL)
        {
            wchar_t szBuff[128];
            swprintf_s(szBuff, 128, L"find window hwnd for %s with parent : %p, %p", windowName, hWnd, parenthWnd);
            Log(szBuff);
        }
    }

    if (hWnd == NULL) {
        Log(L"No hWnd !", Color::Red);
        return;
    }

    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(TRUE);

    //https://docs.microsoft.com/ja-jp/microsoft-edge/webview2/gettingstarted/win32
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(browserPath, dataPath, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this, hWnd, url](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

                // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
                env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [this, hWnd, url](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                        if (controller != nullptr) {
                            webviewController = controller;
                            webviewController->get_CoreWebView2(&webviewWindow);
                        }

                        // WebViewの設定
                        ICoreWebView2Settings* Settings;
                        webviewWindow->get_Settings(&Settings);
                        Settings->put_IsScriptEnabled(TRUE);
                        Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                        Settings->put_IsWebMessageEnabled(TRUE);

                        webviewWindow->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
                        [this](
                            ICoreWebView2*,
                            ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
                        {
                            if (navigationCompletedCallbackInstance != nullptr)
                            {
                                wil::unique_cotaskmem_string uri;
                                webviewWindow->get_Source(&uri);

                                if (wcscmp(uri.get(), L"about:blank") == 0)
                                {
                                    uri = wil::make_cotaskmem_string(L"");
                                }
                                wchar_t szBuff[512];
                                swprintf_s(szBuff, 512, L"navigation complete @ %s", uri.get());
                                navigationCompletedCallbackInstance(szBuff, (int)wcslen(szBuff));
                            }
                            return S_OK;
                        }).Get(), &navigationCompletedCallbackToken);

                        // 最初は親のウィンドウに合わせる
                        if (bounds.right == 0 && bounds.bottom == 0)
                        {
                            GetClientRect(hWnd, &bounds);
                        }
                        webviewController->put_Bounds(bounds);
                        Log(L"created");

                        // URLを指定して表示
                        webviewWindow->Navigate(url);

                        return S_OK;
                    }).Get());
                return S_OK;
            }).Get());

    if (!SUCCEEDED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            Log(L"Cannot found the Edge browser.", Color::Red);
        }
        else
        {
            Log(L"Cannot create the webview environment : ");
            _com_error err(hr);
            Log(err.ErrorMessage());
        }
    }
}

void MyWebview::navigate(LPCWSTR url)
{
    if (webviewWindow != nullptr)
    {
        webviewWindow->Navigate(url);
    }
}

void MyWebview::navigateToHTML(LPCWSTR htmlContent)
{
    if (webviewWindow != nullptr)
    {
        webviewWindow->NavigateToString(htmlContent);
    }
}

void MyWebview::closeWebView()
{
    if (webviewController != nullptr)
    {
        webviewController->Close();
        webviewController = nullptr;
        webviewWindow = nullptr;
    }
}

void MyWebview::setBounds(RECT rect)
{
    if (webviewController != nullptr) {
        webviewController->put_Bounds(bounds);
    }
}

bool MyWebview::isActive()
{
    return webviewController != nullptr;
}

bool MyWebview::isVisible()
{
    BOOL isvisible = FALSE;
    if (webviewController != nullptr)
    {
        webviewController->get_IsVisible(&isvisible);
    }
    return isvisible;
}


void MyWebview::registerNavCallback(EventCallBack cb)
{
    navigationCompletedCallbackInstance = cb;
}
