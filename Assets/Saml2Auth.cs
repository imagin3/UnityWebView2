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
        string targetUrl = "http://iteca.lan";
        string authUrl = "http://samlidp.iteca.lan/";

        // Start is called before the first frame update
        void Awake()
        {
            if (webview == null)
                webview = GetComponent<WebView2Unity>();
            
            webview.OnPageLoaded += OnPageLoaded;
            //webview.OnGetCookies += OnGetCookies;
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

        //void OnGetCookies(string jsonCookies)
        //{
        //}
    }
}
