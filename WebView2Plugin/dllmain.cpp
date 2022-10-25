#include <Windows.h>
#include <WebView2.h>
#include <string>
#include <tchar.h>
#include <vector>
#include <wrl.h>
#include <wil/com.h>

#define PLUGIN_API extern "C" __declspec(dllexport)

// DEBUG LOG by
// https://github.com/programmercert/UnityNativeDebugLog
// all debug funcs not implemented

#define DLLExport __declspec(dllexport)

extern "C"
{
    //Create a callback delegate
    typedef void(*FuncCallBack)(LPCWSTR message, int color, int size);
    static FuncCallBack callbackInstance = nullptr;
    DLLExport void RegisterDebugCallback(FuncCallBack cb);
}

//Color Enum
enum class Color { Red, Green, Blue, Black, White, Yellow, Orange };

//-------------------------------------------------------------------
void  Log(LPCWSTR message, Color color = Color::White) {
    if (callbackInstance != nullptr)
        callbackInstance(message, (int)color, (int)wcslen(message));
}
//-------------------------------------------------------------------

//Create a callback delegate
void RegisterDebugCallback(FuncCallBack cb) {
    callbackInstance = cb;
}

//-------------------------------------------------------------------
using namespace Microsoft::WRL;

static wil::com_ptr<ICoreWebView2Controller> webviewController;
static wil::com_ptr<ICoreWebView2> webviewWindow;


static RECT bounds;

PLUGIN_API void createWebView(LPCWSTR url, LPCWSTR browserPath = NULL, LPCWSTR dataPath = NULL) {
    HWND hWnd = GetActiveWindow();

	if (hWnd == NULL) {
		return;
	}

    Log(L"create");

    //https://docs.microsoft.com/ja-jp/microsoft-edge/webview2/gettingstarted/win32
    CreateCoreWebView2EnvironmentWithOptions(browserPath, dataPath, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hWnd, url](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

                // Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
                env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [hWnd, url](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
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
}

PLUGIN_API void navigate(LPCWSTR url) {
    Log(L"navigate");
    if (webviewWindow != nullptr) {
        webviewWindow->Navigate(url);
    }
}

PLUGIN_API void navigateToHTML(LPCWSTR htmlContent) {
    if (webviewWindow != nullptr) {
        webviewWindow->NavigateToString(htmlContent);
    }
}

PLUGIN_API void updateWebViewBound(int x, int y, int width, int height) {
    RECT bounds;
    bounds.left = x;
    bounds.top = y;
    bounds.right = x + width;
    bounds.bottom = y + height;
    if (webviewController != nullptr) {
        webviewController->put_Bounds(bounds);
    }
}

PLUGIN_API void closeWebView() {
    if (webviewController != nullptr) {
        webviewController->Close();
        webviewController = nullptr;
        webviewWindow = nullptr;
    }
}

PLUGIN_API bool isActive() {
    return webviewController != nullptr;
}