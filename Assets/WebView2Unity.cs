using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Net;
using UnityEngine;
using UnityEngine.UI;

namespace WebView2Unity
{
    public class WebView2Unity : MonoBehaviour
    {
        public bool debug = false;
        public Image ImageRef;
        public string InitialURL = "";

        private string browserPath = null;
        private DirectoryInfo userDataPath = null;

        /// <summary>
        /// Callback Event 
        /// </summary>
        public Action<string> OnPageLoaded;
        public Action<string> OnResponseReceived;
        public Action<string> OnGetCookies;

        /// <summary>
        /// Start webview displayed or not
        /// </summary>
        public bool showWebview = false;

        bool visibility = false;

        // Start is called before the first frame update
        void Start()
        {
            // TODO : Fix Bad Memory Management
            // Leads to freeze ....
            if (debug)
                WebView2Native.RegisterDbgCb();
            InitWebview();
        }

        protected virtual void InitWebview()
        {
            NewWebView();
            Create();
        }

        protected void NewWebView() {
            try
            {
                // TODO define 
                userDataPath = new DirectoryInfo(Application.persistentDataPath + "/../web");
                if (!userDataPath.Exists)
                {
                    userDataPath.Create();
                }
                // Don't look for local browser if already installed in dev
#if UNITY_EDITOR
                bool localMachine = RegistriyKeyExists(Microsoft.Win32.Registry.LocalMachine, @"SOFTWARE\WOW6432Node\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}");
                bool currentUser = RegistriyKeyExists(Microsoft.Win32.Registry.CurrentUser, @"Software\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}");
                if (!localMachine && !currentUser)
#endif
                {
                    browserPath = Path.Combine(Application.dataPath, @"..\Webview2Runtime");
                    DirectoryInfo browserDir = new DirectoryInfo(browserPath);
                    if (!browserDir.Exists)
                    {
#if !UNITY_EDITOR
                        DebugEx.Write("Webview Runtime folder does not exists !");
#else

                        bool retRuntimeInstall = UnityEditor.EditorUtility.DisplayDialog("Runtime pour webview2 inexistant", "Le runtime pour la webview n'est pas installé, voulez-vous l'installer ?", "Oui", "Non");
                        if (retRuntimeInstall)
                        {
                            Application.OpenURL("https://developer.microsoft.com/fr-fr/microsoft-edge/webview2/");
                        }
#endif
                    }
                    else browserPath = browserDir.FullName;
                }
                Debug.Log("new webview");
                WebView2Native.NewWebView(gameObject.name, callbackNavigationCompleted, callbackResponseReceived);

            }
            catch (Exception e)
            {
                Debug.LogError("Error creating webview" + e.Message);
            }
        }

        protected void Create()
        {
            try
            {
                string url = InitialURL;
                if (string.IsNullOrEmpty(url))
                    url = null;

                int created = WebView2Native.Create(gameObject.name, url, showWebview, browserPath, userDataPath.FullName, "UnityEditor.GameView");
                Debug.Log("created webview : " + created);
            }
            catch (Exception e)
            {
                Debug.LogError("Error creating webview" + e.Message);
            }
        }

        protected virtual void callbackResponseReceived(string message)
        {
            if (OnResponseReceived != null)
            {
                OnResponseReceived(message);
            }
        }

        protected virtual void callbackNavigationCompleted(string message){
            Debug.Log("show : " + gameObject.name + " = " + message);
            //Show();
            if (OnPageLoaded != null)
            {
                OnPageLoaded(message);
            }
        }

        public void Show()
        {
            visibility = true;
            WebView2Native.SetVisibility(gameObject.name, true);
        }

        public void Hide()
        {
            visibility = false;
            WebView2Native.SetVisibility(gameObject.name, false);
        }

        public bool isActive()
        {
            return WebView2Native.IsActive(gameObject.name);
        }

        public bool isVisible()
        {
            return visibility;// && WebView2Native.IsVisible(name); 
        }

        public void EvaluateJS(string script)
        {
            WebView2Native.EvaluateJS(gameObject.name, script);
        }

        public void Close()
        {
            WebView2Native.Close(gameObject.name);
        }

        public void Navigate(string url)
        {
            if (string.IsNullOrEmpty(url))
                return;
            WebView2Native.Navigate(gameObject.name, url);
        }

        public void LoadHTML(string html)
        {
            if (string.IsNullOrEmpty(html))
                return;
            WebView2Native.NavigateToHTML(gameObject.name, html);
        }

        public void LoadHTML(string html, string baseUrl)
        {
            if (string.IsNullOrEmpty(html))
                return;
            WebView2Native.NavigateToHTML(gameObject.name, html + baseUrl);
        }

        public void NavigateFile(string file)
        {
            if (string.IsNullOrEmpty(file))
                return;
            WebView2Native.Navigate(gameObject.name, file);
        }

        public void SetMargins(Rect rect)
        {
            WebView2Native.UpdateBound(gameObject.name, (int)rect.x, (int)rect.y, (int)rect.width, (int)rect.height);
        }

        public void SetMargins(int left, int top, int right, int bottom)
        {
            WebView2Native.UpdateBound(gameObject.name, left, top, right, bottom);
        }

        public void SetVisibility(bool visible)
        {
            WebView2Native.SetVisibility(gameObject.name, visible);
        }

        public bool GetVisibility()
        {
            return WebView2Native.IsVisible(gameObject.name);
        }
        
        public void GetCookies(string url)
        {
            WebView2Native.GetCookies(gameObject.name, url, OnGetCookies);
        }

        public void SaveCookies(string url)
        {
            WebView2Native.SaveCookies(gameObject.name, url);
        }

        private void OnApplicationQuit()
        {
            Close();
        }

        private void OnDestroy()
        {
            Close();
        }

        private void OnDisable()
        {
            Close();
        }
        private IEnumerator delayNavigate()
        {
            yield return new WaitForSeconds(1.0f);
            WebView2Native.NavigateToHTML(gameObject.name, "<html><body><h1>Hello, WebView2!</h1></body></html>");
        }

        #region System Utils
        // TODO move this to somewhere more global

        /// <summary>
        /// Check if subKeyName of baseKey exists in registry
        /// </summary>
        /// <param name="baseKey">base key in registry like Microsoft.Win32.RegistryKey.LocalMachine</param>
        /// <param name="subKeyName">path to key to check like "@SOFTWARE\...\MyKey"</param>
        /// <returns>true if Key exists, false if not</returns>
        public static bool RegistriyKeyExists(Microsoft.Win32.RegistryKey baseKey, string subKeyName)
        {
            Microsoft.Win32.RegistryKey ret = baseKey.OpenSubKey(subKeyName);

            return ret != null;
        }
        /// <summary>
        /// Check if Key of registry has value
        /// </summary>
        /// <param name="Key">key to check value</param>
        /// <param name="Value">value to check</param>
        /// <returns>true if Key has Value, false instead</returns>
        public static bool RegistriyValueExists(Microsoft.Win32.RegistryKey Key, string Value)
        {
            try
            {
                return Key.GetValue(Value) != null;
            }
            catch
            {
                return false;
            }
        }
        #endregion
    }
}