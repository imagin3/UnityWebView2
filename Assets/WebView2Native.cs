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
        private static extern void createWebView(string url, string browserPath = null, string dataPath = null);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void navigate(string url);
        [DllImport(PLUGIN, CharSet = CharSet.Unicode)]
        private static extern void navigateToHTML(string htmlContent);
        [DllImport(PLUGIN)]
        private static extern void updateWebViewBound(int x, int y, int width, int height);
        [DllImport(PLUGIN)]
        private static extern void closeWebView();
        [DllImport(PLUGIN)]
        private static extern bool isActive();

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
            string debug_string = Marshal.PtrToStringAnsi(request, size);

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

        public static void Create(string url, string browserPath = null, string dataPath = null)
        {
            createWebView(url, browserPath, dataPath);
        }

        public static void Navigate(string url)
        {
            navigate(url);
        }

        public static void NavigateToHTML(string htmlContent)
        {
            navigateToHTML(htmlContent);
        }

        public static void UpdateBound(int x, int y, int width, int height)
        {
            updateWebViewBound(x, y, width, height);
        }

        public static void UpdateBound(Rect rect)
        {
            UpdateBound((int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
        }

        public static void Close()
        {
            closeWebView();
        }

        public static bool IsActive()
        {
            return isActive();
        }
    }

    public static class UIHelper
    {
        public static Rect GetCanvasRectangle(RectTransform rectTransform)
        {
            Vector3[] corners = new Vector3[4];
            rectTransform.GetWorldCorners(corners);

            return new Rect(
                corners[1].x, Screen.height - corners[1].y,
                corners[3].x - corners[1].x,
                corners[1].y - corners[3].y
                );
        }
    }
}