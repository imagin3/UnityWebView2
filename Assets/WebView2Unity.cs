using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEngine;
using UnityEngine.UI;

namespace WebView2Unity
{
    public class WebView2Unity : MonoBehaviour
    {
        public Image ImageRef;


        // Start is called before the first frame update
        void Start()
        {
            WebView2Native.RegisterDebugCallback();
            DirectoryInfo dir = new DirectoryInfo(Application.dataPath + "/web/");
            if (!dir.Exists)
                dir.Create();
#if UNITY_EDITOR
            WebView2Native.Create(name, "http://www.google.com", null, dir.FullName, "UnityEditor.GameView" );
#else
            WebView2Native.Create(name, "http://www.google.com", null, dir.FullName);
#endif
        }

        // Update is called once per frame
        void Update()
        {
            WebView2Native.UpdateBound(name, UIHelper.GetCanvasRectangle(ImageRef.rectTransform));
        }

        private void OnApplicationQuit()
        {
            if (WebView2Native.IsActive(name))
            {
                WebView2Native.Close(name);
            }
        }

        private void OnDestroy()
        {
            if (WebView2Native.IsActive(name))
            {
                WebView2Native.Close(name);
            }
        }

        private void OnDisable()
        {
            if (WebView2Native.IsActive(name))
            {
                WebView2Native.Close(name);
            }
        }

        private IEnumerator delayNavigate()
        {
            yield return new WaitForSeconds(1.0f);
            WebView2Native.NavigateToHTML(name, "<html><body><h1>Hello, WebView2!</h1></body></html>");
        }
    }
}