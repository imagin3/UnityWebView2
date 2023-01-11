#include "MyWebview.h"
#include <comdef.h>
#include <wrl.h>
#include <WebView2EnvironmentOptions.h>
#include <regex>

using namespace Microsoft::WRL;

MyWebview::MyWebview(LPCWSTR name, EventCallBack navigationEvent)
{
    objectName = name;
    
    registerNavCallback(navigationEvent);
}

int MyWebview::create(LPCWSTR url, bool startVisible, LPCWSTR browserPath, LPCWSTR dataPath, LPCWSTR windowName) {

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
        return -3; // no hwnd found
    }

    userDataPath = dataPath;
    cookiesDataPath = userDataPath;
    if (!userDataPath.empty())
    {
        std::wstring web = L"\\web";
        cookiesDataPath = userDataPath.replace(userDataPath.find(web), web.size(), L"\\");
    }

    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AllowSingleSignOnUsingOSPrimaryAccount(TRUE);

    //https://docs.microsoft.com/ja-jp/microsoft-edge/webview2/gettingstarted/win32
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(browserPath, dataPath, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this, hWnd, url, startVisible](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

                // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
                env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [this, hWnd, url, startVisible](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                        if (controller != nullptr) {
                            webviewController = controller;
                            webviewController->get_CoreWebView2(&webviewWindow);
                        }

                        //! [CookieManager]
                        auto webview2_2 = webviewWindow.try_query<ICoreWebView2_2>();
                        if (webview2_2 != nullptr)
                            webview2_2->get_CookieManager(&cookieManager);
                        //! [CookieManager]

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
                                /*wchar_t szBuff[512];
                                swprintf_s(szBuff, 512, L"%s@%s", objectName, uri.get());*/
                                std::wstring message = objectName + L"@" + uri.get();
                                navigationCompletedCallbackInstance(message.c_str(), (int)message.size());
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

                        if (url != nullptr)
                        {
                            // URLを指定して表示
                            webviewWindow->Navigate(url);
                        }

                        webviewController->put_IsVisible(startVisible);

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

        return -2; // creation error;
    }

    return (int)hr;
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

void MyWebview::setVisibility(bool visible)
{
    if (webviewController != nullptr)
    {
        webviewController->put_IsVisible(visible);
    }
}

void MyWebview::registerNavCallback(EventCallBack cb)
{
    navigationCompletedCallbackInstance = cb;
}

void MyWebview::runJavascript(LPCWSTR js, JSCallBack cb) {
    if (webviewWindow != nullptr)
    {
        webviewWindow->ExecuteScript(js, Callback<ICoreWebView2ExecuteScriptCompletedHandler>([this, cb](HRESULT error, LPCWSTR result) -> HRESULT {
            if (cb != nullptr)
                cb(error, result, wcslen(result));
            return S_OK;
        }).Get());
    }
}

void MyWebview::getCookies(LPCWSTR url, EventCallBack callback) {
    if (cookieManager != nullptr)
    {
        cookieRetrievedCallbackInstance = callback;

        cookieManager->GetCookies(
            url,
            Callback<ICoreWebView2GetCookiesCompletedHandler>(
                [this](HRESULT error_code, ICoreWebView2CookieList* list) -> HRESULT {
            std::wstring cookies = cookieListToString(list);

            if (cookieRetrievedCallbackInstance != nullptr)
            {
                cookies = objectName + L"@" + cookies;
                cookieRetrievedCallbackInstance(cookies.c_str(), (int)cookies.size());
            }

            return S_OK;
        }).Get());
    }
}

bool MyWebview::saveCookies(LPCWSTR url)
{
    if (cookieManager != nullptr && !cookiesDataPath.empty())
    {
        cookieManager->GetCookies(
            url,
            Callback<ICoreWebView2GetCookiesCompletedHandler>(
                [this](HRESULT error_code, ICoreWebView2CookieList* list) -> HRESULT {
            std::wstring cookies = cookieListToString(list);

            std::ofstream cookieFile;
            cookieFile.open(cookiesDataPath + L"cookies.dat", std::ios::trunc || std::ios::out);
            cookieFile << cookies.c_str();
            cookieFile.close();

            return S_OK;
        }).Get());
    }
    return true;
}

std::wstring MyWebview::boolToString(BOOL value)
{
    return value ? L"true" : L"false";
}

std::wstring MyWebview::encodeQuote(std::wstring raw)
{
    return L"\"" + regex_replace(raw, std::wregex(L"\""), L"\\\"") + L"\"";
}

std::wstring MyWebview::cookieListToString(ICoreWebView2CookieList* list)
{
    std::wstring cookies = L"{\n";

    UINT cookie_list_size;
    list->get_Count(&cookie_list_size);
    if (cookie_list_size != 0)
    {
        //cookies += L"[";
        for (UINT i = 0; i < cookie_list_size; ++i)
        {
            wil::com_ptr<ICoreWebView2Cookie> cookie;
            list->GetValueAtIndex(i, &cookie);

            if (cookie.get())
            {
                cookies += cookieToString(cookie.get());
                if (i != cookie_list_size - 1)
                {
                    cookies += L",\n";
                }
            }
        }
        cookies += L"]";
    }
    cookies += L"}";

    return cookies;
}

std::wstring MyWebview::cookieToString(ICoreWebView2Cookie* cookie)
{
    //! [CookieObject]
    wil::unique_cotaskmem_string name;
    cookie->get_Name(&name);
    wil::unique_cotaskmem_string value;
    cookie->get_Value(&value);
    wil::unique_cotaskmem_string domain;
    cookie->get_Domain(&domain);
    wil::unique_cotaskmem_string path;
    cookie->get_Path(&path);
    double expires;
    cookie->get_Expires(&expires);
    BOOL isHttpOnly = FALSE;
    cookie->get_IsHttpOnly(&isHttpOnly);
    COREWEBVIEW2_COOKIE_SAME_SITE_KIND same_site;
    std::wstring same_site_as_string;
    cookie->get_SameSite(&same_site);
    switch (same_site)
    {
    case COREWEBVIEW2_COOKIE_SAME_SITE_KIND_NONE:
        same_site_as_string = L"None";
        break;
    case COREWEBVIEW2_COOKIE_SAME_SITE_KIND_LAX:
        same_site_as_string = L"Lax";
        break;
    case COREWEBVIEW2_COOKIE_SAME_SITE_KIND_STRICT:
        same_site_as_string = L"Strict";
        break;
    }
    BOOL isSecure = FALSE;
    cookie->get_IsSecure(&isSecure);
    BOOL isSession = FALSE;
    cookie->get_IsSession(&isSession);

    std::wstring result = L"{";
    result += L"\"Name\": " + encodeQuote(name.get()) + L", " + L"\"Value\": " +
        encodeQuote(value.get()) + L", " + L"\"Domain\": " + encodeQuote(domain.get()) +
        L", " + L"\"Path\": " + encodeQuote(path.get()) + L", " + L"\"HttpOnly\": " +
        boolToString(isHttpOnly) + L", " + L"\"Secure\": " + boolToString(isSecure) + L", " +
        L"\"SameSite\": " + encodeQuote(same_site_as_string) + L", " + L"\"Expires\": ";
    if (!!isSession)
    {
        result += L"This is a session cookie.";
    }
    else
    {
        result += std::to_wstring(expires);
    }

    return result + L"\"}";
    //! [CookieObject]
}