using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace FastColoredTextBoxNS
{
    public partial class HotkeysEditorForm : Form
    {
        BindingList<HotkeyWrapper> wrappers = new BindingList<HotkeyWrapper>();

        public HotkeysEditorForm(HotkeysMapping hotkeys)
        {
            InitializeComponent();
            BuildWrappers(hotkeys);
            dgv.DataSource = wrappers;
        }

        int CompereKeys(Keys key1, Keys key2)
        {
            var res = ((int)key1 & 0xff).CompareTo((int)key2 & 0xff);
            if (res == 0)
                res = key1.CompareTo(key2);

            return res;
        }

        private void BuildWrappers(HotkeysMapping hotkeys)
        {
            var keys = new List<Keys>(hotkeys.Keys);
            keys.Sort(CompereKeys);

            wrappers.Clear();
            foreach (var k in keys)
                wrappers.Add(new HotkeyWrapper(k, hotkeys[k]));
        }

        /// <summary>
        /// Returns edited hotkey map
        /// </summary>
        /// <returns></returns>
        public HotkeysMapping GetHotkeys()
        {
            var result = new HotkeysMapping();
            foreach (var w in wrappers)
                result[w.ToKeyData()] = w.Action;

            return result;
        }

        private void btAdd_Click(object sender, EventArgs e)
        {
            wrappers.Add(new HotkeyWrapper(Keys.None, FCTBAction.None));
        }

        private void dgv_RowsAdded(object sender, DataGridViewRowsAddedEventArgs e)
        {
            var cell = (dgv[0, e.RowIndex] as DataGridViewComboBoxCell);
            if(cell.Items.Count == 0)
            foreach(var item in new string[]{"", "Ctrl", "Ctrl + Shift", "Ctrl + Alt", "Shift", "Shift + Alt", "Alt", "Ctrl + Shift + Alt"})
                cell.Items.Add(item);

            cell = (dgv[1, e.RowIndex] as DataGridViewComboBoxCell);
            if (cell.Items.Count == 0)
            foreach (var item in Enum.GetValues(typeof(Keys)))
                cell.Items.Add(item);

            cell = (dgv[2, e.RowIndex] as DataGridViewComboBoxCell);
            if (cell.Items.Count == 0)
            foreach (var item in Enum.GetValues(typeof(FCTBAction)))
                cell.Items.Add(item);
        }

        private void btResore_Click(object sender, EventArgs e)
        {
            HotkeysMapping h = new HotkeysMapping();
            h.InitDefault();
            BuildWrappers(h);
        }

        private void btRemove_Click(object sender, EventArgs e)
        {
            for (int i = dgv.RowCount - 1; i >= 0; i--)
                if (dgv.Rows[i].Selected) dgv.Rows.RemoveAt(i);
        }

        private void HotkeysEditorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if(DialogResult == System.Windows.Forms.DialogResult.OK)
            {
                var actions = GetUnAssignedActions();
                if (!string.IsNullOrEmpty(actions))
                {
                    if (MessageBox.Show("Some actions are not assigned!\r\nActions: " + actions + "\r\nPress Yes to save and exit, press No to continue editing", "Some actions is not assigned", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == System.Windows.Forms.DialogResult.No)
                        e.Cancel = true;
                }
            }
        }

        private string GetUnAssignedActions()
        {
            StringBuilder sb = new StringBuilder();
            var dic = new Dictionary<FCTBAction, FCTBAction>();

            foreach (var w in wrappers)
                dic[w.Action] = w.Action;

            foreach (var item in Enum.GetValues(typeof(FCTBAction)))
            if ((FCTBAction)item != FCTBAction.None)
            if(!((FCTBAction)item).ToString().StartsWith("CustomAction"))
            {
                if(!dic.ContainsKey((FCTBAction)item))
                    sb.Append(item+", ");
            }

            return sb.ToString().TrimEnd(' ', ',');
        }
    }

    internal class HotkeyWrapper
    {
        public HotkeyWrapper(Keys keyData, FCTBAction action)
        {
            KeyEventArgs a = new KeyEventArgs(keyData);
            Ctrl = a.Control;
            Shift = a.Shift;
            Alt = a.Alt;

            Key = a.KeyCode;
            Action = action;
        }

        public Keys ToKeyData()
        {
            var res = Key;
            if (Ctrl) res |= Keys.Control;
            if (Alt) res |= Keys.Alt;
            if (Shift) res |= Keys.Shift;

            return res;
        }

        bool Ctrl;
        bool Shift;
        bool Alt;
        
        public string Modifiers
        {
            get
            {
                var res = "";
                if (Ctrl) res += "Ctrl + ";
                if (Shift) res += "Shift + ";
                if (Alt) res += "Alt + ";

                return res.Trim(' ', '+');
            }
            set
            {
                if (value == null)
                {
                    Ctrl = Alt = Shift = false;
                }
                else
                {
                    Ctrl = value.Contains("Ctrl");
                    Shift = value.Contains("Shift");
                    Alt = value.Contains("Alt");
                }
            }
        }

        public Keys Key { get; set; }
        public FCTBAction Action { get; set; }
    }
}
