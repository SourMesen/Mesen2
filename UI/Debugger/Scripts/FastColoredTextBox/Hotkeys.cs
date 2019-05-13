using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.Drawing;
using System.Drawing.Design;
using System.Globalization;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Forms.Design;
using KEYS = System.Windows.Forms.Keys;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Dictionary of shortcuts for FCTB
    /// </summary>
    public class HotkeysMapping : SortedDictionary<Keys, FCTBAction>
    {
        public virtual void InitDefault()
        {
            this[KEYS.Control | KEYS.G] = FCTBAction.GoToDialog;
            this[KEYS.Control | KEYS.F] = FCTBAction.FindDialog;
            this[KEYS.Control | KEYS.Alt | KEYS.F] = FCTBAction.FindChar;
            this[KEYS.F3] = FCTBAction.FindNext;
            this[KEYS.Control | KEYS.H] = FCTBAction.ReplaceDialog;
            //this[KEYS.Control | KEYS.C] = FCTBAction.Copy;
            this[KEYS.Control | KEYS.Shift | KEYS.C] = FCTBAction.CommentSelected;
            //this[KEYS.Control | KEYS.X] = FCTBAction.Cut;
            //this[KEYS.Control | KEYS.V] = FCTBAction.Paste;
            //this[KEYS.Control | KEYS.A] = FCTBAction.SelectAll;
            this[KEYS.Control | KEYS.Z] = FCTBAction.Undo;
            this[KEYS.Control | KEYS.Y] = FCTBAction.Redo;
            this[KEYS.Control | KEYS.U] = FCTBAction.UpperCase;
            this[KEYS.Shift | KEYS.Control | KEYS.U] = FCTBAction.LowerCase;
            this[KEYS.Control | KEYS.OemMinus] = FCTBAction.NavigateBackward;
            this[KEYS.Control | KEYS.Shift | KEYS.OemMinus] = FCTBAction.NavigateForward;
            this[KEYS.Control | KEYS.B] = FCTBAction.BookmarkLine;
            this[KEYS.Control | KEYS.Shift | KEYS.B] = FCTBAction.UnbookmarkLine;
            this[KEYS.Control | KEYS.N] = FCTBAction.GoNextBookmark;
            this[KEYS.Control | KEYS.Shift | KEYS.N] = FCTBAction.GoPrevBookmark;
            this[KEYS.Alt | KEYS.Back] = FCTBAction.Undo;
            this[KEYS.Control | KEYS.Back] = FCTBAction.ClearWordLeft;
            this[KEYS.Insert] = FCTBAction.ReplaceMode;
            this[KEYS.Control | KEYS.Insert] = FCTBAction.Copy;
            this[KEYS.Shift | KEYS.Insert] = FCTBAction.Paste;
            this[KEYS.Delete] = FCTBAction.DeleteCharRight;
            this[KEYS.Control | KEYS.Delete] = FCTBAction.ClearWordRight;
            this[KEYS.Shift | KEYS.Delete] = FCTBAction.Cut;
            this[KEYS.Left] = FCTBAction.GoLeft;
            this[KEYS.Shift | KEYS.Left] = FCTBAction.GoLeftWithSelection;
            this[KEYS.Control | KEYS.Left] = FCTBAction.GoWordLeft;
            this[KEYS.Control | KEYS.Shift | KEYS.Left] = FCTBAction.GoWordLeftWithSelection;
            this[KEYS.Alt | KEYS.Shift | KEYS.Left] = FCTBAction.GoLeft_ColumnSelectionMode;
            this[KEYS.Right] = FCTBAction.GoRight;
            this[KEYS.Shift | KEYS.Right] = FCTBAction.GoRightWithSelection;
            this[KEYS.Control | KEYS.Right] = FCTBAction.GoWordRight;
            this[KEYS.Control | KEYS.Shift | KEYS.Right] = FCTBAction.GoWordRightWithSelection;
            this[KEYS.Alt | KEYS.Shift | KEYS.Right] = FCTBAction.GoRight_ColumnSelectionMode;
            this[KEYS.Up] = FCTBAction.GoUp;
            this[KEYS.Shift | KEYS.Up] = FCTBAction.GoUpWithSelection;
            this[KEYS.Alt | KEYS.Shift | KEYS.Up] = FCTBAction.GoUp_ColumnSelectionMode;
            this[KEYS.Alt | KEYS.Up] = FCTBAction.MoveSelectedLinesUp;
            this[KEYS.Control | KEYS.Up] = FCTBAction.ScrollUp;
            this[KEYS.Down] = FCTBAction.GoDown;
            this[KEYS.Shift | KEYS.Down] = FCTBAction.GoDownWithSelection;
            this[KEYS.Alt | KEYS.Shift | KEYS.Down] = FCTBAction.GoDown_ColumnSelectionMode;
            this[KEYS.Alt | KEYS.Down] = FCTBAction.MoveSelectedLinesDown;
            this[KEYS.Control | KEYS.Down] = FCTBAction.ScrollDown;
            this[KEYS.PageUp] = FCTBAction.GoPageUp;
            this[KEYS.Shift | KEYS.PageUp] = FCTBAction.GoPageUpWithSelection;
            this[KEYS.PageDown] = FCTBAction.GoPageDown;
            this[KEYS.Shift | KEYS.PageDown] = FCTBAction.GoPageDownWithSelection;
            this[KEYS.Home] = FCTBAction.GoHome;
            this[KEYS.Shift | KEYS.Home] = FCTBAction.GoHomeWithSelection;
            this[KEYS.Control | KEYS.Home] = FCTBAction.GoFirstLine;
            this[KEYS.Control | KEYS.Shift | KEYS.Home] = FCTBAction.GoFirstLineWithSelection;
            this[KEYS.End] = FCTBAction.GoEnd;
            this[KEYS.Shift | KEYS.End] = FCTBAction.GoEndWithSelection;
            this[KEYS.Control | KEYS.End] = FCTBAction.GoLastLine;
            this[KEYS.Control | KEYS.Shift | KEYS.End] = FCTBAction.GoLastLineWithSelection;
            this[KEYS.Escape] = FCTBAction.ClearHints;
            this[KEYS.Control | KEYS.M] = FCTBAction.MacroRecord;
            this[KEYS.Control | KEYS.E] = FCTBAction.MacroExecute;
            this[KEYS.Control | KEYS.Space] = FCTBAction.AutocompleteMenu;
            this[KEYS.Tab] = FCTBAction.IndentIncrease;
            this[KEYS.Shift | KEYS.Tab] = FCTBAction.IndentDecrease;
            this[KEYS.Control | KEYS.Subtract] = FCTBAction.ZoomOut;
            this[KEYS.Control | KEYS.Add] = FCTBAction.ZoomIn;
            this[KEYS.Control | KEYS.D0] = FCTBAction.ZoomNormal;
            this[KEYS.Control | KEYS.I] = FCTBAction.AutoIndentChars;   
        }

        public override string ToString()
        {
            var cult = Thread.CurrentThread.CurrentUICulture;
            Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;
            StringBuilder sb = new StringBuilder();
            var kc = new KeysConverter();
            foreach (var pair in this)
            {
                sb.AppendFormat("{0}={1}, ", kc.ConvertToString(pair.Key), pair.Value);
            }

            if (sb.Length > 1)
                sb.Remove(sb.Length - 2, 2);
            Thread.CurrentThread.CurrentUICulture = cult;

            return sb.ToString();
        }

        public static HotkeysMapping Parse(string s)
        {
            var result = new HotkeysMapping();
            result.Clear();
            var cult = Thread.CurrentThread.CurrentUICulture;
            Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

            var kc = new KeysConverter();
            
            foreach (var p in s.Split(','))
            {
                var pp = p.Split('=');
                var k = (Keys)kc.ConvertFromString(pp[0].Trim());
                var a = (FCTBAction)Enum.Parse(typeof(FCTBAction), pp[1].Trim());
                result[k] = a;
            }

            Thread.CurrentThread.CurrentUICulture = cult;

            return result;
        }
    }

    /// <summary>
    /// Actions for shortcuts
    /// </summary>
    public enum FCTBAction
    {
        None,
        AutocompleteMenu,
        AutoIndentChars,
        BookmarkLine,
        ClearHints,
        ClearWordLeft,
        ClearWordRight,
        CommentSelected,
        Copy,
        Cut,
        DeleteCharRight,
        FindChar,
        FindDialog,
        FindNext,
        GoDown,
        GoDownWithSelection,
        GoDown_ColumnSelectionMode,
        GoEnd,
        GoEndWithSelection,
        GoFirstLine,
        GoFirstLineWithSelection,
        GoHome,
        GoHomeWithSelection,
        GoLastLine,
        GoLastLineWithSelection,
        GoLeft,
        GoLeftWithSelection,
        GoLeft_ColumnSelectionMode,
        GoPageDown,
        GoPageDownWithSelection,
        GoPageUp,
        GoPageUpWithSelection,
        GoRight,
        GoRightWithSelection,
        GoRight_ColumnSelectionMode,
        GoToDialog,
        GoNextBookmark,
        GoPrevBookmark,
        GoUp,
        GoUpWithSelection,
        GoUp_ColumnSelectionMode,
        GoWordLeft,
        GoWordLeftWithSelection,
        GoWordRight,
        GoWordRightWithSelection,
        IndentIncrease,
        IndentDecrease,
        LowerCase,
        MacroExecute,
        MacroRecord,
        MoveSelectedLinesDown,
        MoveSelectedLinesUp,
        NavigateBackward,
        NavigateForward,
        Paste,
        Redo,
        ReplaceDialog,
        ReplaceMode,
        ScrollDown,
        ScrollUp,
        SelectAll,
        UnbookmarkLine,
        Undo,
        UpperCase,
        ZoomIn,
        ZoomNormal,
        ZoomOut,
        CustomAction1,
        CustomAction2,
        CustomAction3,
        CustomAction4,
        CustomAction5,
        CustomAction6,
        CustomAction7,
        CustomAction8,
        CustomAction9,
        CustomAction10,
        CustomAction11,
        CustomAction12,
        CustomAction13,
        CustomAction14,
        CustomAction15,
        CustomAction16,
        CustomAction17,
        CustomAction18,
        CustomAction19,
        CustomAction20
    }

    internal class HotkeysEditor : UITypeEditor
    {
        public override UITypeEditorEditStyle GetEditStyle(System.ComponentModel.ITypeDescriptorContext context)
        {
            return UITypeEditorEditStyle.Modal;
        }

        public override object EditValue(ITypeDescriptorContext context, IServiceProvider provider, object value)
        {
            if ((provider != null) && (((IWindowsFormsEditorService) provider.GetService(typeof(IWindowsFormsEditorService))) != null))
            {
                var form = new HotkeysEditorForm(HotkeysMapping.Parse(value as string));

                if (form.ShowDialog() == DialogResult.OK)
                    value = form.GetHotkeys().ToString();
            }
            return value;
        }
    }
}
