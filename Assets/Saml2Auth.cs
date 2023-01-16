using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
namespace WebView2Unity
{

    [RequireComponent(typeof(WebView2Unity))]
    public class Saml2Auth : MonoBehaviour
    {
        WebView2Unity webview;
        string targetUrl = "http://samlidp.iteca.lan/userinfos.php";
        string authUrl = "http://samlidp.iteca.lan/";


        Action<bool> userAuth;

        // Start is called before the first frame update
        void Awake()
        {
            if (webview == null)
                webview = GetComponent<WebView2Unity>();

            webview.InitialURL = authUrl + "?returnto=" + targetUrl;

            //webview.OnPageLoaded += OnPageLoaded;
            webview.OnResponseReceived += OnResponseReceived;
        }

        private void OnResponseReceived(string obj)
        {

            //if (userAuth != null)
            //    userAuth(obj);
        }

        // Update is called once per frame
        void OnPageLoaded(string url)
        {
            // SamlAUth has ended
            if (url == targetUrl || url == targetUrl + "/")
            {
                webview.SaveCookies(url);
            }
        }
    }
}
