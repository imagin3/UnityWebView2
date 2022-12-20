#ifndef __log_h
#define __log_h

#include <Windows.h>
#include <string>
#include <tchar.h>

// DEBUG LOG by
// https://github.com/programmercert/UnityNativeDebugLog
// all debug funcs not implemented

#define DLLExport __declspec(dllexport)

extern "C"
{

    // events callback
    typedef void(*EventCallBack)(LPCWSTR message, int size);


    typedef void(*JSCallBack)(int code, LPCWSTR result, int size);


    typedef void(*FuncCallBack)(LPCWSTR message, int color, int size);
    static FuncCallBack dbgCallbackInstance = nullptr;

    /// <summary>
    /// Register log message callback
    /// </summary>
    /// <param name="cb">function called back</param>
    /// <returns></returns>
    DLLExport void RegisterDebugCallback(FuncCallBack cb);
}

//Color Enum
enum class Color { Red, Green, Blue, Black, White, Yellow, Orange };

void Log(LPCWSTR message, Color color = Color::White);

#endif