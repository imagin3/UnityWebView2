using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using UnityEngine.UI;

namespace WebView2Unity
{

    public class Saml2Auth : WebView2Unity
    {
        [SerializeField] GameObject buttonText;
        string targetUrl = "http://samlidp.iteca.lan/userinfos.php";
        string authUrl = "http://samlidp.iteca.lan/";
        string logoutUrl = "http://samlidp.iteca.lan/?slo";

        Action<bool> userAuth;

        bool loggedIn = false;

        // Start is called before the first frame update
        protected override void InitWebview()
        {
            NewWebView();
            SetAuthUrl();
            Create();
        }

        public void onAuthBtnClick()
        {
            if (loggedIn)
                LogOut();
            else
            {
                LogIn();
            }
        }

        public void LogIn()
        {
            Navigate(InitialURL);
            Show();
        }

        public void LogOut()
        {
            Navigate(logoutUrl);
            loggedIn = false;
            buttonText.GetComponent<Text>().text = "Connexion";
        }

        protected void SetAuthUrl()
        {
            InitialURL = authUrl + "?returnto=" + targetUrl;
            WebView2Native.SetAuthUrl(gameObject.name, targetUrl);
        }

        protected override void callbackResponseReceived(string message)
        {
            Debug.Log("response " + message);
            string url = "";
            try
            {
                JObject json = JObject.Parse(message);
                url = (string)json["url"];
            }
            catch(Exception e)
            {
                Debug.LogError(e.Message);
            }
            if (url == targetUrl || url == targetUrl + "/")
            {
                // SamlAUth has ended
                loggedIn = true;

                if (buttonText != null)
                {
                    buttonText.GetComponent<Text>().text = "Logout";
                }
                Hide();

            }
            base.callbackResponseReceived(message);
        }

        // Update is called once per frame
        protected override void callbackNavigationCompleted(string url)
        {
            Debug.Log("navigated " + url);
            // SamlAUth has ended
            if (url == targetUrl || url == targetUrl + "/")
            {
                SaveCookies(url);
            }
            base.callbackNavigationCompleted(url);
        }
    }
}
