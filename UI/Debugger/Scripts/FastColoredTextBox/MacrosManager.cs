using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.Xml;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// This class records, stores and executes the macros.
    /// </summary>
    public class MacrosManager
    {
        private readonly List<object> macro = new List<object>();

        internal MacrosManager(FastColoredTextBox ctrl)
        {
            UnderlayingControl = ctrl;
            AllowMacroRecordingByUser = true;
        }

        /// <summary>
        /// Allows to user to record macros
        /// </summary>
        public bool AllowMacroRecordingByUser { get;set; }

        private bool isRecording;

        /// <summary>
        /// Returns current recording state. Set to True/False to start/stop recording programmatically.
        /// </summary>
        public bool IsRecording
        {
            get { return isRecording; }
            set { isRecording = value; UnderlayingControl.Invalidate(); }
        }

        /// <summary>
        /// FCTB
        /// </summary>
        public FastColoredTextBox UnderlayingControl { get; private set; }

        /// <summary>
        /// Executes recorded macro
        /// </summary>
        /// <returns></returns>
        public void ExecuteMacros()
        {
            IsRecording = false;
            UnderlayingControl.BeginUpdate();
            UnderlayingControl.Selection.BeginUpdate();
            UnderlayingControl.BeginAutoUndo();
            foreach (var item in macro)
            {
                if (item is Keys)
                {
                    UnderlayingControl.ProcessKey((Keys)item);
                }
                if (item is KeyValuePair<char, Keys>)
                {
                    var p = (KeyValuePair<char, Keys>)item;
                    UnderlayingControl.ProcessKey(p.Key, p.Value);
                }
                
            }
            UnderlayingControl.EndAutoUndo();
            UnderlayingControl.Selection.EndUpdate();
            UnderlayingControl.EndUpdate();
        }

        /// <summary>
        /// Adds the char to current macro
        /// </summary>
        public void AddCharToMacros(char c, Keys modifiers)
        {
            macro.Add(new KeyValuePair<char, Keys>(c, modifiers));
        }

        /// <summary>
        /// Adds keyboard key to current macro
        /// </summary>
        public void AddKeyToMacros(Keys keyData)
        {
            macro.Add(keyData);
        }

        /// <summary>
        /// Clears last recorded macro
        /// </summary>
        public void ClearMacros()
        {
            macro.Clear();
        }


        internal void ProcessKey(Keys keyData)
        {
            if (IsRecording)
                AddKeyToMacros(keyData);
        }

        internal void ProcessKey(char c, Keys modifiers)
        {
            if (IsRecording)
                AddCharToMacros(c, modifiers);
        }

        /// <summary>
        /// Returns True if last macro is empty
        /// </summary>
        public bool MacroIsEmpty { get { return macro.Count == 0; }}

        /// <summary>
        /// Macros as string.
        /// </summary>
        public string Macros
        {
            get
            {
                var cult = Thread.CurrentThread.CurrentUICulture;
                Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;
                var kc = new KeysConverter();

                StringBuilder sb = new StringBuilder();
                sb.AppendLine("<macros>");
                foreach (var item in macro)
                {
                    if (item is Keys)
                    {
                        sb.AppendFormat("<item key='{0}' />\r\n", kc.ConvertToString((Keys)item));
                    }
                    else if (item is KeyValuePair<char, Keys>)
                    {
                        var p = (KeyValuePair<char, Keys>)item;
                        sb.AppendFormat("<item char='{0}' key='{1}' />\r\n", (int)p.Key, kc.ConvertToString(p.Value));
                    }
                }
                sb.AppendLine("</macros>");

                Thread.CurrentThread.CurrentUICulture = cult;

                return sb.ToString();
            }

            set 
            {
                isRecording = false;
                ClearMacros();

                if (string.IsNullOrEmpty(value))
                    return;

                var doc = new XmlDocument();
                doc.LoadXml(value);
                var list = doc.SelectNodes("./macros/item");

                var cult = Thread.CurrentThread.CurrentUICulture;
                Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;
                var kc = new KeysConverter();

                if(list != null)
                foreach (XmlElement node in list)
                {
                    var ca = node.GetAttributeNode("char");
                    var ka = node.GetAttributeNode("key");
                    if (ca != null)
                    {
                        if(ka!=null)
                            AddCharToMacros((char)int.Parse(ca.Value), (Keys)kc.ConvertFromString(ka.Value));
                        else
                            AddCharToMacros((char)int.Parse(ca.Value), Keys.None);
                    }else
                    if(ka!=null)
                            AddKeyToMacros((Keys)kc.ConvertFromString(ka.Value));
                }

                Thread.CurrentThread.CurrentUICulture = cult;
            }
        }
    }
}