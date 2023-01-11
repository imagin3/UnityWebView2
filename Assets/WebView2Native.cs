using AOT;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;

namespace WebView2Unity
{
    public static class WebView2Native
    {
        private const string PLUGIN = "WebView2Plugin";
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern int newWebView(string objectName, EventCallBack cb = null);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern int createWebView(string objectName, string url, bool startVisible = true, string browserPath = null, string dataPath = null, string windowName = null);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void navigate(string objectName, string url);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void navigateToHTML(string objectName, string htmlContent);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void updateWebViewBound(string objectName, int x, int y, int width, int height);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void closeWebView(string objectName);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern bool isActive(string objectName);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern bool isVisible(string objectName);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void setVisibility(string objectName, bool visible);
        //Create string param callback delegate
        delegate void EventCallBack([MarshalAs(UnmanagedType.LPWStr)] IntPtr request, int size);
        //Create string param callback delegate
        delegate void JSCallBack(int errorCode, [MarshalAs(UnmanagedType.LPWStr)] IntPtr resultAsJsonObject, int size);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        static extern void RunJavaScript(string objectName, string script, JSCallBack callback);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        static extern void getCookies(string objectName, string url, EventCallBack cb = null);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        static extern bool saveCookies(string objectName, string url);

        [DllImport(PLUGIN)]
        static extern void RegisterDebugCallback(debugCallback cb);
        //Create string param callback delegate
        delegate void debugCallback([MarshalAs(UnmanagedType.LPWStr)] IntPtr request, int color, int size);
        enum Color { red, green, blue, black, white, yellow, orange };

        public static void RegisterDbgCb()
        {
            RegisterDebugCallback(OnDebugCallback);
        }
        [MonoPInvokeCallback(typeof(debugCallback))]
        static void OnDebugCallback(IntPtr request, int color, int size)
        {
            //Ptr to string
            string debug_string = Marshal.PtrToStringUni(request, size);

            //Add Specified Color
            debug_string =
                String.Format("{0}{1}{2}{3}{4}",
                "<color=",
                ((Color)color).ToString(),
                ">",
                debug_string,
                "</color>"
                );

            UnityEngine.Debug.Log(debug_string);
        }

        //static Dictionary<string, Action<string>> navigationCallbacks;
        static Dictionary<string, Action<string>> navigationCallbacks = new Dictionary<string, Action<string>>();
        static Dictionary<string, Action<string>> cookiesCallbacks = new Dictionary<string, Action<string>>();

        [MonoPInvokeCallback(typeof(EventCallBack))]
        public static void OnNavigationCompletedCallback(IntPtr request, int size)
        {
            if (navigationCallbacks != null && navigationCallbacks.Count > 0)
            {
                //Ptr to string
                string msg_string = Marshal.PtrToStringUni(request, size);

                string[] splittedMsg = msg_string.Split('@');
                string objName = splittedMsg[0];
                string message = splittedMsg.Length > 1 ? splittedMsg[1] : "";

                if (navigationCallbacks.ContainsKey(objName))
                    navigationCallbacks[objName](message);
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="objectName"></param>
        /// <param name="callback"></param>
        /// <returns> -1 for webview already created</returns>
        public static int NewWebView(string objectName, Action<string> callback)
        {
            if (!navigationCallbacks.ContainsKey(objectName))
            {
                navigationCallbacks.Add(objectName, callback);
                //RegisterNavigationCompletedCallback(objectName, OnNavigationCompletedCallback);
            }
            return newWebView(objectName, OnNavigationCompletedCallback);
        }

        public static int Create(string objectName, string url, bool startVisible = true, string browserPath = null, string dataPath = null, string windowName = null)
        {
#if UNITY_EDITOR
            return createWebView(objectName, url, startVisible, browserPath, dataPath, windowName);
#else
            return createWebView(objectName, url, startVisible, browserPath, dataPath);
#endif
        }

        public static void Navigate(string objectName, string url)
        {
            navigate(objectName, url);
        }

        public static void NavigateToHTML(string objectName, string htmlContent)
        {
            navigateToHTML(objectName, htmlContent);
        }

        public static void EvaluateJS(string objectName, string script)
        {
            if (!string.IsNullOrWhiteSpace(script))
            {
                RunJavaScript(objectName, script, (int error, IntPtr jsObject, int size) =>
                {
                    //Ptr to string
                    string result = Marshal.PtrToStringUni(jsObject, size);
                    Debug.Log(error + " : " + result);
                }
                );
            }
        }

        public static void UpdateBound(string objectName, int x, int y, int width, int height)
        {
            updateWebViewBound(objectName, x, y, width, height);
        }

        public static void UpdateBound(string objectName, Rect rect)
        {
            UpdateBound(objectName, (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
        }

        public static void Close(string objectName)
        {
            closeWebView(objectName);
        }

        public static bool IsActive(string objectName)
        {
            return isActive(objectName);
        }

        public static void SetVisibility(string objectName, bool visible)
        {
            setVisibility(objectName, visible);
        }

        public static bool IsVisible(string objectName)
        {
            return isVisible(objectName);
        }




        [MonoPInvokeCallback(typeof(EventCallBack))]
        public static void OnCookiesRetrieved(IntPtr request, int size)
        {
            if (cookiesCallbacks != null && cookiesCallbacks.Count > 0)
            {
                //Ptr to string
                string msg_string = Marshal.PtrToStringUni(request, size);

                string[] splittedMsg = msg_string.Split('@');
                string objName = splittedMsg[0];
                string message = splittedMsg.Length > 1 ? splittedMsg[1] : "";


                if (cookiesCallbacks.ContainsKey(objName))
                    cookiesCallbacks[objName](message);
            }
        }

        public static void GetCookies(string objectName, string url, Action<string> cookieRetrievedCB = null)
        {
            if (!cookiesCallbacks.ContainsKey(objectName) && cookieRetrievedCB != null)
            {
                cookiesCallbacks.Add(objectName, cookieRetrievedCB);
                //RegisterNavigationCompletedCallback(objectName, OnNavigationCompletedCallback);
            }
            getCookies(objectName, url, OnCookiesRetrieved);
        }

        public static void SaveCookies(string objectName, string url)
        {
            saveCookies(objectName, url);
        }
    }
}