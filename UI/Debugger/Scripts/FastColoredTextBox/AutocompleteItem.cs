using System;
using System.Drawing;
using System.Drawing.Printing;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Item of autocomplete menu
    /// </summary>
    public class AutocompleteItem
    {
        public string Text;
        public int ImageIndex = -1;
        public object Tag;
        string toolTipTitle;
        string toolTipText;
        string menuText;
        public AutocompleteMenu Parent { get; internal set; }
        

        public AutocompleteItem()
        {
        }

        public AutocompleteItem(string text)
        {
            Text = text;
        }

        public AutocompleteItem(string text, int imageIndex)
            : this(text)
        {
            this.ImageIndex = imageIndex;
        }

        public AutocompleteItem(string text, int imageIndex, string menuText)
            : this(text, imageIndex)
        {
            this.menuText = menuText;
        }

        public AutocompleteItem(string text, int imageIndex, string menuText, string toolTipTitle, string toolTipText)
            : this(text, imageIndex, menuText)
        {
            this.toolTipTitle = toolTipTitle;
            this.toolTipText = toolTipText;
        }

        /// <summary>
        /// Returns text for inserting into Textbox
        /// </summary>
        public virtual string GetTextForReplace()
        {
            return Text;
        }

        /// <summary>
        /// Compares fragment text with this item
        /// </summary>
        public virtual CompareResult Compare(string fragmentText)
        {
            if (Text.StartsWith(fragmentText, StringComparison.InvariantCultureIgnoreCase) &&
                   Text != fragmentText)
                return CompareResult.VisibleAndSelected;

            return CompareResult.Hidden;
        }

        /// <summary>
        /// Returns text for display into popup menu
        /// </summary>
        public override string ToString()
        {
            return menuText ?? Text;
        }

        /// <summary>
        /// This method is called after item inserted into text
        /// </summary>
        public virtual void OnSelected(AutocompleteMenu popupMenu, SelectedEventArgs e)
        {
            ;
        }

        /// <summary>
        /// Title for tooltip.
        /// </summary>
        /// <remarks>Return null for disable tooltip for this item</remarks>
        public virtual string ToolTipTitle
        {
            get { return toolTipTitle; }
            set { toolTipTitle = value; }
        }

        /// <summary>
        /// Tooltip text.
        /// </summary>
        /// <remarks>For display tooltip text, ToolTipTitle must be not null</remarks>
        public virtual string ToolTipText 
        {
            get{ return toolTipText; }
            set { toolTipText = value; }
        }

        /// <summary>
        /// Menu text. This text is displayed in the drop-down menu.
        /// </summary>
        public virtual string MenuText
        {
            get { return menuText; }
            set { menuText = value; }
        }

        /// <summary>
        /// Fore color of text of item
        /// </summary>
        public virtual Color ForeColor
        {
            get { return Color.Transparent; }
            set { throw new NotImplementedException("Override this property to change color"); }
        }

        /// <summary>
        /// Back color of item
        /// </summary>
        public virtual Color BackColor
        {
            get { return Color.Transparent; }
            set { throw new NotImplementedException("Override this property to change color"); }
        }
    }

    public enum CompareResult
    {
        /// <summary>
        /// Item do not appears
        /// </summary>
        Hidden,
        /// <summary>
        /// Item appears
        /// </summary>
        Visible,
        /// <summary>
        /// Item appears and will selected
        /// </summary>
        VisibleAndSelected
    }

    /// <summary>
    /// Autocomplete item for code snippets
    /// </summary>
    /// <remarks>Snippet can contain special char ^ for caret position.</remarks>
    public class SnippetAutocompleteItem : AutocompleteItem
    {
        public SnippetAutocompleteItem(string snippet)
        {
            Text = snippet.Replace("\r", "");
            ToolTipTitle = "Code snippet:";
            ToolTipText = Text;
        }

        public override string ToString()
        {
            return MenuText ?? Text.Replace("\n", " ").Replace("^", "");
        }

        public override string GetTextForReplace()
        {
            return Text;
        }

        public override void OnSelected(AutocompleteMenu popupMenu, SelectedEventArgs e)
        {
            e.Tb.BeginUpdate();
            e.Tb.Selection.BeginUpdate();
            //remember places
            var p1 = popupMenu.Fragment.Start;
            var p2 = e.Tb.Selection.Start;
            //do auto indent
            if (e.Tb.AutoIndent)
            {
                for (int iLine = p1.iLine + 1; iLine <= p2.iLine; iLine++)
                {
                    e.Tb.Selection.Start = new Place(0, iLine);
                    e.Tb.DoAutoIndent(iLine);
                }
            }
            e.Tb.Selection.Start = p1;
            //move caret position right and find char ^
            while (e.Tb.Selection.CharBeforeStart != '^')
                if (!e.Tb.Selection.GoRightThroughFolded())
                    break;
            //remove char ^
            e.Tb.Selection.GoLeft(true);
            e.Tb.InsertText("");
            //
            e.Tb.Selection.EndUpdate();
            e.Tb.EndUpdate();
        }

        /// <summary>
        /// Compares fragment text with this item
        /// </summary>
        public override CompareResult Compare(string fragmentText)
        {
            if (Text.StartsWith(fragmentText, StringComparison.InvariantCultureIgnoreCase) &&
                   Text != fragmentText)
                return CompareResult.Visible;

            return CompareResult.Hidden;
        }
    }

    /// <summary>
    /// This autocomplete item appears after dot
    /// </summary>
    public class MethodAutocompleteItem : AutocompleteItem
    {
        string firstPart;
        string lowercaseText;

        public MethodAutocompleteItem(string text)
            : base(text)
        {
            lowercaseText = Text.ToLower();
        }

        public override CompareResult Compare(string fragmentText)
        {
            int i = fragmentText.LastIndexOf('.');
            if (i < 0)
                return CompareResult.Hidden;
            string lastPart = fragmentText.Substring(i + 1);
            firstPart = fragmentText.Substring(0, i);

            if(lastPart=="") return CompareResult.Visible;
            if(Text.StartsWith(lastPart, StringComparison.InvariantCultureIgnoreCase))
                return CompareResult.VisibleAndSelected;
            if(lowercaseText.Contains(lastPart.ToLower()))
                return CompareResult.Visible;

            return CompareResult.Hidden;
        }

        public override string GetTextForReplace()
        {
            return firstPart + "." + Text;
        }
    }

    /// <summary>
    /// This Item does not check correspondence to current text fragment.
    /// SuggestItem is intended for dynamic menus.
    /// </summary>
    public class SuggestItem : AutocompleteItem
    {
        public SuggestItem(string text, int imageIndex):base(text, imageIndex)
        {   
        }

        public override CompareResult Compare(string fragmentText)
        {
            return CompareResult.Visible;
        }
    }
}
