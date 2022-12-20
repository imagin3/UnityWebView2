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
        private static extern void createWebView(string objectName, string url, string browserPath = null, string dataPath = null, string windowName = null);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void navigate(string objectName, string url);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void navigateToHTML(string objectName, string htmlContent);
        [DllImport(PLUGIN)]
        private static extern void updateWebViewBound(string objectName, int x, int y, int width, int height);
        [DllImport(PLUGIN)]
        private static extern void closeWebView(string objectName);
        [DllImport(PLUGIN)]
        private static extern bool isActive(string objectName);
        [DllImport(PLUGIN)]
        private static extern bool isVisible(string objectName);

        [DllImport(PLUGIN)]
        static extern void RegisterDebugCallback(debugCallback cb);
        //Create string param callback delegate
        delegate void debugCallback(IntPtr request, int color, int size);
        enum Color { red, green, blue, black, white, yellow, orange };

        public static void RegisterDebugCallback()
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


        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        static extern void RegisterNavigationCompletedCallback(string objectName, EventCallBack cb);
        //Create string param callback delegate
        public delegate void EventCallBack(IntPtr request, int size);

        public static void RegisterNavigationCompletedCB(string objectName, EventCallBack cb)
        {
            RegisterNavigationCompletedCallback(objectName, cb);
        }

        public static void Create(string objectName, string url, string browserPath = null, string dataPath = null, string windowName = null)
        {
            createWebView(objectName, url, browserPath, dataPath, windowName);
        }

        public static void Navigate(string objectName, string url)
        {
            navigate(objectName, url);
        }

        public static void NavigateToHTML(string objectName, string htmlContent)
        {
            navigateToHTML(objectName, htmlContent);
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

        public static bool IsVisible(string objectName)
        {
            return isVisible(objectName);
        }
    }

    public static class UIHelper
    {
        public static Rect GetCanvasRectangle(RectTransform rectTransform)
        {
            Vector3[] corners = new Vector3[4];
            rectTransform.GetWorldCorners(corners);

            #if UNITY_EDITOR
            #endif

            return new Rect(
                corners[1].x, Screen.height - corners[1].y,
                corners[3].x - corners[1].x,
                corners[1].y - corners[3].y
                );
        }
    }
}