//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  License: GNU Lesser General Public License (LGPLv3)
//
//  Email: pavel_torgashov@ukr.net.
//
//  Copyright (C) Pavel Torgashov, 2011-2015. 

// #define debug


// -------------------------------------------------------------------------------
// By default the FastColoredTextbox supports no more 16 styles at the same time.
// This restriction saves memory.
// However, you can to compile FCTB with 32 styles supporting.
// Uncomment following definition if you need 32 styles instead of 16:
//
// #define Styles32

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Design;
using System.Drawing.Drawing2D;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Forms.Design;
using Microsoft.Win32;
using Timer = System.Windows.Forms.Timer;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Fast colored textbox
    /// </summary>
    public partial class FastColoredTextBox : UserControl, ISupportInitialize
    {
        internal const int minLeftIndent = 8;
        private const int maxBracketSearchIterations = 1000;
        private const int maxLinesForFolding = 3000;
        private const int minLinesForAccuracy = 100000;
        private const int WM_IME_SETCONTEXT = 0x0281;
        private const int WM_HSCROLL = 0x114;
        private const int WM_VSCROLL = 0x115;
        private const int SB_ENDSCROLL = 0x8;

        public readonly List<LineInfo> LineInfos = new List<LineInfo>();
        private readonly Range selection;
        private readonly Timer timer = new Timer();
        private readonly Timer timer2 = new Timer();
        private readonly Timer timer3 = new Timer();
        private readonly List<VisualMarker> visibleMarkers = new List<VisualMarker>();
        public int TextHeight;
        public bool AllowInsertRemoveLines = true;
        private Brush backBrush;
        private BaseBookmarks bookmarks;
        private bool caretVisible;
        private Color changedLineColor;
        private int charHeight;
        private Color currentLineColor;
        private Cursor defaultCursor;
        private Range delayedTextChangedRange;
        private SyntaxDescriptor syntaxDescriptor;
        private int endFoldingLine = -1;
        private Color foldingIndicatorColor;
        protected Dictionary<int, int> foldingPairs = new Dictionary<int, int>();
        private bool handledChar;
        private bool highlightFoldingIndicator;
        private Hints hints;
        private Color indentBackColor;
        private bool isChanged;
        private bool isLineSelect;
        private bool isReplaceMode;
        private Language language;
        private Keys lastModifiers;
        private Point lastMouseCoord;
        private DateTime lastNavigatedDateTime;
        private Range leftBracketPosition;
        private Range leftBracketPosition2;
        private int leftPadding;
        private int lineInterval;
        private Color lineNumberColor;
        private uint lineNumberStartValue;
        private int lineSelectFrom;
        private TextSource lines;
        private IntPtr m_hImc;
        private int maxLineLength;
        private bool mouseIsDrag;
        private bool mouseIsDragDrop;
        private bool multiline;
        protected bool needRecalc;
        protected bool needRecalcWordWrap;
        private Point needRecalcWordWrapInterval;
        private bool needRecalcFoldingLines;
        private bool needRiseSelectionChangedDelayed;
        private bool needRiseTextChangedDelayed;
        private bool needRiseVisibleRangeChangedDelayed;
        private Color paddingBackColor;
        private int preferredLineWidth;
        private Range rightBracketPosition;
        private Range rightBracketPosition2;
        private bool scrollBars;
        private Color selectionColor;
        private Color serviceLinesColor;
        private bool showFoldingLines;
        private bool showLineNumbers;
        private FastColoredTextBox sourceTextBox;
        private int startFoldingLine = -1;
        private int updating;
        private Range updatingRange;
        private Range visibleRange;
        private bool wordWrap;
        private WordWrapMode wordWrapMode = WordWrapMode.WordWrapControlWidth;
        private int reservedCountOfLineNumberChars = 1;
        private int zoom = 100;
        private Size localAutoScrollMinSize;
 
        /// <summary>
        /// Constructor
        /// </summary>
        public FastColoredTextBox()
        {
            if(MonoUtility.IsLinux)
            {
                ImeMode |= System.Windows.Forms.ImeMode.Disable;
            } 
            else
            {
                //register type provider
                TypeDescriptionProvider prov = TypeDescriptor.GetProvider(GetType());
                object theProvider =
                    prov.GetType().GetField("Provider", BindingFlags.NonPublic | BindingFlags.Instance).GetValue(prov);
                if (theProvider.GetType() != typeof (FCTBDescriptionProvider))
                    TypeDescriptor.AddProvider(new FCTBDescriptionProvider(GetType()), GetType());
            }
            //drawing optimization
            SetStyle(ControlStyles.AllPaintingInWmPaint, true);
            SetStyle(ControlStyles.UserPaint, true);
            SetStyle(ControlStyles.OptimizedDoubleBuffer, true);
            SetStyle(ControlStyles.ResizeRedraw, true);
            //create one line
            InitTextSource(CreateTextSource());
            if (lines.Count == 0)
                lines.InsertLine(0, lines.CreateLine());
            //append monospace font
            //Font = new Font("Consolas", 9.75f, FontStyle.Regular, GraphicsUnit.Point);
            Font = new Font(FontFamily.GenericMonospace, 9.75f);
            selection = new Range(this) {Start = new Place(0, 0)};
            //default settings
            Cursor = Cursors.IBeam;
            BackColor = Color.White;
            LineNumberColor = Color.Teal;
            IndentBackColor = Color.WhiteSmoke;
            ServiceLinesColor = Color.Silver;
            FoldingIndicatorColor = Color.Green;
            CurrentLineColor = Color.Transparent;
            ChangedLineColor = Color.Transparent;
            HighlightFoldingIndicator = true;
            ShowLineNumbers = true;
            TabLength = 4;
            FoldedBlockStyle = new FoldedBlockStyle(Brushes.Gray, null, FontStyle.Regular);
            SelectionColor = Color.Blue;
            BracketsStyle = new MarkerStyle(new SolidBrush(Color.FromArgb(80, Color.Lime)));
            BracketsStyle2 = new MarkerStyle(new SolidBrush(Color.FromArgb(60, Color.Red)));
            DelayedEventsInterval = 100;
            DelayedTextChangedInterval = 100;
            AllowSeveralTextStyleDrawing = false;
            LeftBracket = '\x0';
            RightBracket = '\x0';
            LeftBracket2 = '\x0';
            RightBracket2 = '\x0';
            SyntaxHighlighter = new SyntaxHighlighter();
            language = Language.Custom;
            PreferredLineWidth = 0;
            needRecalc = true;
            lastNavigatedDateTime = DateTime.Now;
            AutoIndent = true;
            AutoIndentExistingLines = true;
            CommentPrefix = "//";
            lineNumberStartValue = 1;
            multiline = true;
            scrollBars = true;
            AcceptsTab = true;
            AcceptsReturn = true;
            caretVisible = true;
            CaretColor = Color.Black;
            WideCaret = false;
            Paddings = new Padding(0, 0, 0, 0);
            PaddingBackColor = Color.Transparent;
            DisabledColor = Color.FromArgb(100, 180, 180, 180);
            needRecalcFoldingLines = true;
            AllowDrop = true;
            FindEndOfFoldingBlockStrategy = FindEndOfFoldingBlockStrategy.Strategy1;
            VirtualSpace = false;
            bookmarks = new Bookmarks(this);
            BookmarkColor = Color.PowderBlue;
            ToolTip = new ToolTip();
            timer3.Interval = 500;
            hints = new Hints(this);
            SelectionHighlightingForLineBreaksEnabled = true;
            textAreaBorder = TextAreaBorderType.None;
            textAreaBorderColor = Color.Black;
            macrosManager = new MacrosManager(this);
            HotkeysMapping = new HotkeysMapping();
            HotkeysMapping.InitDefault();
            WordWrapAutoIndent = true;
            FoldedBlocks = new Dictionary<int, int>();
            AutoCompleteBrackets = false;
            AutoIndentCharsPatterns = @"^\s*[\w\.]+\s*(?<range>=)\s*(?<range>[^;]+);";
            AutoIndentChars = true;
            CaretBlinking = true;
            ServiceColors = new ServiceColors();
            //
            base.AutoScroll = true;
            timer.Tick += timer_Tick;
            timer2.Tick += timer2_Tick;
            timer3.Tick += timer3_Tick;
            middleClickScrollingTimer.Tick += middleClickScrollingTimer_Tick;
        }

        private char[] autoCompleteBracketsList = { '(', ')', '{', '}', '[', ']', '"', '"', '\'', '\'' };

        public char[] AutoCompleteBracketsList
        {
            get { return autoCompleteBracketsList; }
            set { autoCompleteBracketsList = value; }
        }

        /// <summary>
        /// AutoComplete brackets
        /// </summary>
        [DefaultValue(false)]
        [Description("AutoComplete brackets.")]
        public bool AutoCompleteBrackets { get; set; }

        /// <summary>
        /// Colors of some service visual markers
        /// </summary>
		  [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public ServiceColors ServiceColors { get; set; }

        /// <summary>
        /// Contains UniqueId of start lines of folded blocks
        /// </summary>
        /// <remarks>This dictionary remembers folding state of blocks.
        /// It is needed to restore child folding after user collapsed/expanded top-level folding block.</remarks>
        [Browsable(false)]
        public Dictionary<int, int> FoldedBlocks { get; private set; }

        /// <summary>
        /// Strategy of search of brackets to highlighting
        /// </summary>
        [DefaultValue(typeof(BracketsHighlightStrategy), "Strategy1")]
        [Description("Strategy of search of brackets to highlighting.")]
        public BracketsHighlightStrategy BracketsHighlightStrategy { get; set; }
        
        /// <summary>
        /// Automatically shifts secondary wordwrap lines on the shift amount of the first line
        /// </summary>
        [DefaultValue(true)]
        [Description("Automatically shifts secondary wordwrap lines on the shift amount of the first line.")]
        public bool WordWrapAutoIndent { get; set; }

        /// <summary>
        /// Indent of secondary wordwrap lines (in chars)
        /// </summary>
        [DefaultValue(0)]
        [Description("Indent of secondary wordwrap lines (in chars).")]
        public int WordWrapIndent { get; set; }

        MacrosManager macrosManager;
        /// <summary>
        /// MacrosManager records, stores and executes the macroses
        /// </summary>
        [Browsable(false)]
        public MacrosManager MacrosManager { get { return macrosManager; } }

        /// <summary>
        /// Allows drag and drop
        /// </summary>
        [DefaultValue(true)]
        [Description("Allows drag and drop")]
        public override bool AllowDrop
        {
            get { return base.AllowDrop; }
            set { base.AllowDrop = value; }
        }

        /// <summary>
        /// Collection of Hints.
        /// This is temporary buffer for currently displayed hints.
        /// </summary>
        /// <remarks>You can asynchronously add, remove and clear hints. Appropriate hints will be shown or hidden from the screen.</remarks>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden),
         EditorBrowsable(EditorBrowsableState.Never)]
        public Hints Hints
        {
            get { return hints; }
            set { hints = value; }
        }

        /// <summary>
        /// Delay (ms) of ToolTip
        /// </summary>
        [Browsable(true)]
        [DefaultValue(500)]
        [Description("Delay(ms) of ToolTip.")]
        public int ToolTipDelay
        {
            get { return timer3.Interval; }
            set { timer3.Interval = value; }
        }

        /// <summary>
        /// ToolTip component
        /// </summary>
        [Browsable(true)]
        [Description("ToolTip component.")]
        public ToolTip ToolTip { get; set; }

        /// <summary>
        /// Color of bookmarks
        /// </summary>
        [Browsable(true)]
        [DefaultValue(typeof (Color), "PowderBlue")]
        [Description("Color of bookmarks.")]
        public Color BookmarkColor { get; set; }

        /// <summary>
        /// Bookmarks
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden),
         EditorBrowsable(EditorBrowsableState.Never)]
        public BaseBookmarks Bookmarks
        {
            get { return bookmarks; }
            set { bookmarks = value; }
        }

        /// <summary>
        /// Enables virtual spaces
        /// </summary>
        [DefaultValue(false)]
        [Description("Enables virtual spaces.")]
        public bool VirtualSpace { get; set; }

        /// <summary>
        /// Strategy of search of end of folding block
        /// </summary>
        [DefaultValue(FindEndOfFoldingBlockStrategy.Strategy1)]
        [Description("Strategy of search of end of folding block.")]
        public FindEndOfFoldingBlockStrategy FindEndOfFoldingBlockStrategy { get; set; }

        /// <summary>
        /// Indicates if tab characters are accepted as input
        /// </summary>
        [DefaultValue(true)]
        [Description("Indicates if tab characters are accepted as input.")]
        public bool AcceptsTab { get; set; }

        /// <summary>
        /// Indicates if return characters are accepted as input
        /// </summary>
        [DefaultValue(true)]
        [Description("Indicates if return characters are accepted as input.")]
        public bool AcceptsReturn { get; set; }

        /// <summary>
        /// Shows or hides the caret
        /// </summary>
        [DefaultValue(true)]
        [Description("Shows or hides the caret")]
        public bool CaretVisible
        {
            get { return caretVisible; }
            set
            {
                caretVisible = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Enables caret blinking
        /// </summary>
        [DefaultValue(true)]
        [Description("Enables caret blinking")]
        public bool CaretBlinking { get; set; }


        Color textAreaBorderColor;

        /// <summary>
        /// Color of border of text area
        /// </summary>
        [DefaultValue(typeof(Color), "Black")]
        [Description("Color of border of text area")]
        public Color TextAreaBorderColor
        {
            get { return textAreaBorderColor; }
            set
            {
                textAreaBorderColor = value;
                Invalidate();
            }
        }

        TextAreaBorderType textAreaBorder;
        /// <summary>
        /// Type of border of text area
        /// </summary>
        [DefaultValue(typeof(TextAreaBorderType), "None")]
        [Description("Type of border of text area")]
        public TextAreaBorderType TextAreaBorder
        {
            get { return textAreaBorder; }
            set
            {
                textAreaBorder = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Background color for current line
        /// </summary>
        [DefaultValue(typeof (Color), "Transparent")]
        [Description("Background color for current line. Set to Color.Transparent to hide current line highlighting")]
        public Color CurrentLineColor
        {
            get { return currentLineColor; }
            set
            {
                currentLineColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Background color for highlighting of changed lines
        /// </summary>
        [DefaultValue(typeof (Color), "Transparent")]
        [Description("Background color for highlighting of changed lines. Set to Color.Transparent to hide changed line highlighting")]
        public Color ChangedLineColor
        {
            get { return changedLineColor; }
            set
            {
                changedLineColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Fore color (default style color)
        /// </summary>
        public override Color ForeColor
        {
            get { return base.ForeColor; }
            set
            {
                base.ForeColor = value;
                lines.InitDefaultStyle();
                Invalidate();
            }
        }

        /// <summary>
        /// Height of char in pixels (includes LineInterval)
        /// </summary>
        [Browsable(false)]
        public int CharHeight
        {
            get { return charHeight; }
            set
            {
                charHeight = value;
                NeedRecalc();
                OnCharSizeChanged();
            }
        }

        /// <summary>
        /// Interval between lines (in pixels)
        /// </summary>
        [Description("Interval between lines in pixels")]
        [DefaultValue(0)]
        public int LineInterval
        {
            get { return lineInterval; }
            set
            {
                lineInterval = value;
                SetFont(Font);
                Invalidate();
            }
        }

        /// <summary>
        /// Width of char in pixels
        /// </summary>
        [Browsable(false)]
        public int CharWidth { get; set; }

        /// <summary>
        /// Spaces count for tab
        /// </summary>
        [DefaultValue(4)]
        [Description("Spaces count for tab")]
        public int TabLength { get; set; }

        /// <summary>
        /// Text was changed
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool IsChanged
        {
            get { return isChanged; }
            set
            {
                if (!value)
                    //clear line's IsChanged property
                    lines.ClearIsChanged();

                isChanged = value;
            }
        }

        /// <summary>
        /// Text version
        /// </summary>
        /// <remarks>This counter is incremented each time changes the text</remarks>
        [Browsable(false)]
        public int TextVersion { get; private set; }

        /// <summary>
        /// Read only
        /// </summary>
        [DefaultValue(false)]
        public bool ReadOnly { get; set; }

        /// <summary>
        /// Shows line numbers.
        /// </summary>
        [DefaultValue(true)]
        [Description("Shows line numbers.")]
        public bool ShowLineNumbers
        {
            get { return showLineNumbers; }
            set
            {
                showLineNumbers = value;
                NeedRecalc();
                Invalidate();
            }
        }

        /// <summary>
        /// Shows vertical lines between folding start line and folding end line.
        /// </summary>
        [DefaultValue(false)]
        [Description("Shows vertical lines between folding start line and folding end line.")]
        public bool ShowFoldingLines
        {
            get { return showFoldingLines; }
            set
            {
                showFoldingLines = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Rectangle where located text
        /// </summary>
        [Browsable(false)]
        public Rectangle TextAreaRect
        {
            get 
            {
                int rightPaddingStartX = LeftIndent + maxLineLength * CharWidth + Paddings.Left + 1;
                rightPaddingStartX = Math.Max(ClientSize.Width - Paddings.Right, rightPaddingStartX);
                int bottomPaddingStartY = TextHeight + Paddings.Top;
                bottomPaddingStartY = Math.Max(ClientSize.Height - Paddings.Bottom, bottomPaddingStartY);
                var top = Math.Max(0, Paddings.Top - 1) - VerticalScroll.Value;
                var left = LeftIndent - HorizontalScroll.Value - 2 + Math.Max(0, Paddings.Left - 1);
                var rect = Rectangle.FromLTRB(left, top, rightPaddingStartX - HorizontalScroll.Value, bottomPaddingStartY - VerticalScroll.Value);
                return rect;
            }
        }

        /// <summary>
        /// Color of line numbers.
        /// </summary>
        [DefaultValue(typeof (Color), "Teal")]
        [Description("Color of line numbers.")]
        public Color LineNumberColor
        {
            get { return lineNumberColor; }
            set
            {
                lineNumberColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Start value of first line number.
        /// </summary>
        [DefaultValue(typeof (uint), "1")]
        [Description("Start value of first line number.")]
        public uint LineNumberStartValue
        {
            get { return lineNumberStartValue; }
            set
            {
                lineNumberStartValue = value;
                needRecalc = true;
                Invalidate();
            }
        }

        /// <summary>
        /// Background color of indent area
        /// </summary>
        [DefaultValue(typeof(Color), "WhiteSmoke")]
        [Description("Background color of indent area")]
        public Color IndentBackColor
        {
            get { return indentBackColor; }
            set
            {
                indentBackColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Background color of padding area
        /// </summary>
        [DefaultValue(typeof (Color), "Transparent")]
        [Description("Background color of padding area")]
        public Color PaddingBackColor
        {
            get { return paddingBackColor; }
            set
            {
                paddingBackColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Color of disabled component
        /// </summary>
        [DefaultValue(typeof (Color), "100;180;180;180")]
        [Description("Color of disabled component")]
        public Color DisabledColor { get; set; }

        /// <summary>
        /// Color of caret
        /// </summary>
        [DefaultValue(typeof (Color), "Black")]
        [Description("Color of caret.")]
        public Color CaretColor { get; set; }

        /// <summary>
        /// Wide caret
        /// </summary>
        [DefaultValue(false)]
        [Description("Wide caret.")]
        public bool WideCaret { get; set; }

        /// <summary>
        /// Color of service lines (folding lines, borders of blocks etc.)
        /// </summary>
        [DefaultValue(typeof (Color), "Silver")]
        [Description("Color of service lines (folding lines, borders of blocks etc.)")]
        public Color ServiceLinesColor
        {
            get { return serviceLinesColor; }
            set
            {
                serviceLinesColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Padings of text area
        /// </summary>
        [Browsable(true)]
        [Description("Paddings of text area.")]
        public Padding Paddings { get; set; }

        /// <summary>
        /// --Do not use this property--
        /// </summary>
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden),
         EditorBrowsable(EditorBrowsableState.Never)]
        public new Padding Padding
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        //hide RTL
        [Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden),
         EditorBrowsable(EditorBrowsableState.Never)]
        public new bool RightToLeft
        {
            get { throw new NotImplementedException(); }
            set { throw new NotImplementedException(); }
        }

        /// <summary>
        /// Color of folding area indicator
        /// </summary>
        [DefaultValue(typeof (Color), "Green")]
        [Description("Color of folding area indicator.")]
        public Color FoldingIndicatorColor
        {
            get { return foldingIndicatorColor; }
            set
            {
                foldingIndicatorColor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Enables folding indicator (left vertical line between folding bounds)
        /// </summary>
        [DefaultValue(true)]
        [Description("Enables folding indicator (left vertical line between folding bounds)")]
        public bool HighlightFoldingIndicator
        {
            get { return highlightFoldingIndicator; }
            set
            {
                highlightFoldingIndicator = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Left distance to text beginning
        /// </summary>
        [Browsable(false)]
        [Description("Left distance to text beginning.")]
        public int LeftIndent { get; private set; }

        /// <summary>
        /// Left padding in pixels
        /// </summary>
        [DefaultValue(0)]
        [Description("Width of left service area (in pixels)")]
        public int LeftPadding
        {
            get { return leftPadding; }
            set
            {
                leftPadding = value;
                Invalidate();
            }
        }

        /// <summary>
        /// This property draws vertical line after defined char position.
        /// Set to 0 for disable drawing of vertical line.
        /// </summary>
        [DefaultValue(0)]
        [Description("This property draws vertical line after defined char position. Set to 0 for disable drawing of vertical line.")]
        public int PreferredLineWidth
        {
            get { return preferredLineWidth; }
            set
            {
                preferredLineWidth = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Styles
        /// </summary>
        [Browsable(false)]
        public Style[] Styles
        {
            get { return lines.Styles; }
        }

		/// <summary>
		/// Hotkeys. Do not use this property in your code, use HotkeysMapping property.
		/// </summary>
		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public string Hotkeys { 
            get { return HotkeysMapping.ToString(); }
            set { HotkeysMapping = HotkeysMapping.Parse(value); }
        }

        /// <summary>
        /// Hotkeys mapping
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public HotkeysMapping HotkeysMapping{ get; set;}

        /// <summary>
        /// Default text style
        /// This style is using when no one other TextStyle is not defined in Char.style
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public TextStyle DefaultStyle
        {
            get { return lines.DefaultStyle; }
            set { lines.DefaultStyle = value; }
        }

        /// <summary>
        /// Style for rendering Selection area
        /// </summary>
        [Browsable(false)]
        public SelectionStyle SelectionStyle { get; set; }

        /// <summary>
        /// Style for folded block rendering
        /// </summary>
        [Browsable(false)]
        public TextStyle FoldedBlockStyle { get; set; }

        /// <summary>
        /// Style for brackets highlighting
        /// </summary>
        [Browsable(false)]
        public MarkerStyle BracketsStyle { get; set; }

        /// <summary>
        /// Style for alternative brackets highlighting
        /// </summary>
        [Browsable(false)]
        public MarkerStyle BracketsStyle2 { get; set; }

        /// <summary>
        /// Opening bracket for brackets highlighting.
        /// Set to '\x0' for disable brackets highlighting.
        /// </summary>
        [DefaultValue('\x0')]
        [Description("Opening bracket for brackets highlighting. Set to '\\x0' for disable brackets highlighting.")]
        public char LeftBracket { get; set; }

        /// <summary>
        /// Closing bracket for brackets highlighting.
        /// Set to '\x0' for disable brackets highlighting.
        /// </summary>
        [DefaultValue('\x0')]
        [Description("Closing bracket for brackets highlighting. Set to '\\x0' for disable brackets highlighting.")]
        public char RightBracket { get; set; }

        /// <summary>
        /// Alternative opening bracket for brackets highlighting.
        /// Set to '\x0' for disable brackets highlighting.
        /// </summary>
        [DefaultValue('\x0')]
        [Description("Alternative opening bracket for brackets highlighting. Set to '\\x0' for disable brackets highlighting.")]
        public char LeftBracket2 { get; set; }

        /// <summary>
        /// Alternative closing bracket for brackets highlighting.
        /// Set to '\x0' for disable brackets highlighting.
        /// </summary>
        [DefaultValue('\x0')]
        [Description("Alternative closing bracket for brackets highlighting. Set to '\\x0' for disable brackets highlighting.")]
        public char RightBracket2 { get; set; }

        /// <summary>
        /// Comment line prefix.
        /// </summary>
        [DefaultValue("//")]
        [Description("Comment line prefix.")]
        public string CommentPrefix { get; set; }

        /// <summary>
        /// This property specifies which part of the text will be highlighted as you type (by built-in highlighter).
        /// </summary>
        /// <remarks>When a user enters text, a component refreshes highlighting (because the text was changed).
        /// This property specifies exactly which section of the text will be re-highlighted.
        /// This can be useful to highlight multi-line comments, for example.</remarks>
        [DefaultValue(typeof (HighlightingRangeType), "ChangedRange")]
        [Description("This property specifies which part of the text will be highlighted as you type.")]
        public HighlightingRangeType HighlightingRangeType { get; set; }

        /// <summary>
        /// Is keyboard in replace mode (wide caret) ?
        /// </summary>
        [Browsable(false)]
        public bool IsReplaceMode
        {
            get
            {
                return isReplaceMode && 
                       Selection.IsEmpty &&
                       (!Selection.ColumnSelectionMode) &&
                       Selection.Start.iChar < lines[Selection.Start.iLine].Count;
            }
            set { isReplaceMode = value; }
        }

        /// <summary>
        /// Allows text rendering several styles same time.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(false)]
        [Description("Allows text rendering several styles same time.")]
        public bool AllowSeveralTextStyleDrawing { get; set; }

        /// <summary>
        /// Allows to record macros.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(true)]
        [Description("Allows to record macros.")]
        public bool AllowMacroRecording 
        { 
            get { return macrosManager.AllowMacroRecordingByUser; }
            set { macrosManager.AllowMacroRecordingByUser = value; }
        }

        /// <summary>
        /// Allows AutoIndent. Inserts spaces before new line.
        /// </summary>
        [DefaultValue(true)]
        [Description("Allows auto indent. Inserts spaces before line chars.")]
        public bool AutoIndent { get; set; }

        /// <summary>
        /// Does autoindenting in existing lines. It works only if AutoIndent is True.
        /// </summary>
        [DefaultValue(true)]
        [Description("Does autoindenting in existing lines. It works only if AutoIndent is True.")]
        public bool AutoIndentExistingLines { get; set; }

        /// <summary>
        /// Minimal delay(ms) for delayed events (except TextChangedDelayed).
        /// </summary>
        [Browsable(true)]
        [DefaultValue(100)]
        [Description("Minimal delay(ms) for delayed events (except TextChangedDelayed).")]
        public int DelayedEventsInterval
        {
            get { return timer.Interval; }
            set { timer.Interval = value; }
        }

        /// <summary>
        /// Minimal delay(ms) for TextChangedDelayed event.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(100)]
        [Description("Minimal delay(ms) for TextChangedDelayed event.")]
        public int DelayedTextChangedInterval
        {
            get { return timer2.Interval; }
            set { timer2.Interval = value; }
        }

        /// <summary>
        /// Language for highlighting by built-in highlighter.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(typeof (Language), "Custom")]
        [Description("Language for highlighting by built-in highlighter.")]
        public Language Language
        {
            get { return language; }
            set
            {
                language = value;
                if (SyntaxHighlighter != null)
                    SyntaxHighlighter.InitStyleSchema(language);
                Invalidate();
            }
        }

        /// <summary>
        /// Syntax Highlighter
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public SyntaxHighlighter SyntaxHighlighter { get; set; }

        /// <summary>
        /// XML file with description of syntax highlighting.
        /// This property works only with Language == Language.Custom.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(null)]
        //[Editor(typeof (FileNameEditor), typeof (UITypeEditor))]
        [Description(
            "XML file with description of syntax highlighting. This property works only with Language == Language.Custom."
            )]
        public SyntaxDescriptor SyntaxDescriptor
        {
            get { return syntaxDescriptor; }
            set
            {
                syntaxDescriptor = value;
                Invalidate();
            }
        }

        /// <summary>
        /// Position of left highlighted bracket.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Range LeftBracketPosition
        {
            get { return leftBracketPosition; }
        }

        /// <summary>
        /// Position of right highlighted bracket.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Range RightBracketPosition
        {
            get { return rightBracketPosition; }
        }

        /// <summary>
        /// Position of left highlighted alternative bracket.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Range LeftBracketPosition2
        {
            get { return leftBracketPosition2; }
        }

        /// <summary>
        /// Position of right highlighted alternative bracket.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public Range RightBracketPosition2
        {
            get { return rightBracketPosition2; }
        }

        /// <summary>
        /// Start line index of current highlighted folding area. Return -1 if start of area is not found.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int StartFoldingLine
        {
            get { return startFoldingLine; }
        }

        /// <summary>
        /// End line index of current highlighted folding area. Return -1 if end of area is not found.
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int EndFoldingLine
        {
            get { return endFoldingLine; }
        }

        /// <summary>
        /// TextSource
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public TextSource TextSource
        {
            get { return lines; }
            set { InitTextSource(value); }
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public bool HasSourceTextBox
        {
            get { return SourceTextBox != null; }
        }

        /// <summary>
        /// The source of the text.
        /// Allows to get text from other FastColoredTextBox.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(null)]
        [Description("Allows to get text from other FastColoredTextBox.")]
        //[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public FastColoredTextBox SourceTextBox
        {
            get { return sourceTextBox; }
            set
            {
                if (value == sourceTextBox)
                    return;

                sourceTextBox = value;

                if (sourceTextBox == null)
                {
                    InitTextSource(CreateTextSource());
                    lines.InsertLine(0, TextSource.CreateLine());
                    IsChanged = false;
                }
                else
                {
                    InitTextSource(SourceTextBox.TextSource);
                    isChanged = false;
                }
                Invalidate();
            }
        }

        /// <summary>
        /// Returns current visible range of text
        /// </summary>
        [Browsable(false)]
        public Range VisibleRange
        {
            get
            {
                if (visibleRange != null)
                    return visibleRange;
                return GetRange(
                    PointToPlace(new Point(LeftIndent, 0)),
                    PointToPlace(new Point(ClientSize.Width, ClientSize.Height))
                    );
            }
        }

        /// <summary>
        /// Current selection range
        /// </summary>
        [Browsable(false)]
        public Range Selection
        {
            get { return selection; }
            set
            {
                if (value == selection)
                    return;

                selection.BeginUpdate();
                selection.Start = value.Start;
                selection.End = value.End;
                selection.EndUpdate();
                Invalidate();
            }
        }

        /// <summary>
        /// Background color.
        /// It is used if BackBrush is null.
        /// </summary>
        [DefaultValue(typeof (Color), "White")]
        [Description("Background color.")]
        public override Color BackColor
        {
            get { return base.BackColor; }
            set { base.BackColor = value; }
        }

        /// <summary>
        /// Background brush.
        /// If Null then BackColor is used.
        /// </summary>
        [Browsable(false)]
        public Brush BackBrush
        {
            get { return backBrush; }
            set
            {
                backBrush = value;
                Invalidate();
            }
        }

        [Browsable(true)]
        [DefaultValue(true)]
        [Description("Scollbars visibility.")]
        public bool ShowScrollBars
        {
            get { return scrollBars; }
            set
            {
                if (value == scrollBars) return;
                scrollBars = value;
                needRecalc = true;
                Invalidate();
            }
        }

        /// <summary>
        /// Multiline
        /// </summary>
        [Browsable(true)]
        [DefaultValue(true)]
        [Description("Multiline mode.")]
        public bool Multiline
        {
            get { return multiline; }
            set
            {
                if (multiline == value) return;
                multiline = value;
                needRecalc = true;
                if (multiline)
                {
                    base.AutoScroll = true;
                    ShowScrollBars = true;
                }
                else
                {
                    base.AutoScroll = false;
                    ShowScrollBars = false;
                    if (lines.Count > 1)
                        lines.RemoveLine(1, lines.Count - 1);
                    lines.Manager.ClearHistory();
                }
                Invalidate();
            }
        }

        /// <summary>
        /// WordWrap.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(false)]
        [Description("WordWrap.")]
        public bool WordWrap
        {
            get { return wordWrap; }
            set
            {
                if (wordWrap == value) return;
                wordWrap = value;
                if (wordWrap)
                    Selection.ColumnSelectionMode = false;
                NeedRecalc(false, true);
                //RecalcWordWrap(0, LinesCount - 1);
                Invalidate();
            }
        }

        /// <summary>
        /// WordWrap mode.
        /// </summary>
        [Browsable(true)]
        [DefaultValue(typeof (WordWrapMode), "WordWrapControlWidth")]
        [Description("WordWrap mode.")]
        public WordWrapMode WordWrapMode
        {
            get { return wordWrapMode; }
            set
            {
                if (wordWrapMode == value) return;
                wordWrapMode = value;
                NeedRecalc(false, true);
                //RecalcWordWrap(0, LinesCount - 1);
                Invalidate();
            }
        }

        private bool selectionHighlightingForLineBreaksEnabled;
        /// <summary>
        /// If <c>true</c> then line breaks included into the selection will be selected too.
        /// Then line breaks will be shown as selected blank character.
        /// </summary>
        [DefaultValue(true)]
        [Description("If enabled then line ends included into the selection will be selected too. " +
            "Then line ends will be shown as selected blank character.")]
        public bool SelectionHighlightingForLineBreaksEnabled
        {
            get { return selectionHighlightingForLineBreaksEnabled; }
            set
            {
                selectionHighlightingForLineBreaksEnabled = value;
                Invalidate();
            }
        }


        [Browsable(false)]
        public FindForm findForm { get; private set; }

        [Browsable(false)]
        public ReplaceForm replaceForm { get; private set; }

        /// <summary>
        /// Do not change this property
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public override bool AutoScroll
        {
            get { return base.AutoScroll; }
            set { ; }
        }

        /// <summary>
        /// Count of lines
        /// </summary>
        [Browsable(false)]
        public int LinesCount
        {
            get { return lines.Count; }
        }

        /// <summary>
        /// Gets or sets char and styleId for given place
        /// This property does not fire OnTextChanged event
        /// </summary>
        public Char this[Place place]
        {
            get { return lines[place.iLine][place.iChar]; }
            set { lines[place.iLine][place.iChar] = value; }
        }

        /// <summary>
        /// Gets Line
        /// </summary>
        public Line this[int iLine]
        {
            get { return lines[iLine]; }
        }

        /// <summary>
        /// Text of control
        /// </summary>
        [Browsable(true)]
        [Localizable(true)]
        [Editor(
            "System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a"
            , typeof (UITypeEditor))]
        [SettingsBindable(true)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
        [Description("Text of the control.")]
        [Bindable(true)]
        public override string Text
        {
            get
            {
                if (LinesCount == 0)
                    return "";
                var sel = new Range(this);
                sel.SelectAll();
                return sel.Text;
            }

            set
            {
                if (value == Text && value != "")
                    return;

                SetAsCurrentTB();

                Selection.ColumnSelectionMode = false;

                Selection.BeginUpdate();
                try
                {
                    Selection.SelectAll();
                    InsertText(value);
                    GoHome();
                }
                finally
                {
                    Selection.EndUpdate();
                }
            }
        }

        /// <summary>
        /// Text lines
        /// </summary>
        [Browsable(false)]
        public IList<string> Lines
        {
            get { return lines.GetLines(); }
        }

        /// <summary>
        /// Gets colored text as HTML
        /// </summary>
        /// <remarks>For more flexibility you can use ExportToHTML class also</remarks>
        [Browsable(false)]
        public string Html
        {
            get
            {
                var exporter = new ExportToHTML();
                exporter.UseNbsp = false;
                exporter.UseStyleTag = false;
                exporter.UseBr = false;
                return "<pre>" + exporter.GetHtml(this) + "</pre>";
            }
        }

        /// <summary>
        /// Gets colored text as RTF
        /// </summary>
        /// <remarks>For more flexibility you can use ExportToRTF class also</remarks>
        [Browsable(false)]
        public string Rtf
        {
            get
            {
                var exporter = new ExportToRTF();
                return exporter.GetRtf(this);
            }
        }

        /// <summary>
        /// Text of current selection
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public string SelectedText
        {
            get { return Selection.Text; }
            set { InsertText(value); }
        }

        /// <summary>
        /// Start position of selection
        /// </summary>
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public int SelectionStart
        {
            get { return Math.Min(PlaceToPosition(Selection.Start), PlaceToPosition(Selection.End)); }
            set { Selection.Start = PositionToPlace(value); }
        }

        /// <summary>
        /// Length of selected text
        /// </summary>
        [Browsable(false)]
        [DefaultValue(0)]
        public int SelectionLength
        {
            get { return Math.Abs(PlaceToPosition(Selection.Start) - PlaceToPosition(Selection.End)); }
            set
            {
                Selection.End = PositionToPlace(SelectionStart + value);
            }
        }

        /// <summary>
        /// Font
        /// </summary>
        /// <remarks>Use only monospaced font</remarks>
        [DefaultValue(typeof (Font), "Courier New, 9.75")]
        public override Font Font
        {
            get { return BaseFont; }
            set {
                originalFont = (Font)value.Clone();
                SetFont(value);
            }
        }


        Font baseFont;
        /// <summary>
        /// Font
        /// </summary>
        /// <remarks>Use only monospaced font</remarks>
        [DefaultValue(typeof(Font), "Courier New, 9.75")]
        private Font BaseFont
        {
            get { return baseFont; }
            set
            {
                baseFont = value;
            }
        }

        private void SetFont(Font newFont)
        {
            BaseFont = newFont;
            //check monospace font
            /*SizeF sizeM = GetCharSize(BaseFont, 'M');
            SizeF sizeDot = GetCharSize(BaseFont, '.');
            if (sizeM != sizeDot)
                BaseFont = new Font("Courier New", BaseFont.SizeInPoints, FontStyle.Regular, GraphicsUnit.Point);*/
            //clac size
            SizeF size = GetCharSize(BaseFont, 'M');
            CharWidth = (int) Math.Round(size.Width*1f /*0.85*/) - 1 /*0*/;
            CharHeight = lineInterval + (int) Math.Round(size.Height*1f /*0.9*/) - 1 /*0*/;
            //
            //if (wordWrap)
            //    RecalcWordWrap(0, Lines.Count - 1);
            NeedRecalc(false, wordWrap);
            //
            Invalidate();
        }

        public new Size AutoScrollMinSize
        {
            set
            {
                if (scrollBars)
                {
                    if (!base.AutoScroll)
                        base.AutoScroll = true;
                    Size newSize = value;
                    if (WordWrap && WordWrapMode != FastColoredTextBoxNS.WordWrapMode.Custom)
                    {
                        int maxWidth = GetMaxLineWordWrapedWidth();
                        newSize = new Size(Math.Min(newSize.Width, maxWidth), newSize.Height);
                    }
                    base.AutoScrollMinSize = newSize;
                }
                else
                {
                    if (base.AutoScroll)
                        base.AutoScroll = false;
                    base.AutoScrollMinSize = new Size(0, 0);
                    VerticalScroll.Visible = false;
                    HorizontalScroll.Visible = false;
                    VerticalScroll.Maximum = Math.Max(0, value.Height - ClientSize.Height);
                    HorizontalScroll.Maximum = Math.Max(0, value.Width - ClientSize.Width);
                    localAutoScrollMinSize = value;
                }
            }

            get
            {
                if (scrollBars)
                    return base.AutoScrollMinSize;
                else
                    //return new Size(HorizontalScroll.Maximum, VerticalScroll.Maximum);
                    return localAutoScrollMinSize;
            }
        }

        /// <summary>
        /// Indicates that IME is allowed (for CJK language entering)
        /// </summary>
        [Browsable(false)]
        public bool ImeAllowed
        {
            get
            {
                return ImeMode != ImeMode.Disable &&
                       ImeMode != ImeMode.Off &&
                       ImeMode != ImeMode.NoControl;
            }
        }

        /// <summary>
        /// Is undo enabled?
        /// </summary>
        [Browsable(false)]
        public bool UndoEnabled
        {
            get { return lines.Manager.UndoEnabled; }
        }

        /// <summary>
        /// Is redo enabled?
        /// </summary>
        [Browsable(false)]
        public bool RedoEnabled
        {
            get { return lines.Manager.RedoEnabled; }
        }

        private int LeftIndentLine
        {
            get { return LeftIndent - minLeftIndent/2 - 3; }
        }

        /// <summary>
        /// Range of all text
        /// </summary>
        [Browsable(false)]
        public Range Range
        {
            get { return new Range(this, new Place(0, 0), new Place(lines[lines.Count - 1].Count, lines.Count - 1)); }
        }

        /// <summary>
        /// Color of selected area
        /// </summary>
        [DefaultValue(typeof (Color), "Blue")]
        [Description("Color of selected area.")]
        public virtual Color SelectionColor
        {
            get { return selectionColor; }
            set
            {
                selectionColor = value;
                if (selectionColor.A == 255)
                    selectionColor = Color.FromArgb(60, selectionColor);
                SelectionStyle = new SelectionStyle(new SolidBrush(selectionColor));
                Invalidate();
            }
        }

        public override Cursor Cursor
        {
            get { return base.Cursor; }
            set
            {
                defaultCursor = value;
                base.Cursor = value;
            }
        }

        /// <summary>
        /// Reserved space for line number characters.
        /// If smaller than needed (e. g. line count >= 10 and this value set to 1) this value will have no impact.
        /// If you want to reserve space, e. g. for line numbers >= 10 or >= 100 than you can set this value to 2 or 3 or higher.
        /// </summary>
        [DefaultValue(1)]
        [Description(
            "Reserved space for line number characters. If smaller than needed (e. g. line count >= 10 and " +
            "this value set to 1) this value will have no impact. If you want to reserve space, e. g. for line " +
            "numbers >= 10 or >= 100, than you can set this value to 2 or 3 or higher.")]
        public int ReservedCountOfLineNumberChars
        {
            get { return reservedCountOfLineNumberChars; }
            set
            {
                reservedCountOfLineNumberChars = value;
                NeedRecalc();
                Invalidate();
            }
        }

        /// <summary>
        /// Occurs when mouse is moving over text and tooltip is needed
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when mouse is moving over text and tooltip is needed.")]
        public event EventHandler<ToolTipNeededEventArgs> ToolTipNeeded;

        /// <summary>
        /// Removes all hints
        /// </summary>
        public void ClearHints()
        {
            if (Hints != null)
                Hints.Clear();
        }

        /// <summary>
        /// Add and shows the hint
        /// </summary>
        /// <param name="range">Linked range</param>
        /// <param name="innerControl">Inner control</param>
        /// <param name="scrollToHint">Scrolls textbox to the hint</param>
        /// <param name="inline">Inlining. If True then hint will moves apart text</param>
        /// <param name="dock">Docking. If True then hint will fill whole line</param>
        public virtual Hint AddHint(Range range, Control innerControl, bool scrollToHint, bool inline,
                                    bool dock)
        {
            var hint = new Hint(range, innerControl, inline, dock);
            Hints.Add(hint);
            if (scrollToHint)
                hint.DoVisible();

            return hint;
        }

        /// <summary>
        /// Add and shows the hint
        /// </summary>
        /// <param name="range">Linked range</param>
        /// <param name="innerControl">Inner control</param>
        public Hint AddHint(Range range, Control innerControl)
        {
            return AddHint(range, innerControl, true, true, true);
        }

        /// <summary>
        /// Add and shows simple text hint
        /// </summary>
        /// <param name="range">Linked range</param>
        /// <param name="text">Text of simple hint</param>
        /// <param name="scrollToHint">Scrolls textbox to the hint</param>
        /// <param name="inline">Inlining. If True then hint will moves apart text</param>
        /// <param name="dock">Docking. If True then hint will fill whole line</param>
        public virtual Hint AddHint(Range range, string text, bool scrollToHint, bool inline, bool dock)
        {
            var hint = new Hint(range, text, inline, dock);
            Hints.Add(hint);
            if (scrollToHint)
                hint.DoVisible();

            return hint;
        }

        /// <summary>
        /// Add and shows simple text hint
        /// </summary>
        /// <param name="range">Linked range</param>
        /// <param name="text">Text of simple hint</param>
        public Hint AddHint(Range range, string text)
        {
            return AddHint(range, text, true, true, true);
        }

        /// <summary>
        /// Occurs when user click on the hint
        /// </summary>
        /// <param name="hint"></param>
        public virtual void OnHintClick(Hint hint)
        {
            if (HintClick != null)
                HintClick(this, new HintClickEventArgs(hint));
        }

        private void timer3_Tick(object sender, EventArgs e)
        {
            timer3.Stop();
            OnToolTip();
        }

        protected virtual void OnToolTip()
        {
            if (ToolTip == null)
                return;
            if (ToolTipNeeded == null)
                return;

            //get place under mouse
            Place place = PointToPlace(lastMouseCoord);

            //check distance
            Point p = PlaceToPoint(place);
            if (Math.Abs(p.X - lastMouseCoord.X) > CharWidth*2 ||
                Math.Abs(p.Y - lastMouseCoord.Y) > CharHeight*2)
                return;
            //get word under mouse
            var r = new Range(this, place, place);
            string hoveredWord = r.GetFragment("[a-zA-Z]").Text;
            //event handler
            var ea = new ToolTipNeededEventArgs(place, hoveredWord);
            ToolTipNeeded(this, ea);

            if (ea.ToolTipText != null)
            {
                //show tooltip
                ToolTip.ToolTipTitle = ea.ToolTipTitle;
                ToolTip.ToolTipIcon = ea.ToolTipIcon;
                //ToolTip.SetToolTip(this, ea.ToolTipText);
                ToolTip.Show(ea.ToolTipText, this, new Point(lastMouseCoord.X, lastMouseCoord.Y + CharHeight));
            }
        }

        /// <summary>
        /// Occurs when VisibleRange is changed
        /// </summary>
        public virtual void OnVisibleRangeChanged()
        {
            needRecalcFoldingLines = true;

            needRiseVisibleRangeChangedDelayed = true;
            ResetTimer(timer);
            if (VisibleRangeChanged != null)
                VisibleRangeChanged(this, new EventArgs());
        }

        /// <summary>
        /// Invalidates the entire surface of the control and causes the control to be redrawn.
        /// This method is thread safe and does not require Invoke.
        /// </summary>
        public new void Invalidate()
        {
            if (InvokeRequired)
                BeginInvoke(new MethodInvoker(Invalidate));
            else
                base.Invalidate();
        }

        protected virtual void OnCharSizeChanged()
        {
            VerticalScroll.SmallChange = charHeight;
            VerticalScroll.LargeChange = 10*charHeight;
            HorizontalScroll.SmallChange = CharWidth;
        }

        /// <summary>
        /// HintClick event.
        /// It occurs if user click on the hint.
        /// </summary>
        [Browsable(true)]
        [Description("It occurs if user click on the hint.")]
        public event EventHandler<HintClickEventArgs> HintClick;

        /// <summary>
        /// TextChanged event.
        /// It occurs after insert, delete, clear, undo and redo operations.
        /// </summary>
        [Browsable(true)]
        [Description("It occurs after insert, delete, clear, undo and redo operations.")]
        public new event EventHandler<TextChangedEventArgs> TextChanged;

        /// <summary>
        /// Fake event for correct data binding
        /// </summary>
        [Browsable(false)]
        internal event EventHandler BindingTextChanged;

        /// <summary>
        /// Occurs when user paste text from clipboard
        /// </summary>
        [Description("Occurs when user paste text from clipboard")]
        public event EventHandler<TextChangingEventArgs> Pasting;

        /// <summary>
        /// TextChanging event.
        /// It occurs before insert, delete, clear, undo and redo operations.
        /// </summary>
        [Browsable(true)]
        [Description("It occurs before insert, delete, clear, undo and redo operations.")]
        public event EventHandler<TextChangingEventArgs> TextChanging;

        /// <summary>
        /// SelectionChanged event.
        /// It occurs after changing of selection.
        /// </summary>
        [Browsable(true)]
        [Description("It occurs after changing of selection.")]
        public event EventHandler SelectionChanged;

        /// <summary>
        /// VisibleRangeChanged event.
        /// It occurs after changing of visible range.
        /// </summary>
        [Browsable(true)]
        [Description("It occurs after changing of visible range.")]
        public event EventHandler VisibleRangeChanged;

        /// <summary>
        /// TextChangedDelayed event. 
        /// It occurs after insert, delete, clear, undo and redo operations. 
        /// This event occurs with a delay relative to TextChanged, and fires only once.
        /// </summary>
        [Browsable(true)]
        [Description(
            "It occurs after insert, delete, clear, undo and redo operations. This event occurs with a delay relative to TextChanged, and fires only once."
            )]
        public event EventHandler<TextChangedEventArgs> TextChangedDelayed;

        /// <summary>
        /// SelectionChangedDelayed event.
        /// It occurs after changing of selection.
        /// This event occurs with a delay relative to SelectionChanged, and fires only once.
        /// </summary>
        [Browsable(true)]
        [Description(
            "It occurs after changing of selection. This event occurs with a delay relative to SelectionChanged, and fires only once."
            )]
        public event EventHandler SelectionChangedDelayed;

        /// <summary>
        /// VisibleRangeChangedDelayed event.
        /// It occurs after changing of visible range.
        /// This event occurs with a delay relative to VisibleRangeChanged, and fires only once.
        /// </summary>
        [Browsable(true)]
        [Description(
            "It occurs after changing of visible range. This event occurs with a delay relative to VisibleRangeChanged, and fires only once."
            )]
        public event EventHandler VisibleRangeChangedDelayed;

        /// <summary>
        /// It occurs when user click on VisualMarker.
        /// </summary>
        [Browsable(true)]
        [Description("It occurs when user click on VisualMarker.")]
        public event EventHandler<VisualMarkerEventArgs> VisualMarkerClick;

        /// <summary>
        /// It occurs when visible char is enetering (alphabetic, digit, punctuation, DEL, BACKSPACE)
        /// </summary>
        /// <remarks>Set Handle to True for cancel key</remarks>
        [Browsable(true)]
        [Description("It occurs when visible char is enetering (alphabetic, digit, punctuation, DEL, BACKSPACE).")]
        public event KeyPressEventHandler KeyPressing;

        /// <summary>
        /// It occurs when visible char is enetered (alphabetic, digit, punctuation, DEL, BACKSPACE)
        /// </summary>
        [Browsable(true)]
        [Description("It occurs when visible char is enetered (alphabetic, digit, punctuation, DEL, BACKSPACE).")]
        public event KeyPressEventHandler KeyPressed;

        /// <summary>
        /// It occurs when calculates AutoIndent for new line
        /// </summary>
        [Browsable(true)]
        [Description("It occurs when calculates AutoIndent for new line.")]
        public event EventHandler<AutoIndentEventArgs> AutoIndentNeeded;

        /// <summary>
        /// It occurs when line background is painting
        /// </summary>
        [Browsable(true)]
        [Description("It occurs when line background is painting.")]
        public event EventHandler<PaintLineEventArgs> PaintLine;

        /// <summary>
        /// Occurs when line was inserted/added
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when line was inserted/added.")]
        public event EventHandler<LineInsertedEventArgs> LineInserted;

        /// <summary>
        /// Occurs when line was removed
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when line was removed.")]
        public event EventHandler<LineRemovedEventArgs> LineRemoved;

        /// <summary>
        /// Occurs when current highlighted folding area is changed.
        /// Current folding area see in StartFoldingLine and EndFoldingLine.
        /// </summary>
        /// <remarks></remarks>
        [Browsable(true)]
        [Description("Occurs when current highlighted folding area is changed.")]
        public event EventHandler<EventArgs> FoldingHighlightChanged;

        /// <summary>
        /// Occurs when undo/redo stack is changed
        /// </summary>
        /// <remarks></remarks>
        [Browsable(true)]
        [Description("Occurs when undo/redo stack is changed.")]
        public event EventHandler<EventArgs> UndoRedoStateChanged;

        /// <summary>
        /// Occurs when component was zoomed
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when component was zoomed.")]
        public event EventHandler ZoomChanged;


        /// <summary>
        /// Occurs when user pressed key, that specified as CustomAction
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when user pressed key, that specified as CustomAction.")]
        public event EventHandler<CustomActionEventArgs> CustomAction;

        /// <summary>
        /// Occurs when scroolbars are updated
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when scroolbars are updated.")]
        public event EventHandler ScrollbarsUpdated;

        /// <summary>
        /// Occurs when custom wordwrap is needed
        /// </summary>
        [Browsable(true)]
        [Description("Occurs when custom wordwrap is needed.")]
        public event EventHandler<WordWrapNeededEventArgs> WordWrapNeeded;


        /// <summary>
        /// Returns list of styles of given place
        /// </summary>
        public List<Style> GetStylesOfChar(Place place)
        {
            var result = new List<Style>();
            if (place.iLine < LinesCount && place.iChar < this[place.iLine].Count)
            {
#if Styles32
                var s = (uint) this[place].style;
                for (int i = 0; i < 32; i++)
                    if ((s & ((uint) 1) << i) != 0)
                        result.Add(Styles[i]);
#else
                var s = (ushort)this[place].style;
                for (int i = 0; i < 16; i++)
                    if ((s & ((ushort) 1) << i) != 0)
                        result.Add(Styles[i]);
#endif
            }

            return result;
        }

        protected virtual TextSource CreateTextSource()
        {
            return new TextSource(this);
        }

        private void SetAsCurrentTB()
        {
            TextSource.CurrentTB = this;
        }

        protected void InitTextSource(TextSource ts)
        {
            if (lines != null)
            {
                lines.LineInserted -= ts_LineInserted;
                lines.LineRemoved -= ts_LineRemoved;
                lines.TextChanged -= ts_TextChanged;
                lines.RecalcNeeded -= ts_RecalcNeeded;
                lines.RecalcWordWrap -= ts_RecalcWordWrap;
                lines.TextChanging -= ts_TextChanging;

                lines.Dispose();
            }

            LineInfos.Clear();
            ClearHints();
            if (Bookmarks != null)
                Bookmarks.Clear();

            lines = ts;

            if (ts != null)
            {
                ts.LineInserted += ts_LineInserted;
                ts.LineRemoved += ts_LineRemoved;
                ts.TextChanged += ts_TextChanged;
                ts.RecalcNeeded += ts_RecalcNeeded;
                ts.RecalcWordWrap += ts_RecalcWordWrap;
                ts.TextChanging += ts_TextChanging;

                while (LineInfos.Count < ts.Count)
                    LineInfos.Add(new LineInfo(-1));
            }

            isChanged = false;
            needRecalc = true;
        }

        private void ts_RecalcWordWrap(object sender, TextSource.TextChangedEventArgs e)
        {
            RecalcWordWrap(e.iFromLine, e.iToLine);
        }

        private void ts_TextChanging(object sender, TextChangingEventArgs e)
        {
            if (TextSource.CurrentTB == this)
            {
                string text = e.InsertingText;
                OnTextChanging(ref text);
                e.InsertingText = text;
            }
        }

        private void ts_RecalcNeeded(object sender, TextSource.TextChangedEventArgs e)
        {
            if (e.iFromLine == e.iToLine && !WordWrap && lines.Count > minLinesForAccuracy)
                RecalcScrollByOneLine(e.iFromLine);
            else
                needRecalc = true;
        }

        /// <summary>
        /// Call this method if the recalc of the position of lines is needed.
        /// </summary>
        public void NeedRecalc()
        {
            NeedRecalc(false);
        }

        /// <summary>
        /// Call this method if the recalc of the position of lines is needed.
        /// </summary>
        public void NeedRecalc(bool forced)
        {
            NeedRecalc(forced, false);
        }

        /// <summary>
        /// Call this method if the recalc of the position of lines is needed.
        /// </summary>
        public void NeedRecalc(bool forced, bool wordWrapRecalc)
        {
            needRecalc = true;

            if(wordWrapRecalc)
            {
                needRecalcWordWrapInterval = new Point(0, LinesCount - 1);
                needRecalcWordWrap = true;
            }

            if (forced)
                Recalc();
        }

        private void ts_TextChanged(object sender, TextSource.TextChangedEventArgs e)
        {
            if (e.iFromLine == e.iToLine && !WordWrap)
                RecalcScrollByOneLine(e.iFromLine);
            else
                needRecalc = true;

            Invalidate();
            if (TextSource.CurrentTB == this)
                OnTextChanged(e.iFromLine, e.iToLine);
        }

        private void ts_LineRemoved(object sender, LineRemovedEventArgs e)
        {
            LineInfos.RemoveRange(e.Index, e.Count);
            OnLineRemoved(e.Index, e.Count, e.RemovedLineUniqueIds);
        }

        private void ts_LineInserted(object sender, LineInsertedEventArgs e)
        {
            VisibleState newState = VisibleState.Visible;
            if (e.Index >= 0 && e.Index < LineInfos.Count && LineInfos[e.Index].VisibleState == VisibleState.Hidden)
                newState = VisibleState.Hidden;

            if (e.Count > 100000)
                LineInfos.Capacity = LineInfos.Count + e.Count + 1000;

            var temp = new LineInfo[e.Count];
            for (int i = 0; i < e.Count; i++)
            {
                temp[i].startY = -1;
                temp[i].VisibleState = newState;
            }
            LineInfos.InsertRange(e.Index, temp);

            /* 
            for (int i = 0; i < e.Count; i++)
            {
                LineInfos.Add(new LineInfo(-1) { VisibleState = newState });//<---- needed Insert
                if(i % 1000000 == 0 && i > 0)
                    GC.Collect();
            }*/

            if (e.Count > 1000000)
                GC.Collect();

            OnLineInserted(e.Index, e.Count);
        }

        /// <summary>
        /// Navigates forward (by Line.LastVisit property)
        /// </summary>
        public bool NavigateForward()
        {
            DateTime min = DateTime.Now;
            int iLine = -1;
            for (int i = 0; i < LinesCount; i++)
                if (lines.IsLineLoaded(i))
                    if (lines[i].LastVisit > lastNavigatedDateTime && lines[i].LastVisit < min)
                    {
                        min = lines[i].LastVisit;
                        iLine = i;
                    }
            if (iLine >= 0)
            {
                Navigate(iLine);
                return true;
            }
            else
                return false;
        }

        /// <summary>
        /// Navigates backward (by Line.LastVisit property)
        /// </summary>
        public bool NavigateBackward()
        {
            var max = new DateTime();
            int iLine = -1;
            for (int i = 0; i < LinesCount; i++)
                if (lines.IsLineLoaded(i))
                    if (lines[i].LastVisit < lastNavigatedDateTime && lines[i].LastVisit > max)
                    {
                        max = lines[i].LastVisit;
                        iLine = i;
                    }
            if (iLine >= 0)
            {
                Navigate(iLine);
                return true;
            }
            else
                return false;
        }

        /// <summary>
        /// Navigates to defined line, without Line.LastVisit reseting
        /// </summary>
        public void Navigate(int iLine)
        {
            if (iLine >= LinesCount) return;
            lastNavigatedDateTime = lines[iLine].LastVisit;
            Selection.Start = new Place(0, iLine);
            DoSelectionVisible();
        }

        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            m_hImc = NativeMethodsWrapper.ImmGetContext(Handle);
        }

        private void timer2_Tick(object sender, EventArgs e)
        {
            timer2.Enabled = false;
            if (needRiseTextChangedDelayed)
            {
                needRiseTextChangedDelayed = false;
                if (delayedTextChangedRange == null)
                    return;
                delayedTextChangedRange = Range.GetIntersectionWith(delayedTextChangedRange);
                delayedTextChangedRange.Expand();
                OnTextChangedDelayed(delayedTextChangedRange);
                delayedTextChangedRange = null;
            }
        }

        public void AddVisualMarker(VisualMarker marker)
        {
            visibleMarkers.Add(marker);
        }

        private void timer_Tick(object sender, EventArgs e)
        {
            timer.Enabled = false;
            if (needRiseSelectionChangedDelayed)
            {
                needRiseSelectionChangedDelayed = false;
                OnSelectionChangedDelayed();
            }
            if (needRiseVisibleRangeChangedDelayed)
            {
                needRiseVisibleRangeChangedDelayed = false;
                OnVisibleRangeChangedDelayed();
            }
        }

        public virtual void OnTextChangedDelayed(Range changedRange)
        {
            if (TextChangedDelayed != null)
                TextChangedDelayed(this, new TextChangedEventArgs(changedRange));
        }

        public virtual void OnSelectionChangedDelayed()
        {
            RecalcScrollByOneLine(Selection.Start.iLine);
            //highlight brackets
            ClearBracketsPositions();
            if (LeftBracket != '\x0' && RightBracket != '\x0')
                HighlightBrackets(LeftBracket, RightBracket, ref leftBracketPosition, ref rightBracketPosition);
            if (LeftBracket2 != '\x0' && RightBracket2 != '\x0')
                HighlightBrackets(LeftBracket2, RightBracket2, ref leftBracketPosition2, ref rightBracketPosition2);
            //remember last visit time
            if (Selection.IsEmpty && Selection.Start.iLine < LinesCount)
            {
                if (lastNavigatedDateTime != lines[Selection.Start.iLine].LastVisit)
                {
                    lines[Selection.Start.iLine].LastVisit = DateTime.Now;
                    lastNavigatedDateTime = lines[Selection.Start.iLine].LastVisit;
                }
            }

            if (SelectionChangedDelayed != null)
                SelectionChangedDelayed(this, new EventArgs());
        }

        public virtual void OnVisibleRangeChangedDelayed()
        {
            if (VisibleRangeChangedDelayed != null)
                VisibleRangeChangedDelayed(this, new EventArgs());
        }

        Dictionary<Timer, Timer> timersToReset = new Dictionary<Timer, Timer>();

        private void ResetTimer(Timer timer)
        {
            if(InvokeRequired)
            {
                BeginInvoke(new MethodInvoker(()=>ResetTimer(timer)));
                return;
            }
            timer.Stop();
            if (IsHandleCreated)
                timer.Start();
            else
                timersToReset[timer] = timer;
        }

        protected override void OnHandleCreated(EventArgs e)
        {
            base.OnHandleCreated(e);
            foreach (var timer in new List<Timer>(timersToReset.Keys))
                ResetTimer(timer);
            timersToReset.Clear();

            OnScrollbarsUpdated();
        }

        /// <summary>
        /// Add new style
        /// </summary>
        /// <returns>Layer index of this style</returns>
        public int AddStyle(Style style)
        {
            if (style == null) return -1;

            int i = GetStyleIndex(style);
            if (i >= 0)
                return i;

            for (i = Styles.Length - 1; i >= 0; i--)
                if (Styles[i] != null)
                    break;

            i++;
            if (i >= Styles.Length)
                throw new Exception("Maximum count of Styles is exceeded.");

            Styles[i] = style;
            return i;
        }

        /// <summary>
        /// Shows find dialog
        /// </summary>
        public void ShowFindDialog()
        {
            ShowFindDialog(null);
        }

        /// <summary>
        /// Shows find dialog
        /// </summary>
        public void ShowFindDialog(string findText)
        {
            if (findForm == null)
                findForm = new FindForm(this);

            if (findText != null)
                findForm.tbFind.Text = findText;
            else if (!Selection.IsEmpty && Selection.Start.iLine == Selection.End.iLine)
                findForm.tbFind.Text = Selection.Text;

            findForm.tbFind.SelectAll();
            findForm.StartPosition = FormStartPosition.Manual;
            findForm.Top = this.FindForm().Top + (this.FindForm().Height - findForm.Height) / 2;
            findForm.Left = this.FindForm().Left + (this.FindForm().Width - findForm.Width) / 2;
            findForm.Show(this);
            findForm.Focus();
        }

        /// <summary>
        /// Shows replace dialog
        /// </summary>
        public void ShowReplaceDialog()
        {
            ShowReplaceDialog(null);
        }

        /// <summary>
        /// Shows replace dialog
        /// </summary>
        public virtual void ShowReplaceDialog(string findText)
        {
            if (ReadOnly)
                return;
            if (replaceForm == null)
                replaceForm = new ReplaceForm(this);

            if (findText != null)
                replaceForm.tbFind.Text = findText;
            else if (!Selection.IsEmpty && Selection.Start.iLine == Selection.End.iLine)
                replaceForm.tbFind.Text = Selection.Text;

            replaceForm.tbFind.SelectAll();
            replaceForm.Show();
            replaceForm.Focus();
        }

        /// <summary>
        /// Gets length of given line
        /// </summary>
        /// <param name="iLine">Line index</param>
        /// <returns>Length of line</returns>
        public int GetLineLength(int iLine)
        {
            if (iLine < 0 || iLine >= lines.Count)
                throw new ArgumentOutOfRangeException("Line index out of range");

            return lines[iLine].Count;
        }

        /// <summary>
        /// Get range of line
        /// </summary>
        /// <param name="iLine">Line index</param>
        public Range GetLine(int iLine)
        {
            if (iLine < 0 || iLine >= lines.Count)
                throw new ArgumentOutOfRangeException("Line index out of range");

            var sel = new Range(this);
            sel.Start = new Place(0, iLine);
            sel.End = new Place(lines[iLine].Count, iLine);
            return sel;
        }

        /// <summary>
        /// Copy selected text into Clipboard
        /// </summary>
        public virtual void Copy()
        {
            if (Selection.IsEmpty)
                Selection.Expand();
            if (!Selection.IsEmpty)
            {
                var data = new DataObject();
                OnCreateClipboardData(data);
                //
                var thread = new Thread(() => SetClipboard(data));
                thread.SetApartmentState(ApartmentState.STA);
                thread.Start();
                thread.Join();
            }
        }

        protected virtual void OnCreateClipboardData(DataObject data)
        {
            var exp = new ExportToHTML();
            exp.UseBr = false;
            exp.UseNbsp = false;
            exp.UseStyleTag = true;
            string html = "<pre>" + exp.GetHtml(Selection.Clone()) + "</pre>";

            data.SetData(DataFormats.UnicodeText, true, Selection.Text);
            data.SetData(DataFormats.Html, PrepareHtmlForClipboard(html));
            data.SetData(DataFormats.Rtf, new ExportToRTF().GetRtf(Selection.Clone()));
        }

        protected void SetClipboard(DataObject data)
        {
                try
                {
                    /*
                    while (GetOpenClipboardWindow() != IntPtr.Zero)
                        Thread.Sleep(0);*/
                    NativeMethodsWrapper.CloseClipboard();
                    Clipboard.SetDataObject(data, true, 5, 100);
                }
                catch(ExternalException)
                {
                    //occurs if some other process holds open clipboard
                }
        }

        public static MemoryStream PrepareHtmlForClipboard(string html)
        {
            Encoding enc = Encoding.UTF8;

            string begin = "Version:0.9\r\nStartHTML:{0:000000}\r\nEndHTML:{1:000000}"
                           + "\r\nStartFragment:{2:000000}\r\nEndFragment:{3:000000}\r\n";

            string html_begin = "<html>\r\n<head>\r\n"
                                + "<meta http-equiv=\"Content-Type\""
                                + " content=\"text/html; charset=" + enc.WebName + "\">\r\n"
                                + "<title>HTML clipboard</title>\r\n</head>\r\n<body>\r\n"
                                + "<!--StartFragment-->";

            string html_end = "<!--EndFragment-->\r\n</body>\r\n</html>\r\n";

            string begin_sample = String.Format(begin, 0, 0, 0, 0);

            int count_begin = enc.GetByteCount(begin_sample);
            int count_html_begin = enc.GetByteCount(html_begin);
            int count_html = enc.GetByteCount(html);
            int count_html_end = enc.GetByteCount(html_end);

            string html_total = String.Format(
                begin
                , count_begin
                , count_begin + count_html_begin + count_html + count_html_end
                , count_begin + count_html_begin
                , count_begin + count_html_begin + count_html
                                    ) + html_begin + html + html_end;

            return new MemoryStream(enc.GetBytes(html_total));
        }


        /// <summary>
        /// Cut selected text into Clipboard
        /// </summary>
        public virtual void Cut()
        {
            if (!Selection.IsEmpty)
            {
                Copy();
                ClearSelected();
            }
            else
            if (LinesCount == 1)
            {
                Selection.SelectAll();
                Copy();
                ClearSelected();
            }
            else
            {
                Copy();
                //remove current line
                if (Selection.Start.iLine >= 0 && Selection.Start.iLine < LinesCount)
                {
                    int iLine = Selection.Start.iLine;
                    RemoveLines(new List<int> {iLine});
                    Selection.Start = new Place(0, Math.Max(0, Math.Min(iLine, LinesCount - 1)));
                }
            }
        }

        /// <summary>
        /// Paste text from clipboard into selected position
        /// </summary>
        public virtual void Paste()
        {
            string text = null;
            var thread = new Thread(() =>
                                        {
                                            if (Clipboard.ContainsText())
                                                text = Clipboard.GetText();
                                        });
            thread.SetApartmentState(ApartmentState.STA);
            thread.Start();
            thread.Join();

            if (Pasting != null)
            {
                var args = new TextChangingEventArgs
                               {
                                   Cancel = false,
                                   InsertingText = text
                               };

                Pasting(this, args);

                if (args.Cancel)
                    text = string.Empty;
                else
                    text = args.InsertingText;
            }

            if (!string.IsNullOrEmpty(text))
                InsertText(text);
        }

        /// <summary>
        /// Select all chars of text
        /// </summary>
        public void SelectAll()
        {
            Selection.SelectAll();
        }

        /// <summary>
        /// Move caret to end of text
        /// </summary>
        public void GoEnd()
        {
            if (lines.Count > 0)
                Selection.Start = new Place(lines[lines.Count - 1].Count, lines.Count - 1);
            else
                Selection.Start = new Place(0, 0);

            DoCaretVisible();
        }

        /// <summary>
        /// Move caret to first position
        /// </summary>
        public void GoHome()
        {
            Selection.Start = new Place(0, 0);

            DoCaretVisible();
            //VerticalScroll.Value = 0;
            //HorizontalScroll.Value = 0;
        }

        /// <summary>
        /// Clear text, styles, history, caches
        /// </summary>
        public virtual void Clear()
        {
            Selection.BeginUpdate();
            try
            {
                Selection.SelectAll();
                ClearSelected();
                lines.Manager.ClearHistory();
                Invalidate();
            }
            finally
            {
                Selection.EndUpdate();
            }
        }

        /// <summary>
        /// Clear buffer of styles
        /// </summary>
        public void ClearStylesBuffer()
        {
            for (int i = 0; i < Styles.Length; i++)
                Styles[i] = null;
        }

        /// <summary>
        /// Clear style of all text
        /// </summary>
        public void ClearStyle(StyleIndex styleIndex)
        {
            foreach (Line line in lines)
                line.ClearStyle(styleIndex);

            for (int i = 0; i < LineInfos.Count; i++)
                SetVisibleState(i, VisibleState.Visible);

            Invalidate();
        }


        /// <summary>
        /// Clears undo and redo stacks
        /// </summary>
        public void ClearUndo()
        {
            lines.Manager.ClearHistory();
        }

        /// <summary>
        /// Insert text into current selected position
        /// </summary>
        public virtual void InsertText(string text)
        {
            InsertText(text, true);
        }

        /// <summary>
        /// Insert text into current selected position
        /// </summary>
        /// <param name="text"></param>
        public virtual void InsertText(string text, bool jumpToCaret)
        {
            if (text == null)
                return;
            if (text == "\r")
                text = "\n";

            lines.Manager.BeginAutoUndoCommands();
            try
            {
                if (!Selection.IsEmpty)
                    lines.Manager.ExecuteCommand(new ClearSelectedCommand(TextSource));

                //insert virtual spaces
                if(this.TextSource.Count > 0)
                if (Selection.IsEmpty && Selection.Start.iChar > GetLineLength(Selection.Start.iLine) && VirtualSpace)
                    InsertVirtualSpaces();

                lines.Manager.ExecuteCommand(new InsertTextCommand(TextSource, text));
                if (updating <= 0 && jumpToCaret)
                    DoCaretVisible();
            }
            finally
            {
                lines.Manager.EndAutoUndoCommands();
            }
            //
            Invalidate();
        }

        /// <summary>
        /// Insert text into current selection position (with predefined style)
        /// </summary>
        /// <param name="text"></param>
        public virtual Range InsertText(string text, Style style)
        {
            return InsertText(text, style, true);
        }

        /// <summary>
        /// Insert text into current selection position (with predefined style)
        /// </summary>
        public virtual Range InsertText(string text, Style style, bool jumpToCaret)
        {
            if (text == null)
                return null;

            //remember last caret position
            Place last = Selection.Start > Selection.End ? Selection.End : Selection.Start;
            //insert text
            InsertText(text, jumpToCaret);
            //get range
            var range = new Range(this, last, Selection.Start){ColumnSelectionMode = Selection.ColumnSelectionMode};
            range = range.GetIntersectionWith(Range);
            //set style for range
            range.SetStyle(style);

            return range;
        }

        /// <summary>
        /// Append string to end of the Text
        /// </summary>
        public virtual void AppendText(string text)
        {
            AppendText(text, null);
        }

        /// <summary>
        /// Append string to end of the Text
        /// </summary>
        public virtual void AppendText(string text, Style style)
        {
            if (text == null)
                return;

            Selection.ColumnSelectionMode = false;

            Place oldStart = Selection.Start;
            Place oldEnd = Selection.End;

            Selection.BeginUpdate();
            lines.Manager.BeginAutoUndoCommands();
            try
            {
                if (lines.Count > 0)
                    Selection.Start = new Place(lines[lines.Count - 1].Count, lines.Count - 1);
                else
                    Selection.Start = new Place(0, 0);

                //remember last caret position
                Place last = Selection.Start;

                lines.Manager.ExecuteCommand(new InsertTextCommand(TextSource, text));

                if (style != null)
                    new Range(this, last, Selection.Start).SetStyle(style);
            }
            finally
            {
                lines.Manager.EndAutoUndoCommands();
                Selection.Start = oldStart;
                Selection.End = oldEnd;
                Selection.EndUpdate();
            }
            //
            Invalidate();
        }

        /// <summary>
        /// Returns index of the style in Styles
        /// -1 otherwise
        /// </summary>
        /// <param name="style"></param>
        /// <returns>Index of the style in Styles</returns>
        public int GetStyleIndex(Style style)
        {
            return Array.IndexOf(Styles, style);
        }

        /// <summary>
        /// Returns StyleIndex mask of given styles
        /// </summary>
        /// <param name="styles"></param>
        /// <returns>StyleIndex mask of given styles</returns>
        public StyleIndex GetStyleIndexMask(Style[] styles)
        {
            StyleIndex mask = StyleIndex.None;
            foreach (Style style in styles)
            {
                int i = GetStyleIndex(style);
                if (i >= 0)
                    mask |= Range.ToStyleIndex(i);
            }

            return mask;
        }

        internal int GetOrSetStyleLayerIndex(Style style)
        {
            int i = GetStyleIndex(style);
            if (i < 0)
                i = AddStyle(style);
            return i;
        }

        public static SizeF GetCharSize(Font font, char c)
        {
            Size sz2 = TextRenderer.MeasureText("<" + c.ToString() + ">", font);
            Size sz3 = TextRenderer.MeasureText("<>", font);

            return new SizeF(sz2.Width - sz3.Width + 1, /*sz2.Height*/font.Height);
        }

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_HSCROLL || m.Msg == WM_VSCROLL)
                if (m.WParam.ToInt32() != SB_ENDSCROLL)
                    Invalidate();
            
            base.WndProc(ref m);

            if (ImeAllowed)
                if (m.Msg == WM_IME_SETCONTEXT && m.WParam.ToInt32() == 1)
                {
                    NativeMethodsWrapper.ImmAssociateContext(Handle, m_hImc);
                }
        }

        public void OnScroll(ScrollEventArgs se, bool alignByLines)
        {
            if (se.ScrollOrientation == ScrollOrientation.VerticalScroll)
            {
                //align by line height
                int newValue = se.NewValue;
                if (alignByLines)
                    newValue = (int)(Math.Ceiling(1d * newValue / CharHeight) * CharHeight);
                //
                VerticalScroll.Value = Math.Max(VerticalScroll.Minimum, Math.Min(VerticalScroll.Maximum, newValue));
            }
            if (se.ScrollOrientation == ScrollOrientation.HorizontalScroll)
                HorizontalScroll.Value = Math.Max(HorizontalScroll.Minimum, Math.Min(HorizontalScroll.Maximum, se.NewValue));

            UpdateScrollbars();

            Invalidate();
            //
            base.OnScroll(se);
            OnVisibleRangeChanged();
        }

        protected override void OnScroll(ScrollEventArgs se)
        {
            OnScroll(se, true);
        }

        protected virtual void InsertChar(char c)
        {
            lines.Manager.BeginAutoUndoCommands();
            try
            {
                if (!Selection.IsEmpty)
                    lines.Manager.ExecuteCommand(new ClearSelectedCommand(TextSource));

                //insert virtual spaces
                if (Selection.IsEmpty && Selection.Start.iChar > GetLineLength(Selection.Start.iLine) && VirtualSpace)
                    InsertVirtualSpaces();

                //insert char
                lines.Manager.ExecuteCommand(new InsertCharCommand(TextSource, c));
            }
            finally
            {
                lines.Manager.EndAutoUndoCommands();
            }

            Invalidate();
        }

        private void InsertVirtualSpaces()
        {
            int lineLength = GetLineLength(Selection.Start.iLine);
            int count = Selection.Start.iChar - lineLength;
            Selection.BeginUpdate();
            try
            {
                Selection.Start = new Place(lineLength, Selection.Start.iLine);
                lines.Manager.ExecuteCommand(new InsertTextCommand(TextSource, new string(' ', count)));
            }
            finally
            {
                Selection.EndUpdate();
            }
        }

        /// <summary>
        /// Deletes selected chars
        /// </summary>
        public virtual void ClearSelected()
        {
            if (!Selection.IsEmpty)
            {
                lines.Manager.ExecuteCommand(new ClearSelectedCommand(TextSource));
                Invalidate();
            }
        }

        /// <summary>
        /// Deletes current line(s)
        /// </summary>
        public void ClearCurrentLine()
        {
            Selection.Expand();

            lines.Manager.ExecuteCommand(new ClearSelectedCommand(TextSource));
            if (Selection.Start.iLine == 0)
                if (!Selection.GoRightThroughFolded()) return;
            if (Selection.Start.iLine > 0)
                lines.Manager.ExecuteCommand(new InsertCharCommand(TextSource, '\b')); //backspace
            Invalidate();
        }

        private void Recalc()
        {
            if (!needRecalc)
                return;

#if debug
            var sw = Stopwatch.StartNew();
#endif

            needRecalc = false;
            //calc min left indent
            LeftIndent = LeftPadding;
            long maxLineNumber = LinesCount + lineNumberStartValue - 1;
            int charsForLineNumber = 2 + (maxLineNumber > 0 ? (int) Math.Log10(maxLineNumber) : 0);

            // If there are reserved character for line numbers: correct this
            if (this.ReservedCountOfLineNumberChars + 1 > charsForLineNumber)
                charsForLineNumber = this.ReservedCountOfLineNumberChars + 1;

            if (Created)
            {
                if (ShowLineNumbers)
                    LeftIndent += charsForLineNumber*CharWidth + minLeftIndent + 1;

                //calc wordwrapping
                if (needRecalcWordWrap)
                {
                    RecalcWordWrap(needRecalcWordWrapInterval.X, needRecalcWordWrapInterval.Y);
                    needRecalcWordWrap = false;
                }
            }
            else
                needRecalc = true;

            //calc max line length and count of wordWrapLines
            TextHeight = 0;

            maxLineLength = RecalcMaxLineLength();

            //adjust AutoScrollMinSize
            int minWidth;
            CalcMinAutosizeWidth(out minWidth, ref maxLineLength);
            
            AutoScrollMinSize = new Size(minWidth, TextHeight + Paddings.Top + Paddings.Bottom);
            UpdateScrollbars();
#if debug
            sw.Stop();
            Console.WriteLine("Recalc: " + sw.ElapsedMilliseconds);
#endif
        }

        private void CalcMinAutosizeWidth(out int minWidth, ref int maxLineLength)
        {
            //adjust AutoScrollMinSize
            minWidth = LeftIndent + (maxLineLength)*CharWidth + 2 + Paddings.Left + Paddings.Right;
            if (wordWrap)
                switch (WordWrapMode)
                {
                    case WordWrapMode.WordWrapControlWidth:
                    case WordWrapMode.CharWrapControlWidth:
                        maxLineLength = Math.Min(maxLineLength,
                                                 (ClientSize.Width - LeftIndent - Paddings.Left - Paddings.Right)/
                                                 CharWidth);
                        minWidth = 0;
                        break;
                    case WordWrapMode.WordWrapPreferredWidth:
                    case WordWrapMode.CharWrapPreferredWidth:
                        maxLineLength = Math.Min(maxLineLength, PreferredLineWidth);
                        minWidth = LeftIndent + PreferredLineWidth*CharWidth + 2 + Paddings.Left + Paddings.Right;
                        break;
                }
        }

        private void RecalcScrollByOneLine(int iLine)
        {
            if (iLine >= lines.Count)
                return;

            int maxLineLength = lines[iLine].Count;
            if (this.maxLineLength < maxLineLength && !WordWrap)
                this.maxLineLength = maxLineLength;

            int minWidth;
            CalcMinAutosizeWidth(out minWidth, ref maxLineLength);

            if (AutoScrollMinSize.Width < minWidth)
                AutoScrollMinSize = new Size(minWidth, AutoScrollMinSize.Height);
        }

        private int RecalcMaxLineLength()
        {
            int maxLineLength = 0;
            TextSource lines = this.lines;
            int count = lines.Count;
            int charHeight = CharHeight;
            int topIndent = Paddings.Top;
            TextHeight = topIndent;

            for (int i = 0; i < count; i++)
            {
                int lineLength = lines.GetLineLength(i);
                LineInfo lineInfo = LineInfos[i];
                if (lineLength > maxLineLength && lineInfo.VisibleState == VisibleState.Visible)
                    maxLineLength = lineLength;
                lineInfo.startY = TextHeight;
                TextHeight += lineInfo.WordWrapStringsCount*charHeight + lineInfo.bottomPadding;
                LineInfos[i] = lineInfo;
            }

            TextHeight -= topIndent;

            return maxLineLength;
        }

        private int GetMaxLineWordWrapedWidth()
        {
            if (wordWrap)
                switch (wordWrapMode)
                {
                    case WordWrapMode.WordWrapControlWidth:
                    case WordWrapMode.CharWrapControlWidth:
                        return ClientSize.Width;
                    case WordWrapMode.WordWrapPreferredWidth:
                    case WordWrapMode.CharWrapPreferredWidth:
                        return LeftIndent + PreferredLineWidth*CharWidth + 2 + Paddings.Left + Paddings.Right;
                }

            return int.MaxValue;
        }

        private void RecalcWordWrap(int fromLine, int toLine)
        {
            int maxCharsPerLine = 0;
            bool charWrap = false;

            toLine = Math.Min(LinesCount - 1, toLine);

            switch (WordWrapMode)
            {
                case WordWrapMode.WordWrapControlWidth:
                    maxCharsPerLine = (ClientSize.Width - LeftIndent - Paddings.Left - Paddings.Right)/CharWidth;
                    break;
                case WordWrapMode.CharWrapControlWidth:
                    maxCharsPerLine = (ClientSize.Width - LeftIndent - Paddings.Left - Paddings.Right)/CharWidth;
                    charWrap = true;
                    break;
                case WordWrapMode.WordWrapPreferredWidth:
                    maxCharsPerLine = PreferredLineWidth;
                    break;
                case WordWrapMode.CharWrapPreferredWidth:
                    maxCharsPerLine = PreferredLineWidth;
                    charWrap = true;
                    break;
            }

            for (int iLine = fromLine; iLine <= toLine; iLine++)
                if (lines.IsLineLoaded(iLine))
                {
                    if (!wordWrap)
                        LineInfos[iLine].CutOffPositions.Clear();
                    else
                    {
                        LineInfo li = LineInfos[iLine];

                        li.wordWrapIndent = WordWrapAutoIndent ? lines[iLine].StartSpacesCount + WordWrapIndent : WordWrapIndent;

                        if (WordWrapMode == WordWrapMode.Custom)
                        {
                            if (WordWrapNeeded != null)
                                WordWrapNeeded(this, new WordWrapNeededEventArgs(li.CutOffPositions, ImeAllowed, lines[iLine]));
                        }
                        else
                            CalcCutOffs(li.CutOffPositions, maxCharsPerLine, maxCharsPerLine - li.wordWrapIndent, ImeAllowed, charWrap, lines[iLine]);

                        LineInfos[iLine] = li;
                    }
                }
            needRecalc = true;
        }

        /// <summary>
        /// Calculates wordwrap cutoffs
        /// </summary>
        public static void CalcCutOffs(List<int> cutOffPositions, int maxCharsPerLine, int maxCharsPerSecondaryLine, bool allowIME, bool charWrap, Line line)
        {
            if (maxCharsPerSecondaryLine < 1) maxCharsPerSecondaryLine = 1;
            if (maxCharsPerLine < 1) maxCharsPerLine = 1;

            int segmentLength = 0;
            int cutOff = 0;
            cutOffPositions.Clear();

            for (int i = 0; i < line.Count - 1; i++)
            {
                char c = line[i].c;
                if (charWrap)
                {
                    //char wrapping
                    cutOff = i + 1;
                }
                else
                {
                    //word wrapping
                    if (allowIME && IsCJKLetter(c))//in CJK languages cutoff can be in any letter
                    {
                        cutOff = i;
                    }
                    else
                        if (!char.IsLetterOrDigit(c) && c != '_' && c != '\'' && c != '\xa0')
                            cutOff = Math.Min(i + 1, line.Count - 1);
                }

                segmentLength++;

                if (segmentLength == maxCharsPerLine)
                {
                    if (cutOff == 0 || (cutOffPositions.Count > 0 && cutOff == cutOffPositions[cutOffPositions.Count - 1]))
                        cutOff = i + 1;
                    cutOffPositions.Add(cutOff);
                    segmentLength = 1 + i - cutOff;
                    maxCharsPerLine = maxCharsPerSecondaryLine;
                }
            }
        }

        public static bool IsCJKLetter(char c)
        {
            int code = Convert.ToInt32(c);
            return
            (code >= 0x3300 && code <= 0x33FF) ||
            (code >= 0xFE30 && code <= 0xFE4F) ||
            (code >= 0xF900 && code <= 0xFAFF) ||
            (code >= 0x2E80 && code <= 0x2EFF) ||
            (code >= 0x31C0 && code <= 0x31EF) ||
            (code >= 0x4E00 && code <= 0x9FFF) ||
            (code >= 0x3400 && code <= 0x4DBF) ||
            (code >= 0x3200 && code <= 0x32FF) ||
            (code >= 0x2460 && code <= 0x24FF) ||
            (code >= 0x3040 && code <= 0x309F) ||
            (code >= 0x2F00 && code <= 0x2FDF) ||
            (code >= 0x31A0 && code <= 0x31BF) ||
            (code >= 0x4DC0 && code <= 0x4DFF) ||
            (code >= 0x3100 && code <= 0x312F) ||
            (code >= 0x30A0 && code <= 0x30FF) ||
            (code >= 0x31F0 && code <= 0x31FF) ||
            (code >= 0x2FF0 && code <= 0x2FFF) ||
            (code >= 0x1100 && code <= 0x11FF) ||
            (code >= 0xA960 && code <= 0xA97F) ||
            (code >= 0xD7B0 && code <= 0xD7FF) ||
            (code >= 0x3130 && code <= 0x318F) ||
            (code >= 0xAC00 && code <= 0xD7AF);

        }

        protected override void OnClientSizeChanged(EventArgs e)
        {
            base.OnClientSizeChanged(e);
            if (WordWrap)
            {
                //RecalcWordWrap(0, lines.Count - 1);
                NeedRecalc(false, true);
                Invalidate();
            }
            OnVisibleRangeChanged();
            UpdateScrollbars();
        }

        /// <summary>
        /// Scroll control for display defined rectangle
        /// </summary>
        /// <param name="rect"></param>
        private void DoVisibleRectangle(Rectangle rect)
        {
            int oldV = VerticalScroll.Value;
            int v = VerticalScroll.Value;
            int h = HorizontalScroll.Value;

            if (rect.Bottom > ClientRectangle.Height)
                v += rect.Bottom - ClientRectangle.Height;
            else if (rect.Top < 0)
                v += rect.Top;

            if (rect.Right > ClientRectangle.Width)
                h += rect.Right - ClientRectangle.Width;
            else if (rect.Left < LeftIndent)
                h += rect.Left - LeftIndent;
            //
            if (!Multiline)
                v = 0;
            //
            v = Math.Max(VerticalScroll.Minimum, v); // was 0
            h = Math.Max(HorizontalScroll.Minimum, h); // was 0
            //
            try
            {
                if (VerticalScroll.Visible || !ShowScrollBars)
                    VerticalScroll.Value = Math.Min(v, VerticalScroll.Maximum);
                if (HorizontalScroll.Visible || !ShowScrollBars)
                    HorizontalScroll.Value = Math.Min(h, HorizontalScroll.Maximum);
            }
            catch (ArgumentOutOfRangeException)
            {
                ;
            }

            UpdateScrollbars();
            //
            if (oldV != VerticalScroll.Value)
                OnVisibleRangeChanged();
        }

        /// <summary>
        /// Updates scrollbar position after Value changed
        /// </summary>
        public void UpdateScrollbars()
        {
            if (ShowScrollBars)
            {
                //some magic for update scrolls
                base.AutoScrollMinSize -= new Size(1, 0);
                base.AutoScrollMinSize += new Size(1, 0);

            }
            else
                AutoScrollMinSize = AutoScrollMinSize;

            if(IsHandleCreated)
                BeginInvoke((MethodInvoker)OnScrollbarsUpdated);
        }

        protected virtual void OnScrollbarsUpdated()
        {           
            if (ScrollbarsUpdated != null)
                ScrollbarsUpdated(this, EventArgs.Empty);
        }

        /// <summary>
        /// Scroll control for display caret
        /// </summary>
        public void DoCaretVisible()
        {
            Invalidate();
            Recalc();
            Point car = PlaceToPoint(Selection.Start);
            car.Offset(-CharWidth, 0);
            DoVisibleRectangle(new Rectangle(car, new Size(2*CharWidth, 2*CharHeight)));
        }

        /// <summary>
        /// Scroll control left
        /// </summary>
        public void ScrollLeft()
        {
            Invalidate();
            HorizontalScroll.Value = 0;
            AutoScrollMinSize -= new Size(1, 0);
            AutoScrollMinSize += new Size(1, 0);
        }

        /// <summary>
        /// Scroll control for display selection area
        /// </summary>
        public void DoSelectionVisible()
        {
            if (LineInfos[Selection.End.iLine].VisibleState != VisibleState.Visible)
                ExpandBlock(Selection.End.iLine);

            if (LineInfos[Selection.Start.iLine].VisibleState != VisibleState.Visible)
                ExpandBlock(Selection.Start.iLine);

            Recalc();
            DoVisibleRectangle(new Rectangle(PlaceToPoint(new Place(0, Selection.End.iLine)),
                                             new Size(2*CharWidth, 2*CharHeight)));

            Point car = PlaceToPoint(Selection.Start);
            Point car2 = PlaceToPoint(Selection.End);
            car.Offset(-CharWidth, -ClientSize.Height/2);
            DoVisibleRectangle(new Rectangle(car, new Size(Math.Abs(car2.X - car.X), ClientSize.Height)));
            //Math.Abs(car2.Y-car.Y) + 2 * CharHeight

            Invalidate();
        }

        /// <summary>
        /// Scroll control for display given range
        /// </summary>
        public void DoRangeVisible(Range range)
        {
            DoRangeVisible(range, false);
        }

        /// <summary>
        /// Scroll control for display given range
        /// </summary>
        public void DoRangeVisible(Range range, bool tryToCentre)
        {
            range = range.Clone();
            range.Normalize();
            range.End = new Place(range.End.iChar,
                                  Math.Min(range.End.iLine, range.Start.iLine + ClientSize.Height/CharHeight));

            if (LineInfos[range.End.iLine].VisibleState != VisibleState.Visible)
                ExpandBlock(range.End.iLine);

            if (LineInfos[range.Start.iLine].VisibleState != VisibleState.Visible)
                ExpandBlock(range.Start.iLine);

            Recalc();
            int h = (1 + range.End.iLine - range.Start.iLine)*CharHeight;
            Point p = PlaceToPoint(new Place(0, range.Start.iLine));
            if (tryToCentre)
            {
                p.Offset(0, -ClientSize.Height/2);
                h = ClientSize.Height;
            }
            DoVisibleRectangle(new Rectangle(p, new Size(2*CharWidth, h)));

            Invalidate();
        }


        protected override void OnKeyUp(KeyEventArgs e)
        {
            base.OnKeyUp(e);

            if (e.KeyCode == Keys.ShiftKey)
                lastModifiers &= ~Keys.Shift;
            if (e.KeyCode == Keys.Alt)
                lastModifiers &= ~Keys.Alt;
            if (e.KeyCode == Keys.ControlKey)
                lastModifiers &= ~Keys.Control;
        }


        bool findCharMode;

        protected override void OnKeyDown(KeyEventArgs e)
        {
            if (middleClickScrollingActivated)
                return;

            base.OnKeyDown(e);

            if (Focused)//??? 
                lastModifiers = e.Modifiers;

            handledChar = false;

            if (e.Handled)
            {
                handledChar = true;
                return;
            }

            if (ProcessKey(e.KeyData))
                return;

            e.Handled = true;

            DoCaretVisible();
            Invalidate();
        }

        protected override bool ProcessDialogKey(Keys keyData)
        {
            if ((keyData & Keys.Alt) > 0)
            {
                if (HotkeysMapping.ContainsKey(keyData))
                {
                    ProcessKey(keyData);
                    return true;
                }
            }

            return base.ProcessDialogKey(keyData);
        }

        static Dictionary<FCTBAction, bool> scrollActions = new Dictionary<FCTBAction, bool>() { { FCTBAction.ScrollDown, true }, { FCTBAction.ScrollUp, true }, { FCTBAction.ZoomOut, true }, { FCTBAction.ZoomIn, true }, { FCTBAction.ZoomNormal, true } };

        /// <summary>
        /// Process control keys
        /// </summary>
        public virtual bool ProcessKey(Keys keyData)
        {
            KeyEventArgs a = new KeyEventArgs(keyData);

            if(a.KeyCode == Keys.Tab && !AcceptsTab)
                 return false;


            if (macrosManager != null)
            if (!HotkeysMapping.ContainsKey(keyData) || (HotkeysMapping[keyData] != FCTBAction.MacroExecute && HotkeysMapping[keyData] != FCTBAction.MacroRecord))
                macrosManager.ProcessKey(keyData);


            if (HotkeysMapping.ContainsKey(keyData))
            {
                var act = HotkeysMapping[keyData];
                DoAction(act);
                if (scrollActions.ContainsKey(act))
                    return true;
                if (keyData == Keys.Tab || keyData == (Keys.Tab | Keys.Shift))
                {
                    handledChar = true;
                    return true;
                }
            }
            else
            {
                //
                if (a.KeyCode == Keys.Alt)
                    return true;

                if ((a.Modifiers & Keys.Control) != 0)
                    return true;

                if ((a.Modifiers & Keys.Alt) != 0)
                {
                    if ((MouseButtons & MouseButtons.Left) != 0)
                        CheckAndChangeSelectionType();
                    return true;
                }

                if (a.KeyCode == Keys.ShiftKey)
                    return true;
            }

            return false;
        }

        private void DoAction(FCTBAction action)
        {
            switch (action)
            {
                case FCTBAction.ZoomIn:
                    ChangeFontSize(2);
                    break;
                case FCTBAction.ZoomOut:
                    ChangeFontSize(-2);
                    break;
                case FCTBAction.ZoomNormal:
                    RestoreFontSize();
                    break;
                case FCTBAction.ScrollDown:
                    DoScrollVertical(1, -1);
                    break;

                case FCTBAction.ScrollUp:
                    DoScrollVertical(1, 1);
                    break;

                case FCTBAction.GoToDialog:
                    ShowGoToDialog();
                    break;

                case FCTBAction.FindDialog:
                    ShowFindDialog();
                    break;

                case FCTBAction.FindChar:
                    findCharMode = true;
                    break;

                case FCTBAction.FindNext:
                    if (findForm == null || findForm.tbFind.Text == "")
                        ShowFindDialog();
                    else
                        findForm.FindNext(findForm.tbFind.Text);
                    break;

                case FCTBAction.ReplaceDialog:
                    ShowReplaceDialog();
                    break;

                case FCTBAction.Copy:
                    Copy();
                    break;

                case FCTBAction.CommentSelected:
                    CommentSelected();
                    break;

                case FCTBAction.Cut:
                    if (!Selection.ReadOnly)
                        Cut();
                    break;

                case FCTBAction.Paste:
                    if (!Selection.ReadOnly)
                        Paste();
                    break;

                case FCTBAction.SelectAll:
                    Selection.SelectAll();
                    break;

                case FCTBAction.Undo:
                    if (!ReadOnly)
                        Undo();
                    break;

                case FCTBAction.Redo:
                    if (!ReadOnly)
                        Redo();
                    break;

                case FCTBAction.LowerCase:
                    if (!Selection.ReadOnly)
                        LowerCase();
                    break;

                case FCTBAction.UpperCase:
                    if (!Selection.ReadOnly)
                        UpperCase();
                    break;

                case FCTBAction.IndentDecrease:
                    if (!Selection.ReadOnly)
                    {
                        var sel = Selection.Clone();
                        if(sel.Start.iLine == sel.End.iLine)
                        {
                            var line = this[sel.Start.iLine];
                            if (sel.Start.iChar == 0 && sel.End.iChar == line.Count)
                                Selection = new Range(this, line.StartSpacesCount, sel.Start.iLine, line.Count, sel.Start.iLine);
                            else
                            if (sel.Start.iChar == line.Count && sel.End.iChar == 0)
                                Selection = new Range(this, line.Count, sel.Start.iLine, line.StartSpacesCount, sel.Start.iLine);
                        }


                        DecreaseIndent();
                    }
                    break;

                case FCTBAction.IndentIncrease:
                    if (!Selection.ReadOnly)
                    {
                        var sel = Selection.Clone();
                        var inverted = sel.Start > sel.End;
                        sel.Normalize();
                        var spaces = this[sel.Start.iLine].StartSpacesCount;
                        if (sel.Start.iLine != sel.End.iLine || //selected several lines
                           (sel.Start.iChar <= spaces && sel.End.iChar == this[sel.Start.iLine].Count) || //selected whole line
                           sel.End.iChar <= spaces)//selected space prefix
                        {
                            IncreaseIndent();
                            if (sel.Start.iLine == sel.End.iLine && !sel.IsEmpty)
                            {
                                Selection = new Range(this, this[sel.Start.iLine].StartSpacesCount, sel.End.iLine, this[sel.Start.iLine].Count, sel.End.iLine); //select whole line
                                if (inverted)
                                    Selection.Inverse();
                            }
                        }
                        else
                            ProcessKey('\t', Keys.None);
                    }
                    break;

                case FCTBAction.AutoIndentChars:
                    if (!Selection.ReadOnly)
                        DoAutoIndentChars(Selection.Start.iLine);
                    break;

                case FCTBAction.NavigateBackward:
                    NavigateBackward();
                    break;

                case FCTBAction.NavigateForward:
                    NavigateForward();
                    break;

                case FCTBAction.UnbookmarkLine:
                    UnbookmarkLine(Selection.Start.iLine);
                    break;

                case FCTBAction.BookmarkLine:
                    BookmarkLine(Selection.Start.iLine);
                    break;

                case FCTBAction.GoNextBookmark:
                    GotoNextBookmark(Selection.Start.iLine);
                    break;

                case FCTBAction.GoPrevBookmark:
                    GotoPrevBookmark(Selection.Start.iLine);
                    break;

                case FCTBAction.ClearWordLeft:
                    if (OnKeyPressing('\b')) //KeyPress event processed key
                        break;
                    if (!Selection.ReadOnly)
                    {
                        if (!Selection.IsEmpty)
                            ClearSelected();
                        Selection.GoWordLeft(true);
                        if (!Selection.ReadOnly)
                            ClearSelected();
                    }
                    OnKeyPressed('\b');
                    break;

                case FCTBAction.ReplaceMode:
                    if (!ReadOnly)
                        isReplaceMode = !isReplaceMode;
                    break;

                case FCTBAction.DeleteCharRight:
                    if (!Selection.ReadOnly)
                    {
                        if (OnKeyPressing((char) 0xff)) //KeyPress event processed key
                            break;
                        if (!Selection.IsEmpty)
                            ClearSelected();
                        else
                        {
                            //if line contains only spaces then delete line
                            if (this[Selection.Start.iLine].StartSpacesCount == this[Selection.Start.iLine].Count)
                                RemoveSpacesAfterCaret();

                            if (!Selection.IsReadOnlyRightChar())
                                if (Selection.GoRightThroughFolded())
                                {
                                    int iLine = Selection.Start.iLine;

                                    InsertChar('\b');

                                    //if removed \n then trim spaces
                                    if (iLine != Selection.Start.iLine && AutoIndent)
                                        if (Selection.Start.iChar > 0)
                                            RemoveSpacesAfterCaret();
                                }
                        }

                        if (AutoIndentChars)
                            DoAutoIndentChars(Selection.Start.iLine);

                        OnKeyPressed((char) 0xff);
                    }
                    break;

                case FCTBAction.ClearWordRight:
                    if (OnKeyPressing((char) 0xff)) //KeyPress event processed key
                        break;
                    if (!Selection.ReadOnly)
                    {
                        if (!Selection.IsEmpty)
                            ClearSelected();
                        Selection.GoWordRight(true);
                        if (!Selection.ReadOnly)
                            ClearSelected();
                    }
                    OnKeyPressed((char) 0xff);
                    break;

                case FCTBAction.GoWordLeft:
                    Selection.GoWordLeft(false);
                    break;

                case FCTBAction.GoWordLeftWithSelection:
                    Selection.GoWordLeft(true);
                    break;

                case FCTBAction.GoLeft:
                    Selection.GoLeft(false);
                    break;

                case FCTBAction.GoLeftWithSelection:
                    Selection.GoLeft(true);
                    break;

                case FCTBAction.GoLeft_ColumnSelectionMode:
                    CheckAndChangeSelectionType();
                    if (Selection.ColumnSelectionMode)
                        Selection.GoLeft_ColumnSelectionMode();
                    Invalidate();
                    break;

                case FCTBAction.GoWordRight:
                    Selection.GoWordRight(false, true);
                    break;

                case FCTBAction.GoWordRightWithSelection:
                    Selection.GoWordRight(true, true);
                    break;

                case FCTBAction.GoRight:
                    Selection.GoRight(false);
                    break;

                case FCTBAction.GoRightWithSelection:
                    Selection.GoRight(true);
                    break;

                case FCTBAction.GoRight_ColumnSelectionMode:
                    CheckAndChangeSelectionType();
                    if (Selection.ColumnSelectionMode)
                        Selection.GoRight_ColumnSelectionMode();
                    Invalidate();
                    break;

                case FCTBAction.GoUp:
                    Selection.GoUp(false);
                    ScrollLeft();
                    break;

                case FCTBAction.GoUpWithSelection:
                    Selection.GoUp(true);
                    ScrollLeft();
                    break;

                case FCTBAction.GoUp_ColumnSelectionMode:
                    CheckAndChangeSelectionType();
                    if (Selection.ColumnSelectionMode)
                        Selection.GoUp_ColumnSelectionMode();
                    Invalidate();
                    break;

                case FCTBAction.MoveSelectedLinesUp:
                    if (!Selection.ColumnSelectionMode)
                        MoveSelectedLinesUp();
                    break;

                case FCTBAction.GoDown:
                    Selection.GoDown(false);
                    ScrollLeft();
                    break;

                case FCTBAction.GoDownWithSelection:
                    Selection.GoDown(true);
                    ScrollLeft();
                    break;

                case FCTBAction.GoDown_ColumnSelectionMode:
                    CheckAndChangeSelectionType();
                    if (Selection.ColumnSelectionMode)
                        Selection.GoDown_ColumnSelectionMode();
                    Invalidate();
                    break;

                case FCTBAction.MoveSelectedLinesDown:
                    if (!Selection.ColumnSelectionMode)
                        MoveSelectedLinesDown();
                    break;
                case FCTBAction.GoPageUp:
                    Selection.GoPageUp(false);
                    ScrollLeft();
                    break;

                case FCTBAction.GoPageUpWithSelection:
                    Selection.GoPageUp(true);
                    ScrollLeft();
                    break;

                case FCTBAction.GoPageDown:
                    Selection.GoPageDown(false);
                    ScrollLeft();
                    break;

                case FCTBAction.GoPageDownWithSelection:
                    Selection.GoPageDown(true);
                    ScrollLeft();
                    break;

                case FCTBAction.GoFirstLine:
                    Selection.GoFirst(false);
                    break;

                case FCTBAction.GoFirstLineWithSelection:
                    Selection.GoFirst(true);
                    break;

                case FCTBAction.GoHome:
                    GoHome(false);
                    ScrollLeft();
                    break;

                case FCTBAction.GoHomeWithSelection:
                    GoHome(true);
                    ScrollLeft();
                    break;

                case FCTBAction.GoLastLine:
                    Selection.GoLast(false);
                    break;

                case FCTBAction.GoLastLineWithSelection:
                    Selection.GoLast(true);
                    break;

                case FCTBAction.GoEnd:
                    Selection.GoEnd(false);
                    break;

                case FCTBAction.GoEndWithSelection:
                    Selection.GoEnd(true);
                    break;

                case FCTBAction.ClearHints:
                    ClearHints();
                    if(MacrosManager != null)
                        MacrosManager.IsRecording = false;
                    break;

                case FCTBAction.MacroRecord:
                    if(MacrosManager != null)
                    {
                        if (MacrosManager.AllowMacroRecordingByUser)
                            MacrosManager.IsRecording = !MacrosManager.IsRecording;
                        if (MacrosManager.IsRecording)
                            MacrosManager.ClearMacros();
                    }
                    break;

                case FCTBAction.MacroExecute:
                    if (MacrosManager != null)
                    {
                        MacrosManager.IsRecording = false;
                        MacrosManager.ExecuteMacros();
                    }
                    break;
                case FCTBAction.CustomAction1 :
                case FCTBAction.CustomAction2 :
                case FCTBAction.CustomAction3 :
                case FCTBAction.CustomAction4 :
                case FCTBAction.CustomAction5 :
                case FCTBAction.CustomAction6 :
                case FCTBAction.CustomAction7 :
                case FCTBAction.CustomAction8 :
                case FCTBAction.CustomAction9 :
                case FCTBAction.CustomAction10:
                case FCTBAction.CustomAction11:
                case FCTBAction.CustomAction12:
                case FCTBAction.CustomAction13:
                case FCTBAction.CustomAction14:
                case FCTBAction.CustomAction15:
                case FCTBAction.CustomAction16:
                case FCTBAction.CustomAction17:
                case FCTBAction.CustomAction18:
                case FCTBAction.CustomAction19:
                case FCTBAction.CustomAction20:
                    OnCustomAction(new CustomActionEventArgs(action));
                    break;
            }
        }

        protected virtual void OnCustomAction(CustomActionEventArgs e)
        {
            if (CustomAction != null)
                CustomAction(this, e);
        }

        Font originalFont;
        public Font OriginalFont { get { return originalFont; } }

        public void RestoreFontSize()
        {
            Zoom = 100;
        }

        /// <summary>
        /// Scrolls to nearest bookmark or to first bookmark
        /// </summary>
        /// <param name="iLine">Current bookmark line index</param>
        public bool GotoNextBookmark(int iLine)
        {
            Bookmark nearestBookmark = null;
            int minNextLineIndex = int.MaxValue;
            Bookmark minBookmark = null;
            int minLineIndex = int.MaxValue;
            foreach (Bookmark bookmark in bookmarks)
            {
                if (bookmark.LineIndex < minLineIndex)
                {
                    minLineIndex = bookmark.LineIndex;
                    minBookmark = bookmark;
                }

                if (bookmark.LineIndex > iLine && bookmark.LineIndex < minNextLineIndex)
                {
                    minNextLineIndex = bookmark.LineIndex;
                    nearestBookmark = bookmark;
                }
            }

            if (nearestBookmark != null)
            {
                nearestBookmark.DoVisible();
                return true;
            }
            else if (minBookmark != null)
            {
                minBookmark.DoVisible();
                return true;
            }

            return false;
        }

        /// <summary>
        /// Scrolls to nearest previous bookmark or to last bookmark
        /// </summary>
        /// <param name="iLine">Current bookmark line index</param>
        public bool GotoPrevBookmark(int iLine)
        {
            Bookmark nearestBookmark = null;
            int maxPrevLineIndex = -1;
            Bookmark maxBookmark = null;
            int maxLineIndex = -1;
            foreach (Bookmark bookmark in bookmarks)
            {
                if (bookmark.LineIndex > maxLineIndex)
                {
                    maxLineIndex = bookmark.LineIndex;
                    maxBookmark = bookmark;
                }

                if (bookmark.LineIndex < iLine && bookmark.LineIndex > maxPrevLineIndex)
                {
                    maxPrevLineIndex = bookmark.LineIndex;
                    nearestBookmark = bookmark;
                }
            }

            if (nearestBookmark != null)
            {
                nearestBookmark.DoVisible();
                return true;
            }
            else if (maxBookmark != null)
            {
                maxBookmark.DoVisible();
                return true;
            }

            return false;
        }

        /// <summary>
        /// Bookmarks line
        /// </summary>
        public virtual void BookmarkLine(int iLine)
        {
            if (!bookmarks.Contains(iLine))
                bookmarks.Add(iLine);
        }

        /// <summary>
        /// Unbookmarks current line
        /// </summary>
        public virtual void UnbookmarkLine(int iLine)
        {
            bookmarks.Remove(iLine);
        }

        /// <summary>
        /// Moves selected lines down
        /// </summary>
        public virtual void MoveSelectedLinesDown()
        {
            Range prevSelection = Selection.Clone();
            Selection.Expand();
            if (!Selection.ReadOnly)
            {
                int iLine = Selection.Start.iLine;
                if (Selection.End.iLine >= LinesCount - 1)
                {
                    Selection = prevSelection;
                    return;
                }
                string text = SelectedText;
                var temp = new List<int>();
                for (int i = Selection.Start.iLine; i <= Selection.End.iLine; i++)
                    temp.Add(i);
                RemoveLines(temp);
                Selection.Start = new Place(GetLineLength(iLine), iLine);
                SelectedText = "\n" + text;
                Selection.Start = new Place(prevSelection.Start.iChar, prevSelection.Start.iLine + 1);
                Selection.End = new Place(prevSelection.End.iChar, prevSelection.End.iLine + 1);
            }else
                Selection = prevSelection;
        }

        /// <summary>
        /// Moves selected lines up
        /// </summary>
        public virtual void MoveSelectedLinesUp()
        {
            Range prevSelection = Selection.Clone();
            Selection.Expand();
            if (!Selection.ReadOnly)
            {
                int iLine = Selection.Start.iLine;
                if (iLine == 0)
                {
                    Selection = prevSelection;
                    return;
                }
                string text = SelectedText;
                var temp = new List<int>();
                for (int i = Selection.Start.iLine; i <= Selection.End.iLine; i++)
                    temp.Add(i);
                RemoveLines(temp);
                Selection.Start = new Place(0, iLine - 1);
                SelectedText = text + "\n";
                Selection.Start = new Place(prevSelection.Start.iChar, prevSelection.Start.iLine - 1);
                Selection.End = new Place(prevSelection.End.iChar, prevSelection.End.iLine - 1);
            }else
                Selection = prevSelection;
        }

        private void GoHome(bool shift)
        {
            Selection.BeginUpdate();
            try
            {
                int iLine = Selection.Start.iLine;
                int spaces = this[iLine].StartSpacesCount;
                if (Selection.Start.iChar <= spaces)
                    Selection.GoHome(shift);
                else
                {
                    Selection.GoHome(shift);
                    for (int i = 0; i < spaces; i++)
                        Selection.GoRight(shift);
                }
            }
            finally
            {
                Selection.EndUpdate();
            }
        }

        /// <summary>
        /// Convert selected text to upper case
        /// </summary>
        public virtual void UpperCase()
        {
            Range old = Selection.Clone();
            SelectedText = SelectedText.ToUpper();
            Selection.Start = old.Start;
            Selection.End = old.End;
        }

        /// <summary>
        /// Convert selected text to lower case
        /// </summary>
        public virtual void LowerCase()
        {
            Range old = Selection.Clone();
            SelectedText = SelectedText.ToLower();
            Selection.Start = old.Start;
            Selection.End = old.End;
        }

        /// <summary>
        /// Convert selected text to title case
        /// </summary>
        public virtual void TitleCase()
        {
            Range old = Selection.Clone();
            SelectedText = Thread.CurrentThread.CurrentCulture.TextInfo.ToTitleCase(SelectedText.ToLower());
            Selection.Start = old.Start;
            Selection.End = old.End;
        }

        /// <summary>
        /// Insert/remove comment prefix into selected lines
        /// </summary>
        public void CommentSelected()
        {
            CommentSelected(CommentPrefix);
        }

        /// <summary>
        /// Insert/remove comment prefix into selected lines
        /// </summary>
        public virtual void CommentSelected(string commentPrefix)
        {
            if (string.IsNullOrEmpty(commentPrefix))
                return;
            Selection.Normalize();
            bool isCommented = lines[Selection.Start.iLine].Text.TrimStart().StartsWith(commentPrefix);
            if (isCommented)
                RemoveLinePrefix(commentPrefix);
            else
                InsertLinePrefix(commentPrefix);
        }

        public void OnKeyPressing(KeyPressEventArgs args)
        {
            if (KeyPressing != null)
                KeyPressing(this, args);
        }

        private bool OnKeyPressing(char c)
        {
            if (findCharMode)
            {
                findCharMode = false;
                FindChar(c);
                return true;
            }
            var args = new KeyPressEventArgs(c);
            OnKeyPressing(args);
            return args.Handled;
        }

        public void OnKeyPressed(char c)
        {
            var args = new KeyPressEventArgs(c);
            if (KeyPressed != null)
                KeyPressed(this, args);
        }

        protected override bool ProcessMnemonic(char charCode)
        {
            if (middleClickScrollingActivated)
                return false;

            if (Focused)
                return ProcessKey(charCode, lastModifiers) || base.ProcessMnemonic(charCode);
            else
                return false;
        }

        const int WM_CHAR = 0x102;

        protected override bool ProcessKeyMessage(ref Message m)
        {
            if (m.Msg == WM_CHAR)
                ProcessMnemonic(Convert.ToChar(m.WParam.ToInt32()));

            return base.ProcessKeyMessage(ref m);
        }

        /// <summary>
        /// Process "real" keys (no control)
        /// </summary>
        public virtual bool ProcessKey(char c, Keys modifiers)
        {
            if (handledChar)
                return true;

            if (macrosManager != null)
                macrosManager.ProcessKey(c, modifiers);
            /*  !!!!
            if (c == ' ')
                return true;*/

            //backspace
            if (c == '\b' && (modifiers == Keys.None || modifiers == Keys.Shift || (modifiers & Keys.Alt) != 0))
            {
                if (ReadOnly || !Enabled)
                    return false;

                if (OnKeyPressing(c))
                    return true;

                if (Selection.ReadOnly)
                    return false;

                if (!Selection.IsEmpty)
                    ClearSelected();
                else
                    if (!Selection.IsReadOnlyLeftChar()) //is not left char readonly?
                        InsertChar('\b');

                if (AutoIndentChars)
                    DoAutoIndentChars(Selection.Start.iLine);

                OnKeyPressed('\b');
                return true;
            }
 
            /* !!!!
            if (c == '\b' && (modifiers & Keys.Alt) != 0)
                return true;*/

            if (char.IsControl(c) && c != '\r' && c != '\t')
                return false;

            if (ReadOnly || !Enabled)
                return false;


            if (modifiers != Keys.None &&
                modifiers != Keys.Shift &&
                modifiers != (Keys.Control | Keys.Alt) && //ALT+CTRL is special chars (AltGr)
                modifiers != (Keys.Shift | Keys.Control | Keys.Alt) && //SHIFT + ALT + CTRL is special chars (AltGr)
                (modifiers != (Keys.Alt) || char.IsLetterOrDigit(c)) //may be ALT+LetterOrDigit is mnemonic code
                )
                return false; //do not process Ctrl+? and Alt+? keys

            char sourceC = c;
            if (OnKeyPressing(sourceC)) //KeyPress event processed key
                return true;

            //
            if (Selection.ReadOnly)
                return false;
            //
            if (c == '\r' && !AcceptsReturn)
                return false;

            //replace \r on \n
            if (c == '\r')
                c = '\n';
            //replace mode? select forward char
            if (IsReplaceMode)
            {
                Selection.GoRight(true);
                Selection.Inverse();
            }
            //insert char
            if (!Selection.ReadOnly)
            {
                if (!DoAutocompleteBrackets(c))
                    InsertChar(c);
            }

            //do autoindent
            if (c == '\n' || AutoIndentExistingLines)
                DoAutoIndentIfNeed();

            if (AutoIndentChars)
                DoAutoIndentChars(Selection.Start.iLine);

            DoCaretVisible();
            Invalidate();

            OnKeyPressed(sourceC);

            return true;
        }

        #region AutoIndentChars

        /// <summary>
        /// Enables AutoIndentChars mode
        /// </summary>
        [Description("Enables AutoIndentChars mode")]
        [DefaultValue(true)]
        public bool AutoIndentChars { get; set; }

        /// <summary>
        /// Regex patterns for AutoIndentChars (one regex per line)
        /// </summary>
        [Description("Regex patterns for AutoIndentChars (one regex per line)")] 
        [Editor( "System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=2.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a" , typeof(UITypeEditor))]
        [DefaultValue(@"^\s*[\w\.]+\s*(?<range>=)\s*(?<range>[^;]+);")]
        public string AutoIndentCharsPatterns { get; set; }

        /// <summary>
        /// Do AutoIndentChars
        /// </summary>
        public void DoAutoIndentChars(int iLine)
        {
            var patterns = AutoIndentCharsPatterns.Split(new char[] {'\r', '\n'}, StringSplitOptions.RemoveEmptyEntries);

            foreach (var pattern in patterns)
            {
                var m = Regex.Match(this[iLine].Text, pattern);
                if (m.Success)
                {
                    DoAutoIndentChars(iLine, new Regex(pattern));
                    break;
                }
            }
        }

        protected void DoAutoIndentChars(int iLine, Regex regex)
        {
            var oldSel = Selection.Clone();

            var captures = new SortedDictionary<int, CaptureCollection>();
            var texts = new SortedDictionary<int, string>();
            var maxCapturesCount = 0;

            var spaces = this[iLine].StartSpacesCount;

            for(var i = iLine; i >= 0; i--)
            {
                if (spaces != this[i].StartSpacesCount)
                    break;

                var text = this[i].Text;
                var m = regex.Match(text);
                if (m.Success)
                {
                    captures[i] = m.Groups["range"].Captures;
                    texts[i] = text;

                    if (captures[i].Count > maxCapturesCount)
                        maxCapturesCount = captures[i].Count;
                }
                else
                    break;
            }

            for (var i = iLine + 1; i < LinesCount; i++)
            {
                if (spaces != this[i].StartSpacesCount)
                    break;

                var text = this[i].Text;
                var m = regex.Match(text);
                if (m.Success)
                {
                    captures[i] = m.Groups["range"].Captures;
                    texts[i] = text;

                    if (captures[i].Count > maxCapturesCount)
                        maxCapturesCount = captures[i].Count;

                }
                else
                    break;
            }

            var changed = new Dictionary<int, bool>();
            var was = false;

            for (int iCapture = maxCapturesCount - 1; iCapture >= 0; iCapture--)
            {
                //find max dist
                var maxDist = 0;
                foreach(var i in captures.Keys)
                {
                    var caps = captures[i];
                    if (caps.Count <= iCapture)
                        continue;
                    var dist = 0;
                    var cap = caps[iCapture];

                    var index = cap.Index;

                    var text = texts[i];
                    while (index > 0 && text[index - 1] == ' ') index--;

                    if (iCapture == 0)
                        dist = index;
                    else
                        dist = index - caps[iCapture - 1].Index - 1;

                    if (dist > maxDist)
                        maxDist = dist;
                }

                //insert whitespaces
                foreach(var i in new List<int>(texts.Keys))
                {
                    if (captures[i].Count <= iCapture)
                        continue;

                    var dist = 0;
                    var cap = captures[i][iCapture];

                    if (iCapture == 0)
                        dist = cap.Index;
                    else
                        dist = cap.Index - captures[i][iCapture - 1].Index - 1;

                    var addSpaces = maxDist - dist + 1;//+1 because min space count is 1

                    if (addSpaces == 0)
                        continue;

                    if (oldSel.Start.iLine == i && oldSel.Start.iChar > cap.Index)
                        oldSel.Start = new Place(oldSel.Start.iChar + addSpaces, i);

                    if (addSpaces > 0)
                        texts[i] = texts[i].Insert(cap.Index, new string(' ', addSpaces));
                    else
                        texts[i] = texts[i].Remove(cap.Index + addSpaces, -addSpaces);
                    
                    changed[i] = true;
                    was = true;
                }
            }

            //insert text
            if (was)
            {
                Selection.BeginUpdate();
                BeginAutoUndo();
                BeginUpdate();

                TextSource.Manager.ExecuteCommand(new SelectCommand(TextSource));

                foreach (var i in texts.Keys)
                if (changed.ContainsKey(i))
                {
                    Selection = new Range(this, 0, i, this[i].Count, i);
                    if(!Selection.ReadOnly)
                        InsertText(texts[i]);
                }

                Selection = oldSel;

                EndUpdate();
                EndAutoUndo();
                Selection.EndUpdate();
            }
        }

        #endregion

        private bool DoAutocompleteBrackets(char c)
        {
            if (AutoCompleteBrackets)
            {
                if (!Selection.ColumnSelectionMode)
                    for (int i = 1; i < autoCompleteBracketsList.Length; i += 2)
                        if (c == autoCompleteBracketsList[i] && c == Selection.CharAfterStart)
                        {
                            Selection.GoRight();
                            return true;
                        }

                for (int i = 0; i < autoCompleteBracketsList.Length; i += 2)
                    if (c == autoCompleteBracketsList[i])
                    {
                        InsertBrackets(autoCompleteBracketsList[i], autoCompleteBracketsList[i + 1]);
                        return true;
                    }
            }
            return false;
        }

        private bool InsertBrackets(char left, char right)
        {
            if (Selection.ColumnSelectionMode)
            {
                var range = Selection.Clone();
                range.Normalize();
                Selection.BeginUpdate();
                BeginAutoUndo();
                Selection = new Range(this, range.Start.iChar, range.Start.iLine, range.Start.iChar, range.End.iLine) { ColumnSelectionMode = true };
                InsertChar(left);
                Selection = new Range(this, range.End.iChar + 1, range.Start.iLine, range.End.iChar + 1, range.End.iLine) { ColumnSelectionMode = true };
                InsertChar(right);
                if (range.IsEmpty)
                    Selection = new Range(this, range.End.iChar + 1, range.Start.iLine, range.End.iChar + 1, range.End.iLine) { ColumnSelectionMode = true };
                EndAutoUndo();
                Selection.EndUpdate();
            }
            else
                if (Selection.IsEmpty)
                {
                    InsertText(left + "" + right);
                    Selection.GoLeft();
                }
                else
                    InsertText(left + SelectedText + right);

            return true;
        }

        /// <summary>
        /// Finds given char after current caret position, moves the caret to found pos.
        /// </summary>
        /// <param name="c"></param>
        protected virtual void FindChar(char c)
        {
            if (c == '\r')
                c = '\n';

            var r = Selection.Clone();
            while (r.GoRight())
            {
                if (r.CharBeforeStart == c)
                {
                    Selection = r;
                    DoCaretVisible();
                    return;
                }
            }
        }

        public virtual void DoAutoIndentIfNeed()
        {
            if (Selection.ColumnSelectionMode)
                return;
            if (AutoIndent)
            {
                DoCaretVisible();
                int needSpaces = CalcAutoIndent(Selection.Start.iLine);
                if (this[Selection.Start.iLine].AutoIndentSpacesNeededCount != needSpaces)
                {
                    DoAutoIndent(Selection.Start.iLine);
                    this[Selection.Start.iLine].AutoIndentSpacesNeededCount = needSpaces;
                }
            }
        }

        private void RemoveSpacesAfterCaret()
        {
            if (!Selection.IsEmpty)
                return;
            Place end = Selection.Start;
            while (Selection.CharAfterStart == ' ')
                Selection.GoRight(true);
            ClearSelected();
        }

        /// <summary>
        /// Inserts autoindent's spaces in the line
        /// </summary>
        public virtual void DoAutoIndent(int iLine)
        {
            if (Selection.ColumnSelectionMode)
                return;
            Place oldStart = Selection.Start;
            //
            int needSpaces = CalcAutoIndent(iLine);
            //
            int spaces = lines[iLine].StartSpacesCount;
            int needToInsert = needSpaces - spaces;
            if (needToInsert < 0)
                needToInsert = -Math.Min(-needToInsert, spaces);
            //insert start spaces
            if (needToInsert == 0)
                return;
            Selection.Start = new Place(0, iLine);
            if (needToInsert > 0)
                InsertText(new String(' ', needToInsert));
            else
            {
                Selection.Start = new Place(0, iLine);
                Selection.End = new Place(-needToInsert, iLine);
                ClearSelected();
            }

            Selection.Start = new Place(Math.Min(lines[iLine].Count, Math.Max(0, oldStart.iChar + needToInsert)), iLine);
        }

        /// <summary>
        /// Returns needed start space count for the line
        /// </summary>
        public virtual int CalcAutoIndent(int iLine)
        {
            if (iLine < 0 || iLine >= LinesCount) return 0;


            EventHandler<AutoIndentEventArgs> calculator = AutoIndentNeeded;
            if (calculator == null)
                if (Language != Language.Custom && SyntaxHighlighter != null)
                    calculator = SyntaxHighlighter.AutoIndentNeeded;
                else
                    calculator = CalcAutoIndentShiftByCodeFolding;

            int needSpaces = 0;

            var stack = new Stack<AutoIndentEventArgs>();
            //calc indent for previous lines, find stable line
            int i;
            for (i = iLine - 1; i >= 0; i--)
            {
                var args = new AutoIndentEventArgs(i, lines[i].Text, i > 0 ? lines[i - 1].Text : "", TabLength, 0);
                calculator(this, args);
                stack.Push(args);
                if (args.Shift == 0 && args.AbsoluteIndentation == 0 && args.LineText.Trim() != "")
                    break;
            }
            int indent = lines[i >= 0 ? i : 0].StartSpacesCount;
            while (stack.Count != 0)
            {
                var arg = stack.Pop();
                if (arg.AbsoluteIndentation != 0)
                    indent = arg.AbsoluteIndentation + arg.ShiftNextLines;
                else
                    indent += arg.ShiftNextLines;
            }
            //clalc shift for current line
            var a = new AutoIndentEventArgs(iLine, lines[iLine].Text, iLine > 0 ? lines[iLine - 1].Text : "", TabLength, indent);
            calculator(this, a);
            needSpaces = a.AbsoluteIndentation + a.Shift;

            return needSpaces;
        }

        internal virtual void CalcAutoIndentShiftByCodeFolding(object sender, AutoIndentEventArgs args)
        {
            //inset TAB after start folding marker
            if (string.IsNullOrEmpty(lines[args.iLine].FoldingEndMarker) &&
                !string.IsNullOrEmpty(lines[args.iLine].FoldingStartMarker))
            {
                args.ShiftNextLines = TabLength;
                return;
            }
            //remove TAB before end folding marker
            if (!string.IsNullOrEmpty(lines[args.iLine].FoldingEndMarker) &&
                string.IsNullOrEmpty(lines[args.iLine].FoldingStartMarker))
            {
                args.Shift = -TabLength;
                args.ShiftNextLines = -TabLength;
                return;
            }
        }


        protected int GetMinStartSpacesCount(int fromLine, int toLine)
        {
            if (fromLine > toLine)
                return 0;

            int result = int.MaxValue;
            for (int i = fromLine; i <= toLine; i++)
            {
                int count = lines[i].StartSpacesCount;
                if (count < result)
                    result = count;
            }

            return result;
        }

        protected int GetMaxStartSpacesCount(int fromLine, int toLine)
        {
            if (fromLine > toLine)
                return 0;

            int result = 0;
            for (int i = fromLine; i <= toLine; i++)
            {
                int count = lines[i].StartSpacesCount;
                if (count > result)
                    result = count;
            }

            return result;
        }

        /// <summary>
        /// Undo last operation
        /// </summary>
        public virtual void Undo()
        {
            lines.Manager.Undo();
            DoCaretVisible();
            Invalidate();
        }

        /// <summary>
        /// Redo
        /// </summary>
        public virtual void Redo()
        {
            lines.Manager.Redo();
            DoCaretVisible();
            Invalidate();
        }

        protected override bool IsInputKey(Keys keyData)
        {
            if (keyData == Keys.Tab && !AcceptsTab)
                return false;
            if (keyData == Keys.Enter && !AcceptsReturn)
                return false;

            if ((keyData & Keys.Alt) == Keys.None)
            {
                Keys keys = keyData & Keys.KeyCode;
                if (keys == Keys.Return)
                    return true;
            }

            if ((keyData & Keys.Alt) != Keys.Alt)
            {
                switch ((keyData & Keys.KeyCode))
                {
                    case Keys.Prior:
                    case Keys.Next:
                    case Keys.End:
                    case Keys.Home:
                    case Keys.Left:
                    case Keys.Right:
                    case Keys.Up:
                    case Keys.Down:
                        return true;

                    case Keys.Escape:
                        return false;

                    case Keys.Tab:
                        return (keyData & Keys.Control) == Keys.None;
                }
            }

            return base.IsInputKey(keyData);
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            if (BackBrush == null)
                base.OnPaintBackground(e);
            else
                e.Graphics.FillRectangle(BackBrush, ClientRectangle);
        }

        /// <summary>
        /// Draws text to given Graphics
        /// </summary>
        /// <param name="gr"></param>
        /// <param name="start">Start place of drawing text</param>
        /// <param name="size">Size of drawing</param>
        public void DrawText(Graphics gr, Place start, Size size)
        {
            if (needRecalc)
                Recalc();

            if (needRecalcFoldingLines)
                RecalcFoldingLines();

            var startPoint = PlaceToPoint(start);
            var startY = startPoint.Y + VerticalScroll.Value;
            var startX = startPoint.X + HorizontalScroll.Value - LeftIndent - Paddings.Left;
            int firstChar = start.iChar;
            int lastChar = (startX + size.Width) / CharWidth;

            var startLine = start.iLine;
            //draw text
            for (int iLine = startLine; iLine < lines.Count; iLine++)
            {
                Line line = lines[iLine];
                LineInfo lineInfo = LineInfos[iLine];
                //
                if (lineInfo.startY > startY + size.Height)
                    break;
                if (lineInfo.startY + lineInfo.WordWrapStringsCount * CharHeight < startY)
                    continue;
                if (lineInfo.VisibleState == VisibleState.Hidden)
                    continue;

                int y = lineInfo.startY - startY;
                //
                gr.SmoothingMode = SmoothingMode.None;
                //draw line background
                if (lineInfo.VisibleState == VisibleState.Visible)
                    if (line.BackgroundBrush != null)
                        gr.FillRectangle(line.BackgroundBrush, new Rectangle(0, y, size.Width, CharHeight * lineInfo.WordWrapStringsCount));
                //
                gr.SmoothingMode = SmoothingMode.AntiAlias;

                //draw wordwrap strings of line
                for (int iWordWrapLine = 0; iWordWrapLine < lineInfo.WordWrapStringsCount; iWordWrapLine++)
                {
                    y = lineInfo.startY + iWordWrapLine * CharHeight - startY;
                    //indent 
                    var indent = iWordWrapLine == 0 ? 0 : lineInfo.wordWrapIndent * CharWidth;
                    //draw chars
                    DrawLineChars(gr, firstChar, lastChar, iLine, iWordWrapLine, -startX + indent, y);
                }
            }
        }

        /// <summary>
        /// Draw control
        /// </summary>
        protected override void OnPaint(PaintEventArgs e)
        {
            if (needRecalc)
                Recalc();

            if (needRecalcFoldingLines)
                RecalcFoldingLines();
#if debug
            var sw = Stopwatch.StartNew();
#endif
            visibleMarkers.Clear();
            e.Graphics.SmoothingMode = SmoothingMode.None;
            //
            var servicePen = new Pen(ServiceLinesColor);
            Brush changedLineBrush = new SolidBrush(ChangedLineColor);
            Brush indentBrush = new SolidBrush(IndentBackColor);
            Brush paddingBrush = new SolidBrush(PaddingBackColor);
            Brush currentLineBrush =
                new SolidBrush(Color.FromArgb(CurrentLineColor.A == 255 ? 50 : CurrentLineColor.A, CurrentLineColor));
            //draw padding area
            var textAreaRect = TextAreaRect;
            //top
            e.Graphics.FillRectangle(paddingBrush, 0, -VerticalScroll.Value, ClientSize.Width, Math.Max(0, Paddings.Top - 1));
            //bottom
            e.Graphics.FillRectangle(paddingBrush, 0, textAreaRect.Bottom, ClientSize.Width,ClientSize.Height);
            //right
            e.Graphics.FillRectangle(paddingBrush, textAreaRect.Right, 0, ClientSize.Width, ClientSize.Height);
            //left
            e.Graphics.FillRectangle(paddingBrush, LeftIndentLine, 0, LeftIndent - LeftIndentLine - 1, ClientSize.Height);
            if (HorizontalScroll.Value <= Paddings.Left)
                e.Graphics.FillRectangle(paddingBrush, LeftIndent - HorizontalScroll.Value - 2, 0,
                                         Math.Max(0, Paddings.Left - 1), ClientSize.Height);
            //
            int textWidth = textAreaRect.Width;
            //draw indent area
            e.Graphics.FillRectangle(indentBrush, 0, 0, LeftIndentLine, ClientSize.Height);
            if (LeftIndent > minLeftIndent)
                e.Graphics.DrawLine(servicePen, LeftIndentLine, 0, LeftIndentLine, ClientSize.Height);
            //draw preferred line width
            if (PreferredLineWidth > 0)
                e.Graphics.DrawLine(servicePen,
                                    new Point(
                                        LeftIndent + Paddings.Left + PreferredLineWidth*CharWidth -
                                        HorizontalScroll.Value + 1, textAreaRect.Top + 1),
                                    new Point(
                                        LeftIndent + Paddings.Left + PreferredLineWidth*CharWidth -
                                        HorizontalScroll.Value + 1, textAreaRect.Bottom - 1));

            //draw text area border
            DrawTextAreaBorder(e.Graphics);
            //
            int firstChar = (Math.Max(0, HorizontalScroll.Value - Paddings.Left))/CharWidth;
            int lastChar = (HorizontalScroll.Value + ClientSize.Width)/CharWidth;
            //
            var x = LeftIndent + Paddings.Left - HorizontalScroll.Value;
            if (x < LeftIndent)
                firstChar++;
            //create dictionary of bookmarks
            var bookmarksByLineIndex = new Dictionary<int, Bookmark>();
            foreach (Bookmark item in bookmarks)
                bookmarksByLineIndex[item.LineIndex] = item;
            //
            int startLine = YtoLineIndex(VerticalScroll.Value);
            int iLine;

            e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;

            //draw text
            for (iLine = startLine; iLine < lines.Count; iLine++)
            {
                Line line = lines[iLine];
                LineInfo lineInfo = LineInfos[iLine];
                //
                if (lineInfo.startY > VerticalScroll.Value + ClientSize.Height)
                    break;
                if (lineInfo.startY + lineInfo.WordWrapStringsCount*CharHeight < VerticalScroll.Value)
                    continue;
                if (lineInfo.VisibleState == VisibleState.Hidden)
                    continue;

                int y = lineInfo.startY - VerticalScroll.Value;
                //
                e.Graphics.SmoothingMode = SmoothingMode.None;
                //draw line background
                if (lineInfo.VisibleState == VisibleState.Visible)
                    if (line.BackgroundBrush != null)
                        e.Graphics.FillRectangle(line.BackgroundBrush,
                                                 new Rectangle(textAreaRect.Left, y, textAreaRect.Width,
                                                               CharHeight*lineInfo.WordWrapStringsCount));
                //draw current line background
                if (CurrentLineColor != Color.Transparent && iLine == Selection.Start.iLine)
                    if (Selection.IsEmpty)
                        e.Graphics.FillRectangle(currentLineBrush,
                                                 new Rectangle(textAreaRect.Left, y, textAreaRect.Width, CharHeight));
                //draw changed line marker
                if (ChangedLineColor != Color.Transparent && line.IsChanged)
                    e.Graphics.FillRectangle(changedLineBrush,
                                             new RectangleF(-10, y, LeftIndent - minLeftIndent - 2 + 10, CharHeight + 1));
                //
                e.Graphics.SmoothingMode = SmoothingMode.AntiAlias;
                //
                //draw bookmark
                if (bookmarksByLineIndex.ContainsKey(iLine))
                    bookmarksByLineIndex[iLine].Paint(e.Graphics,
                                                      new Rectangle(LeftIndent, y, Width,
                                                                    CharHeight*lineInfo.WordWrapStringsCount));
                //OnPaintLine event
                if (lineInfo.VisibleState == VisibleState.Visible)
                    OnPaintLine(new PaintLineEventArgs(iLine,
                                                       new Rectangle(LeftIndent, y, Width,
                                                                     CharHeight*lineInfo.WordWrapStringsCount),
                                                       e.Graphics, e.ClipRectangle));
                //draw line number
                if (ShowLineNumbers)
                    using (var lineNumberBrush = new SolidBrush(LineNumberColor))
                        e.Graphics.DrawString((iLine + lineNumberStartValue).ToString(), Font, lineNumberBrush,
                                              new RectangleF(-10, y, LeftIndent - minLeftIndent - 2 + 10, CharHeight),
                                              new StringFormat(StringFormatFlags.DirectionRightToLeft));
                //create markers
                if (lineInfo.VisibleState == VisibleState.StartOfHiddenBlock)
                    visibleMarkers.Add(new ExpandFoldingMarker(iLine, new Rectangle(LeftIndentLine - 4, y + CharHeight/2 - 3, 8, 8)));

                if (!string.IsNullOrEmpty(line.FoldingStartMarker) && lineInfo.VisibleState == VisibleState.Visible &&
                    string.IsNullOrEmpty(line.FoldingEndMarker))
                        visibleMarkers.Add(new CollapseFoldingMarker(iLine, new Rectangle(LeftIndentLine - 4, y + CharHeight/2 - 3, 8, 8)));

                if (lineInfo.VisibleState == VisibleState.Visible && !string.IsNullOrEmpty(line.FoldingEndMarker) &&
                    string.IsNullOrEmpty(line.FoldingStartMarker))
                    e.Graphics.DrawLine(servicePen, LeftIndentLine, y + CharHeight*lineInfo.WordWrapStringsCount - 1,
                                        LeftIndentLine + 4, y + CharHeight*lineInfo.WordWrapStringsCount - 1);
                //draw wordwrap strings of line
                for (int iWordWrapLine = 0; iWordWrapLine < lineInfo.WordWrapStringsCount; iWordWrapLine++)
                {
                    y = lineInfo.startY + iWordWrapLine*CharHeight - VerticalScroll.Value;
                    //indent
                    var indent = iWordWrapLine == 0 ? 0 : lineInfo.wordWrapIndent * CharWidth;
                    //draw chars
                    DrawLineChars(e.Graphics, firstChar, lastChar, iLine, iWordWrapLine, x + indent, y);
                }
            }

            int endLine = iLine - 1;

            //draw folding lines
            if (ShowFoldingLines)
                DrawFoldingLines(e, startLine, endLine);

            //draw column selection
            if (Selection.ColumnSelectionMode)
                if (SelectionStyle.BackgroundBrush is SolidBrush)
                {
                    Color color = ((SolidBrush) SelectionStyle.BackgroundBrush).Color;
                    Point p1 = PlaceToPoint(Selection.Start);
                    Point p2 = PlaceToPoint(Selection.End);
                    using (var pen = new Pen(color))
                        e.Graphics.DrawRectangle(pen,
                                                 Rectangle.FromLTRB(Math.Min(p1.X, p2.X) - 1, Math.Min(p1.Y, p2.Y),
                                                                    Math.Max(p1.X, p2.X),
                                                                    Math.Max(p1.Y, p2.Y) + CharHeight));
                }
            //draw brackets highlighting
            if (BracketsStyle != null && leftBracketPosition != null && rightBracketPosition != null)
            {
                BracketsStyle.Draw(e.Graphics, PlaceToPoint(leftBracketPosition.Start), leftBracketPosition);
                BracketsStyle.Draw(e.Graphics, PlaceToPoint(rightBracketPosition.Start), rightBracketPosition);
            }
            if (BracketsStyle2 != null && leftBracketPosition2 != null && rightBracketPosition2 != null)
            {
                BracketsStyle2.Draw(e.Graphics, PlaceToPoint(leftBracketPosition2.Start), leftBracketPosition2);
                BracketsStyle2.Draw(e.Graphics, PlaceToPoint(rightBracketPosition2.Start), rightBracketPosition2);
            }
            //
            e.Graphics.SmoothingMode = SmoothingMode.None;
            //draw folding indicator
            if ((startFoldingLine >= 0 || endFoldingLine >= 0) && Selection.Start == Selection.End)
                if (endFoldingLine < LineInfos.Count)
                {
                    //folding indicator
                    int startFoldingY = (startFoldingLine >= 0 ? LineInfos[startFoldingLine].startY : 0) -
                                        VerticalScroll.Value + CharHeight/2;
                    int endFoldingY = (endFoldingLine >= 0
                                           ? LineInfos[endFoldingLine].startY +
                                             (LineInfos[endFoldingLine].WordWrapStringsCount - 1)*CharHeight
                                           : TextHeight + CharHeight) - VerticalScroll.Value + CharHeight;

                    using (var indicatorPen = new Pen(Color.FromArgb(100, FoldingIndicatorColor), 4))
                        e.Graphics.DrawLine(indicatorPen, LeftIndent - 5, startFoldingY, LeftIndent - 5, endFoldingY);
                }
            //draw hint's brackets
            PaintHintBrackets(e.Graphics);
            //draw markers
            DrawMarkers(e, servicePen);
            //draw caret
            Point car = PlaceToPoint(Selection.Start);
            var caretHeight = CharHeight - lineInterval;
            car.Offset(0, lineInterval / 2);

            if ((Focused || IsDragDrop) && car.X >= LeftIndent && CaretVisible)
            {
                int carWidth = (IsReplaceMode || WideCaret) ? CharWidth : 1;
                if (WideCaret)
                {
                    using (var brush = new SolidBrush(CaretColor))
                        e.Graphics.FillRectangle(brush, car.X, car.Y, carWidth, caretHeight + 1);
                }
                else
                    using (var pen = new Pen(CaretColor))
                        e.Graphics.DrawLine(pen, car.X, car.Y, car.X, car.Y + caretHeight);

                var caretRect = new Rectangle(HorizontalScroll.Value + car.X, VerticalScroll.Value + car.Y, carWidth, caretHeight + 1);

                if (CaretBlinking)
                if (prevCaretRect != caretRect || !ShowScrollBars)
                {
                    NativeMethodsWrapper.CreateCaret(Handle, 0, carWidth, caretHeight + 1);
                    NativeMethodsWrapper.SetCaretPos(car.X, car.Y);
                    NativeMethodsWrapper.ShowCaret(Handle);
                }

                prevCaretRect = caretRect;
            }
            else
            {
                NativeMethodsWrapper.HideCaret(Handle);
                prevCaretRect = Rectangle.Empty;
            }

            //draw disabled mask
            if (!Enabled)
                using (var brush = new SolidBrush(DisabledColor))
                    e.Graphics.FillRectangle(brush, ClientRectangle);

            if (MacrosManager.IsRecording)
                DrawRecordingHint(e.Graphics);

            if (middleClickScrollingActivated)
                DrawMiddleClickScrolling(e.Graphics);

            //dispose resources
            servicePen.Dispose();
            changedLineBrush.Dispose();
            indentBrush.Dispose();
            currentLineBrush.Dispose();
            paddingBrush.Dispose();
            //
#if debug
            sw.Stop();
            Console.WriteLine("OnPaint: "+ sw.ElapsedMilliseconds);
#endif
            //
            base.OnPaint(e);
        }

        private void DrawMarkers(PaintEventArgs e, Pen servicePen)
        {
            foreach (VisualMarker m in visibleMarkers)
            {
                if(m is CollapseFoldingMarker)
                    using(var bk = new SolidBrush(ServiceColors.CollapseMarkerBackColor))
                    using(var fore = new Pen(ServiceColors.CollapseMarkerForeColor))
                    using(var border = new Pen(ServiceColors.CollapseMarkerBorderColor))
                        (m as CollapseFoldingMarker).Draw(e.Graphics, border, bk, fore);
                else
                if (m is ExpandFoldingMarker)
                    using (var bk = new SolidBrush(ServiceColors.ExpandMarkerBackColor))
                    using (var fore = new Pen(ServiceColors.ExpandMarkerForeColor))
                    using (var border = new Pen(ServiceColors.ExpandMarkerBorderColor))
                        (m as ExpandFoldingMarker).Draw(e.Graphics, border, bk, fore);
                else
                    m.Draw(e.Graphics, servicePen);
            }
        }

        private Rectangle prevCaretRect;

        private void DrawRecordingHint(Graphics graphics)
        {
            const int w = 75;
            const int h = 13;
            var rect = new Rectangle(ClientRectangle.Right - w, ClientRectangle.Bottom - h, w, h);
            var iconRect = new Rectangle(-h/2 + 3, -h/2 + 3, h - 7, h - 7);
            var state = graphics.Save();
            graphics.SmoothingMode = SmoothingMode.HighQuality;
            graphics.TranslateTransform(rect.Left + h/2, rect.Top + h/2);
            graphics.RotateTransform(180 * (DateTime.Now.Millisecond/1000f));
            using (var pen = new Pen(Color.Red, 2))
            {
                graphics.DrawArc(pen, iconRect, 0, 90);
                graphics.DrawArc(pen, iconRect, 180, 90);
            }
            graphics.DrawEllipse(Pens.Red, iconRect);
            graphics.Restore(state);
            using (var font = new Font(FontFamily.GenericSansSerif, 8f))
                graphics.DrawString("Recording...", font, Brushes.Red, new PointF(rect.Left + h, rect.Top));
            System.Threading.Timer tm = null;
            tm = new System.Threading.Timer(
                (o) => {
                    Invalidate(rect);
                    tm.Dispose();
                }, null, 200, System.Threading.Timeout.Infinite);
        }

        private void DrawTextAreaBorder(Graphics graphics)
        {
            if (TextAreaBorder == TextAreaBorderType.None)
                return;

            var rect = TextAreaRect;

            if (TextAreaBorder == TextAreaBorderType.Shadow)
            {
                const int shadowSize = 4;
                var rBottom = new Rectangle(rect.Left + shadowSize, rect.Bottom, rect.Width - shadowSize, shadowSize);
                var rCorner = new Rectangle(rect.Right, rect.Bottom, shadowSize, shadowSize);
                var rRight = new Rectangle(rect.Right, rect.Top + shadowSize, shadowSize, rect.Height - shadowSize);

                using (var brush = new SolidBrush(Color.FromArgb(80, TextAreaBorderColor)))
                {
                    graphics.FillRectangle(brush, rBottom);
                    graphics.FillRectangle(brush, rRight);
                    graphics.FillRectangle(brush, rCorner);
                }
            }

            using(Pen pen = new Pen(TextAreaBorderColor))
                graphics.DrawRectangle(pen, rect);
        }

        private void PaintHintBrackets(Graphics gr)
        {
            foreach (Hint hint in hints)
            {
                Range r = hint.Range.Clone();
                r.Normalize();
                Point p1 = PlaceToPoint(r.Start);
                Point p2 = PlaceToPoint(r.End);
                if (GetVisibleState(r.Start.iLine) != VisibleState.Visible ||
                    GetVisibleState(r.End.iLine) != VisibleState.Visible)
                    continue;

                using (var pen = new Pen(hint.BorderColor))
                {
                    pen.DashStyle = DashStyle.Dash;
                    if (r.IsEmpty)
                    {
                        p1.Offset(1, -1);
                        gr.DrawLines(pen, new[] {p1, new Point(p1.X, p1.Y + charHeight + 2)});
                    }
                    else
                    {
                        p1.Offset(-1, -1);
                        p2.Offset(1, -1);
                        gr.DrawLines(pen,
                                     new[]
                                         {
                                             new Point(p1.X + CharWidth/2, p1.Y), p1,
                                             new Point(p1.X, p1.Y + charHeight + 2),
                                             new Point(p1.X + CharWidth/2, p1.Y + charHeight + 2)
                                         });
                        gr.DrawLines(pen,
                                     new[]
                                         {
                                             new Point(p2.X - CharWidth/2, p2.Y), p2,
                                             new Point(p2.X, p2.Y + charHeight + 2),
                                             new Point(p2.X - CharWidth/2, p2.Y + charHeight + 2)
                                         });
                    }
                }
            }
        }

        protected virtual void DrawFoldingLines(PaintEventArgs e, int startLine, int endLine)
        {
            e.Graphics.SmoothingMode = SmoothingMode.None;
            using (var pen = new Pen(Color.FromArgb(200, ServiceLinesColor)) {DashStyle = DashStyle.Dot})
                foreach (var iLine in foldingPairs)
                    if (iLine.Key < endLine && iLine.Value > startLine)
                    {
                        Line line = lines[iLine.Key];
                        int y = LineInfos[iLine.Key].startY - VerticalScroll.Value + CharHeight;
                        y += y%2;

                        int y2;

                        if (iLine.Value >= LinesCount)
                            y2 = LineInfos[LinesCount - 1].startY + CharHeight - VerticalScroll.Value;
                        else if (LineInfos[iLine.Value].VisibleState == VisibleState.Visible)
                        {
                            int d = 0;
                            int spaceCount = line.StartSpacesCount;
                            if (lines[iLine.Value].Count <= spaceCount || lines[iLine.Value][spaceCount].c == ' ')
                                d = CharHeight;
                            y2 = LineInfos[iLine.Value].startY - VerticalScroll.Value + d;
                        }
                        else
                            continue;

                        int x = LeftIndent + Paddings.Left + line.StartSpacesCount*CharWidth - HorizontalScroll.Value;
                        if (x >= LeftIndent + Paddings.Left)
                            e.Graphics.DrawLine(pen, x, y >= 0 ? y : 0, x,
                                                y2 < ClientSize.Height ? y2 : ClientSize.Height);
                    }
        }

        private void DrawLineChars(Graphics gr, int firstChar, int lastChar, int iLine, int iWordWrapLine, int startX,
                                   int y)
        {
            Line line = lines[iLine];
            LineInfo lineInfo = LineInfos[iLine];
            int from = lineInfo.GetWordWrapStringStartPosition(iWordWrapLine);
            int to = lineInfo.GetWordWrapStringFinishPosition(iWordWrapLine, line);

            lastChar = Math.Min(to - from, lastChar);

            gr.SmoothingMode = SmoothingMode.AntiAlias;

            //folded block ?
            if (lineInfo.VisibleState == VisibleState.StartOfHiddenBlock)
            {
                //rendering by FoldedBlockStyle
                FoldedBlockStyle.Draw(gr, new Point(startX + firstChar*CharWidth, y),
                                      new Range(this, from + firstChar, iLine, from + lastChar + 1, iLine));
            }
            else
            {
                //render by custom styles
                StyleIndex currentStyleIndex = StyleIndex.None;
                int iLastFlushedChar = firstChar - 1;

                for (int iChar = firstChar; iChar <= lastChar; iChar++)
                {
                    StyleIndex style = line[from + iChar].style;
                    if (currentStyleIndex != style)
                    {
                        FlushRendering(gr, currentStyleIndex,
                                       new Point(startX + (iLastFlushedChar + 1)*CharWidth, y),
                                       new Range(this, from + iLastFlushedChar + 1, iLine, from + iChar, iLine));
                        iLastFlushedChar = iChar - 1;
                        currentStyleIndex = style;
                    }
                }
                FlushRendering(gr, currentStyleIndex, new Point(startX + (iLastFlushedChar + 1)*CharWidth, y),
                               new Range(this, from + iLastFlushedChar + 1, iLine, from + lastChar + 1, iLine));
            }

            //draw selection
            if (SelectionHighlightingForLineBreaksEnabled  && iWordWrapLine == lineInfo.WordWrapStringsCount - 1) lastChar++;//draw selection for CR
            if (!Selection.IsEmpty && lastChar >= firstChar)
            {
                gr.SmoothingMode = SmoothingMode.None;
                var textRange = new Range(this, from + firstChar, iLine, from + lastChar + 1, iLine);
                textRange = Selection.GetIntersectionWith(textRange);
                if (textRange != null && SelectionStyle != null)
                {
                    SelectionStyle.Draw(gr, new Point(startX + (textRange.Start.iChar - from)*CharWidth, 1 + y),
                                        textRange);
                }
            }
        }

        private void FlushRendering(Graphics gr, StyleIndex styleIndex, Point pos, Range range)
        {
            if (range.End > range.Start)
            {
                int mask = 1;
                bool hasTextStyle = false;
                for (int i = 0; i < Styles.Length; i++)
                {
                    if (Styles[i] != null && ((int) styleIndex & mask) != 0)
                    {
                        Style style = Styles[i];
                        bool isTextStyle = style is TextStyle;
                        if (!hasTextStyle || !isTextStyle || AllowSeveralTextStyleDrawing)
                            //cancelling secondary rendering by TextStyle
                            style.Draw(gr, pos, range); //rendering
                        hasTextStyle |= isTextStyle;
                    }
                    mask = mask << 1;
                }
                //draw by default renderer
                if (!hasTextStyle)
                    DefaultStyle.Draw(gr, pos, range);
            }
        }

        protected override void OnEnter(EventArgs e)
        {
            base.OnEnter(e);
            mouseIsDrag = false;
            mouseIsDragDrop = false;
            draggedRange = null;
        }

        protected override void OnMouseUp(MouseEventArgs e)
        {
            base.OnMouseUp(e);
            isLineSelect = false;

            if (e.Button == System.Windows.Forms.MouseButtons.Left)
            {
                if (mouseIsDragDrop)
                    OnMouseClickText(e);
            }
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            base.OnMouseDown(e);

            if (middleClickScrollingActivated)
            {
                DeactivateMiddleClickScrollingMode();
                mouseIsDrag = false;
                if(e.Button == System.Windows.Forms.MouseButtons.Middle)
                    RestoreScrollsAfterMiddleClickScrollingMode();
                return;
            }

            MacrosManager.IsRecording = false;

            Select();
            ActiveControl = null;

            if (e.Button == MouseButtons.Left)
            {
                VisualMarker marker = FindVisualMarkerForPoint(e.Location);
                //click on marker
                if (marker != null)
                {
                    mouseIsDrag = false;
                    mouseIsDragDrop = false;
                    draggedRange = null;
                    OnMarkerClick(e, marker);
                    return;
                }

                mouseIsDrag = true;
                mouseIsDragDrop = false;
                draggedRange = null;
                isLineSelect = (e.Location.X < LeftIndentLine);

                if (!isLineSelect)
                {
                    var p = PointToPlace(e.Location);

                    if (e.Clicks == 2)
                    {
                        mouseIsDrag = false;
                        mouseIsDragDrop = false;
                        draggedRange = null;

                        SelectWord(p);
                        return;
                    }

                    if (Selection.IsEmpty || !Selection.Contains(p) || this[p.iLine].Count <= p.iChar || ReadOnly)
                        OnMouseClickText(e);
                    else
                    {
                        mouseIsDragDrop = true;
                        mouseIsDrag = false;
                    }
                }
                else
                {
                    CheckAndChangeSelectionType();

                    Selection.BeginUpdate();
                    //select whole line
                    int iLine = PointToPlaceSimple(e.Location).iLine;
                    lineSelectFrom = iLine;
                    Selection.Start = new Place(0, iLine);
                    Selection.End = new Place(GetLineLength(iLine), iLine);
                    Selection.EndUpdate();
                    Invalidate();
                }
            }
            else
            if (e.Button == MouseButtons.Middle)
            {
                ActivateMiddleClickScrollingMode(e);
            }
        }

        private void OnMouseClickText(MouseEventArgs e)
        {
            //click on text
            Place oldEnd = Selection.End;
            Selection.BeginUpdate();

            if (Selection.ColumnSelectionMode)
            {
                Selection.Start = PointToPlaceSimple(e.Location);
                Selection.ColumnSelectionMode = true;
            }
            else
            {
                if (VirtualSpace)
                    Selection.Start = PointToPlaceSimple(e.Location);
                else
                    Selection.Start = PointToPlace(e.Location);
            }

            if ((lastModifiers & Keys.Shift) != 0)
                Selection.End = oldEnd;

            CheckAndChangeSelectionType();

            Selection.EndUpdate();
            Invalidate();
            return;
        }

        protected virtual void CheckAndChangeSelectionType()
        {
            //change selection type to ColumnSelectionMode
            if ((ModifierKeys & Keys.Alt) != 0 && !WordWrap)
            {
                Selection.ColumnSelectionMode = true;
            }
            else
            //change selection type to Range
            {
                Selection.ColumnSelectionMode = false;
            }
        }

        protected override void OnMouseWheel(MouseEventArgs e)
        {
            Invalidate();

            if (lastModifiers == Keys.Control)
            {
                ChangeFontSize(2 * Math.Sign(e.Delta));
                if (!MonoUtility.IsLinux)
                {
                    ((HandledMouseEventArgs)e).Handled = true;
                }
            }
            else
            if (VerticalScroll.Visible || !ShowScrollBars)
            {
                //base.OnMouseWheel(e);

                // Determine scoll offset
                int mouseWheelScrollLinesSetting = GetControlPanelWheelScrollLinesValue();

                DoScrollVertical(mouseWheelScrollLinesSetting, e.Delta);

                if (!MonoUtility.IsLinux)
                {
                    ((HandledMouseEventArgs)e).Handled = true;
                }
            }

            DeactivateMiddleClickScrollingMode();
        }

        private void DoScrollVertical(int countLines, int direction)
        {
            if (VerticalScroll.Visible || !ShowScrollBars)
            {
                int numberOfVisibleLines = ClientSize.Height/CharHeight;

                int offset;
                if ((countLines == -1) || (countLines > numberOfVisibleLines))
                    offset = CharHeight*numberOfVisibleLines;
                else
                    offset = CharHeight*countLines;

                var newScrollPos = VerticalScroll.Value - Math.Sign(direction)*offset;

                var ea =
                    new ScrollEventArgs(direction > 0 ? ScrollEventType.SmallDecrement : ScrollEventType.SmallIncrement,
                                        VerticalScroll.Value,
                                        newScrollPos,
                                        ScrollOrientation.VerticalScroll);

                OnScroll(ea);
            }
        }

        /// <summary>
        /// Gets the value for the system control panel mouse wheel scroll settings.
        /// The value returns the number of lines that shall be scolled if the user turns the mouse wheet one step.
        /// </summary>
        /// <remarks>
        /// This methods gets the "WheelScrollLines" value our from the registry key "HKEY_CURRENT_USER\Control Panel\Desktop".
        /// If the value of this option is 0, the screen will not scroll when the mouse wheel is turned.
        /// If the value of this option is -1 or is greater than the number of lines visible in the window,
        /// the screen will scroll up or down by one page.
        /// </remarks>
        /// <returns>
        /// Number of lines to scrol l when the mouse wheel is turned
        /// </returns>
        private static int GetControlPanelWheelScrollLinesValue()
        {
            try
            {
                using (RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Control Panel\Desktop", false))
                {
                    return Convert.ToInt32(key.GetValue("WheelScrollLines"));
                }
            }
            catch
            {
                // Use default value
                return 1;
            }
        }


        public void ChangeFontSize(int step)
        {
            var points = Font.SizeInPoints;
            using (var gr = Graphics.FromHwnd(Handle))
            {
                var dpi = gr.DpiY;
                var newPoints = points + step * 72f / dpi;
                if(newPoints < 1f) return;
                var k = newPoints / originalFont.SizeInPoints;
                 Zoom = (int)(100 * k);
            }
        }

        /// <summary>
        /// Zooming (in percentages)
        /// </summary>
        [Browsable(false)]
        public int Zoom 
        {
            get { return zoom; }
            set {
                if(value >= 30 && value <= 500) {
                   zoom = value;
                   DoZoom(zoom / 100f);
                   OnZoomChanged();
                }
            }
        }

        protected virtual void OnZoomChanged()
        {
            if (ZoomChanged != null)
                ZoomChanged(this, EventArgs.Empty);
        }

        private void DoZoom(float koeff)
        {
            //remmber first displayed line
            var iLine = YtoLineIndex(VerticalScroll.Value);
            //
            var points = originalFont.SizeInPoints;
            points *= koeff;

            if (points < 1f || points > 300f) return;

            var oldFont = Font;
            SetFont(new Font(Font.FontFamily, points, Font.Style, GraphicsUnit.Point));
            oldFont.Dispose();

            NeedRecalc(true);

            //restore first displayed line
            if (iLine < LinesCount)
                VerticalScroll.Value = Math.Min(VerticalScroll.Maximum, LineInfos[iLine].startY - Paddings.Top);
            UpdateScrollbars();
            //
            Invalidate();
            OnVisibleRangeChanged();
        }

        protected override void OnMouseLeave(EventArgs e)
        {
            base.OnMouseLeave(e);

            CancelToolTip();
        }

        protected Range draggedRange;

        protected override void OnMouseMove(MouseEventArgs e)
        {
            base.OnMouseMove(e);

            if (middleClickScrollingActivated)
                return;

            if (lastMouseCoord != e.Location)
            {
                //restart tooltip timer
                CancelToolTip();
                timer3.Start();
            }
            lastMouseCoord = e.Location;

            if (e.Button == MouseButtons.Left && mouseIsDragDrop)
            {
                draggedRange = Selection.Clone();
                DoDragDrop(SelectedText, DragDropEffects.Copy);
                draggedRange = null;
                return;
            }

            if (e.Button == MouseButtons.Left && mouseIsDrag)
            {
                Place place;
                if (Selection.ColumnSelectionMode || VirtualSpace)
                    place = PointToPlaceSimple(e.Location);
                else
                    place = PointToPlace(e.Location);

                if (isLineSelect)
                {
                    Selection.BeginUpdate();

                    int iLine = place.iLine;
                    if (iLine < lineSelectFrom)
                    {
                        Selection.Start = new Place(0, iLine);
                        Selection.End = new Place(GetLineLength(lineSelectFrom), lineSelectFrom);
                    }
                    else
                    {
                        Selection.Start = new Place(GetLineLength(iLine), iLine);
                        Selection.End = new Place(0, lineSelectFrom);
                    }

                    Selection.EndUpdate();
                    DoCaretVisible();
                    HorizontalScroll.Value = 0;
                    UpdateScrollbars();
                    Invalidate();
                }
                else if (place != Selection.Start)
                {
                    Place oldEnd = Selection.End;
                    Selection.BeginUpdate();
                    if (Selection.ColumnSelectionMode)
                    {
                        Selection.Start = place;
                        Selection.ColumnSelectionMode = true;
                    }
                    else
                        Selection.Start = place;
                    Selection.End = oldEnd;
                    Selection.EndUpdate();
                    DoCaretVisible();
                    Invalidate();
                    return;
                }
            }

            VisualMarker marker = FindVisualMarkerForPoint(e.Location);
            if (marker != null)
                base.Cursor = marker.Cursor;
            else
            {
                if (e.Location.X < LeftIndentLine || isLineSelect)
                    base.Cursor = Cursors.Arrow;
                else
                    base.Cursor = defaultCursor;
            }
        }

        private void CancelToolTip()
        {
            timer3.Stop();
            if (ToolTip != null && !string.IsNullOrEmpty(ToolTip.GetToolTip(this)))
            {
                ToolTip.Hide(this);
                ToolTip.SetToolTip(this, null);
            }
        }

        protected override void OnMouseDoubleClick(MouseEventArgs e)
        {
            base.OnMouseDoubleClick(e);

            var m = FindVisualMarkerForPoint(e.Location);
            if (m != null)
                OnMarkerDoubleClick(m);
        }

        private void SelectWord(Place p)
        {
            int fromX = p.iChar;
            int toX = p.iChar;

            for (int i = p.iChar; i < lines[p.iLine].Count; i++)
            {
                char c = lines[p.iLine][i].c;
                if (char.IsLetterOrDigit(c) || c == '_')
                    toX = i + 1;
                else
                    break;
            }

            for (int i = p.iChar - 1; i >= 0; i--)
            {
                char c = lines[p.iLine][i].c;
                if (char.IsLetterOrDigit(c) || c == '_')
                    fromX = i;
                else
                    break;
            }

            Selection = new Range(this, toX, p.iLine, fromX, p.iLine);
        }

        private int YtoLineIndex(int y)
        {
            int i = LineInfos.BinarySearch(new LineInfo(-10), new LineYComparer(y));
            i = i < 0 ? -i - 2 : i;
            if (i < 0) return 0;
            if (i > lines.Count - 1) return lines.Count - 1;
            return i;
        }

        /// <summary>
        /// Gets nearest line and char position from coordinates
        /// </summary>
        /// <param name="point">Point</param>
        /// <returns>Line and char position</returns>
        public Place PointToPlace(Point point)
        {
            #if debug
            var sw = Stopwatch.StartNew();
            #endif
            point.Offset(HorizontalScroll.Value, VerticalScroll.Value);
            point.Offset(-LeftIndent - Paddings.Left, 0);
            int iLine = YtoLineIndex(point.Y);
            if (iLine < 0)
                return Place.Empty;

            int y = 0;

            for (; iLine < lines.Count; iLine++)
            {
                y = LineInfos[iLine].startY + LineInfos[iLine].WordWrapStringsCount*CharHeight;
                if (y > point.Y && LineInfos[iLine].VisibleState == VisibleState.Visible)
                    break;
            }
            if (iLine >= lines.Count)
                iLine = lines.Count - 1;
            if (LineInfos[iLine].VisibleState != VisibleState.Visible)
                iLine = FindPrevVisibleLine(iLine);
            //
            int iWordWrapLine = LineInfos[iLine].WordWrapStringsCount;
            do
            {
                iWordWrapLine--;
                y -= CharHeight;
            } while (y > point.Y);
            if (iWordWrapLine < 0) iWordWrapLine = 0;
            //
            int start = LineInfos[iLine].GetWordWrapStringStartPosition(iWordWrapLine);
            int finish = LineInfos[iLine].GetWordWrapStringFinishPosition(iWordWrapLine, lines[iLine]);
            var x = (int) Math.Round((float) point.X/CharWidth);
            if (iWordWrapLine > 0)
                x -= LineInfos[iLine].wordWrapIndent;

            x = x < 0 ? start : start + x;
            if (x > finish)
                x = finish + 1;
            if (x > lines[iLine].Count)
                x = lines[iLine].Count;

#if debug
            Console.WriteLine("PointToPlace: " + sw.ElapsedMilliseconds);
#endif

            return new Place(x, iLine);
        }

        private Place PointToPlaceSimple(Point point)
        {
            point.Offset(HorizontalScroll.Value, VerticalScroll.Value);
            point.Offset(-LeftIndent - Paddings.Left, 0);
            int iLine = YtoLineIndex(point.Y);
            var x = (int) Math.Round((float) point.X/CharWidth);
            if (x < 0) x = 0;
            return new Place(x, iLine);
        }

        /// <summary>
        /// Gets nearest absolute text position for given point
        /// </summary>
        /// <param name="point">Point</param>
        /// <returns>Position</returns>
        public int PointToPosition(Point point)
        {
            return PlaceToPosition(PointToPlace(point));
        }

        /// <summary>
        /// Fires TextChanging event
        /// </summary>
        public virtual void OnTextChanging(ref string text)
        {
            ClearBracketsPositions();

            if (TextChanging != null)
            {
                var args = new TextChangingEventArgs {InsertingText = text};
                TextChanging(this, args);
                text = args.InsertingText;
                if (args.Cancel)
                    text = string.Empty;
            }
        }

        public virtual void OnTextChanging()
        {
            string temp = null;
            OnTextChanging(ref temp);
        }

        /// <summary>
        /// Fires TextChanged event
        /// </summary>
        public virtual void OnTextChanged()
        {
            var r = new Range(this);
            r.SelectAll();
            OnTextChanged(new TextChangedEventArgs(r));
        }

        /// <summary>
        /// Fires TextChanged event
        /// </summary>
        public virtual void OnTextChanged(int fromLine, int toLine)
        {
            var r = new Range(this);
            r.Start = new Place(0, Math.Min(fromLine, toLine));
            r.End = new Place(lines[Math.Max(fromLine, toLine)].Count, Math.Max(fromLine, toLine));
            OnTextChanged(new TextChangedEventArgs(r));
        }

        /// <summary>
        /// Fires TextChanged event
        /// </summary>
        public virtual void OnTextChanged(Range r)
        {
            OnTextChanged(new TextChangedEventArgs(r));
        }

        /// <summary>
        /// Call this method before multiple text changing
        /// </summary>
        public void BeginUpdate()
        {
            if (updating == 0)
                updatingRange = null;
            updating++;
        }

        /// <summary>
        /// Call this method after multiple text changing
        /// </summary>
        public void EndUpdate()
        {
            updating--;

            if (updating == 0 && updatingRange != null)
            {
                updatingRange.Expand();
                OnTextChanged(updatingRange);
            }
        }


        /// <summary>
        /// Fires TextChanged event
        /// </summary>
        protected virtual void OnTextChanged(TextChangedEventArgs args)
        {
            //
            args.ChangedRange.Normalize();
            //
            if (updating > 0)
            {
                if (updatingRange == null)
                    updatingRange = args.ChangedRange.Clone();
                else
                {
                    if (updatingRange.Start.iLine > args.ChangedRange.Start.iLine)
                        updatingRange.Start = new Place(0, args.ChangedRange.Start.iLine);
                    if (updatingRange.End.iLine < args.ChangedRange.End.iLine)
                        updatingRange.End = new Place(lines[args.ChangedRange.End.iLine].Count,
                                                      args.ChangedRange.End.iLine);
                    updatingRange = updatingRange.GetIntersectionWith(Range);
                }
                return;
            }
            //
#if debug
            var sw = Stopwatch.StartNew();
            #endif
            CancelToolTip();
            ClearHints();
            IsChanged = true;
            TextVersion++;
            MarkLinesAsChanged(args.ChangedRange);
            ClearFoldingState(args.ChangedRange);
            //
            if (wordWrap)
                RecalcWordWrap(args.ChangedRange.Start.iLine, args.ChangedRange.End.iLine);
            //
            base.OnTextChanged(args);

            //dalayed event stuffs
            if (delayedTextChangedRange == null)
                delayedTextChangedRange = args.ChangedRange.Clone();
            else
                delayedTextChangedRange = delayedTextChangedRange.GetUnionWith(args.ChangedRange);

            needRiseTextChangedDelayed = true;
            ResetTimer(timer2);
            //
            OnSyntaxHighlight(args);
            //
            if (TextChanged != null)
                TextChanged(this, args);
            //
            if (BindingTextChanged != null)
                BindingTextChanged(this, EventArgs.Empty);
            //
            base.OnTextChanged(EventArgs.Empty);
            //
#if debug
            Console.WriteLine("OnTextChanged: " + sw.ElapsedMilliseconds);
#endif

            OnVisibleRangeChanged();
        }

        /// <summary>
        /// Clears folding state for range of text
        /// </summary>
        private void ClearFoldingState(Range range)
        {
            for (int iLine = range.Start.iLine; iLine <= range.End.iLine; iLine++)
                if (iLine >= 0 && iLine < lines.Count)
                    FoldedBlocks.Remove(this[iLine].UniqueId);
        }


        private void MarkLinesAsChanged(Range range)
        {
            for (int iLine = range.Start.iLine; iLine <= range.End.iLine; iLine++)
                if (iLine >= 0 && iLine < lines.Count)
                    lines[iLine].IsChanged = true;
        }

        /// <summary>
        /// Fires SelectionChanged event
        /// </summary>
        public virtual void OnSelectionChanged()
        {
#if debug
            var sw = Stopwatch.StartNew();
            #endif
            //find folding markers for highlighting
            if (HighlightFoldingIndicator)
                HighlightFoldings();
            //
            needRiseSelectionChangedDelayed = true;
            ResetTimer(timer);

            if (SelectionChanged != null)
                SelectionChanged(this, new EventArgs());

#if debug
            Console.WriteLine("OnSelectionChanged: "+ sw.ElapsedMilliseconds);
#endif
        }

        //find folding markers for highlighting
        private void HighlightFoldings()
        {
            if (LinesCount == 0)
                return;
            //
            int prevStartFoldingLine = startFoldingLine;
            int prevEndFoldingLine = endFoldingLine;
            //
            startFoldingLine = -1;
            endFoldingLine = -1;
            int counter = 0;
            for (int i = Selection.Start.iLine; i >= Math.Max(Selection.Start.iLine - maxLinesForFolding, 0); i--)
            {
                bool hasStartMarker = lines.LineHasFoldingStartMarker(i);
                bool hasEndMarker = lines.LineHasFoldingEndMarker(i);

                if (hasEndMarker && hasStartMarker)
                    continue;

                if (hasStartMarker)
                {
                    counter--;
                    if (counter == -1) //found start folding
                    {
                        startFoldingLine = i;
                        break;
                    }
                }
                if (hasEndMarker && i != Selection.Start.iLine)
                    counter++;
            }
            if (startFoldingLine >= 0)
            {
                //find end of block
                endFoldingLine = FindEndOfFoldingBlock(startFoldingLine, maxLinesForFolding);
                if (endFoldingLine == startFoldingLine)
                    endFoldingLine = -1;
            }

            if (startFoldingLine != prevStartFoldingLine || endFoldingLine != prevEndFoldingLine)
                OnFoldingHighlightChanged();
        }

        protected virtual void OnFoldingHighlightChanged()
        {
            if (FoldingHighlightChanged != null)
                FoldingHighlightChanged(this, EventArgs.Empty);
        }

        protected override void OnGotFocus(EventArgs e)
        {
            SetAsCurrentTB();
            base.OnGotFocus(e);
            Invalidate();
        }

        protected override void OnLostFocus(EventArgs e)
        {
            lastModifiers = Keys.None;
            DeactivateMiddleClickScrollingMode();
            base.OnLostFocus(e);
            Invalidate();
        }

        /// <summary>
        /// Gets absolute text position from line and char position
        /// </summary>
        /// <param name="point">Line and char position</param>
        /// <returns>Point of char</returns>
        public int PlaceToPosition(Place point)
        {
            if (point.iLine < 0 || point.iLine >= lines.Count ||
                point.iChar >= lines[point.iLine].Count + Environment.NewLine.Length)
                return -1;

            int result = 0;
            for (int i = 0; i < point.iLine; i++)
                result += lines[i].Count + Environment.NewLine.Length;
            result += point.iChar;

            return result;
        }

        /// <summary>
        /// Gets line and char position from absolute text position
        /// </summary>
        public Place PositionToPlace(int pos)
        {
            if (pos < 0)
                return new Place(0, 0);

            for (int i = 0; i < lines.Count; i++)
            {
                int lineLength = lines[i].Count + Environment.NewLine.Length;
                if (pos < lines[i].Count)
                    return new Place(pos, i);
                if (pos < lineLength)
                    return new Place(lines[i].Count, i);

                pos -= lineLength;
            }

            if (lines.Count > 0)
                return new Place(lines[lines.Count - 1].Count, lines.Count - 1);
            else
                return new Place(0, 0);
            //throw new ArgumentOutOfRangeException("Position out of range");
        }

        /// <summary>
        /// Gets absolute char position from char position
        /// </summary>
        public Point PositionToPoint(int pos)
        {
            return PlaceToPoint(PositionToPlace(pos));
        }

        /// <summary>
        /// Gets point for given line and char position
        /// </summary>
        /// <param name="place">Line and char position</param>
        /// <returns>Coordiantes</returns>
        public Point PlaceToPoint(Place place)
        {
            if (place.iLine >= LineInfos.Count)
                return new Point();
            int y = LineInfos[place.iLine].startY;
            //
            int iWordWrapIndex = LineInfos[place.iLine].GetWordWrapStringIndex(place.iChar);
            y += iWordWrapIndex*CharHeight;
            int x = (place.iChar - LineInfos[place.iLine].GetWordWrapStringStartPosition(iWordWrapIndex))*CharWidth;
            if(iWordWrapIndex > 0 )
                x += LineInfos[place.iLine].wordWrapIndent * CharWidth;
            //
            y = y - VerticalScroll.Value;
            x = LeftIndent + Paddings.Left + x - HorizontalScroll.Value;

            return new Point(x, y);
        }

        /// <summary>
        /// Get range of text
        /// </summary>
        /// <param name="fromPos">Absolute start position</param>
        /// <param name="toPos">Absolute finish position</param>
        /// <returns>Range</returns>
        public Range GetRange(int fromPos, int toPos)
        {
            var sel = new Range(this);
            sel.Start = PositionToPlace(fromPos);
            sel.End = PositionToPlace(toPos);
            return sel;
        }

        /// <summary>
        /// Get range of text
        /// </summary>
        /// <param name="fromPlace">Line and char position</param>
        /// <param name="toPlace">Line and char position</param>
        /// <returns>Range</returns>
        public Range GetRange(Place fromPlace, Place toPlace)
        {
            return new Range(this, fromPlace, toPlace);
        }

        /// <summary>
        /// Finds ranges for given regex pattern
        /// </summary>
        /// <param name="regexPattern">Regex pattern</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRanges(string regexPattern)
        {
            var range = new Range(this);
            range.SelectAll();
            //
            foreach (Range r in range.GetRanges(regexPattern, RegexOptions.None))
                yield return r;
        }

        /// <summary>
        /// Finds ranges for given regex pattern
        /// </summary>
        /// <param name="regexPattern">Regex pattern</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRanges(string regexPattern, RegexOptions options)
        {
            var range = new Range(this);
            range.SelectAll();
            //
            foreach (Range r in range.GetRanges(regexPattern, options))
                yield return r;
        }

        /// <summary>
        /// Get text of given line
        /// </summary>
        /// <param name="iLine">Line index</param>
        /// <returns>Text</returns>
        public string GetLineText(int iLine)
        {
            if (iLine < 0 || iLine >= lines.Count)
                throw new ArgumentOutOfRangeException("Line index out of range");
            var sb = new StringBuilder(lines[iLine].Count);
            foreach (Char c in lines[iLine])
                sb.Append(c.c);
            return sb.ToString();
        }

        /// <summary>
        /// Exapnds folded block
        /// </summary>
        /// <param name="iLine">Start line</param>
        public virtual void ExpandFoldedBlock(int iLine)
        {
            if (iLine < 0 || iLine >= lines.Count)
                throw new ArgumentOutOfRangeException("Line index out of range");
            //find all hidden lines afetr iLine
            int end = iLine;
            for (; end < LinesCount - 1; end++)
            {
                if (LineInfos[end + 1].VisibleState != VisibleState.Hidden)
                    break;
            }

            ExpandBlock(iLine, end);

            FoldedBlocks.Remove(this[iLine].UniqueId);//remove folded state for this line
            AdjustFolding();
        }

        /// <summary>
        /// Collapse folding blocks using FoldedBlocks dictionary.
        /// </summary>
        public virtual void AdjustFolding()
        {
            //collapse folded blocks
            for (int iLine = 0; iLine < LinesCount; iLine++)
                if (LineInfos[iLine].VisibleState == VisibleState.Visible)
                    if (FoldedBlocks.ContainsKey(this[iLine].UniqueId))
                        CollapseFoldingBlock(iLine);
        }


        /// <summary>
        /// Expand collapsed block
        /// </summary>
        public virtual void ExpandBlock(int fromLine, int toLine)
        {
            int from = Math.Min(fromLine, toLine);
            int to = Math.Max(fromLine, toLine);
            for (int i = from; i <= to; i++)
                SetVisibleState(i, VisibleState.Visible);
            needRecalc = true;

            Invalidate();
            OnVisibleRangeChanged();
        }

        /// <summary>
        /// Expand collapsed block
        /// </summary>
        /// <param name="iLine">Any line inside collapsed block</param>
        public void ExpandBlock(int iLine)
        {
            if (LineInfos[iLine].VisibleState == VisibleState.Visible)
                return;

            for (int i = iLine; i < LinesCount; i++)
                if (LineInfos[i].VisibleState == VisibleState.Visible)
                    break;
                else
                {
                    SetVisibleState(i, VisibleState.Visible);
                    needRecalc = true;
                }

            for (int i = iLine - 1; i >= 0; i--)
                if (LineInfos[i].VisibleState == VisibleState.Visible)
                    break;
                else
                {
                    SetVisibleState(i, VisibleState.Visible);
                    needRecalc = true;
                }

            Invalidate();
            OnVisibleRangeChanged();
        }

        /// <summary>
        /// Collapses all folding blocks
        /// </summary>
        public virtual void CollapseAllFoldingBlocks()
        {
            for (int i = 0; i < LinesCount; i++)
                if (lines.LineHasFoldingStartMarker(i))
                {
                    int iFinish = FindEndOfFoldingBlock(i);
                    if (iFinish >= 0)
                    {
                        CollapseBlock(i, iFinish);
                        i = iFinish;
                    }
                }

            OnVisibleRangeChanged();
            UpdateScrollbars();
        }

        /// <summary>
        /// Exapnds all folded blocks
        /// </summary>
        /// <param name="iLine"></param>
        public virtual void ExpandAllFoldingBlocks()
        {
            for (int i = 0; i < LinesCount; i++)
                SetVisibleState(i, VisibleState.Visible);

            FoldedBlocks.Clear();

            OnVisibleRangeChanged();
            Invalidate();
            UpdateScrollbars();
        }

        /// <summary>
        /// Collapses folding block
        /// </summary>
        /// <param name="iLine">Start folding line</param>
        public virtual void CollapseFoldingBlock(int iLine)
        {
            if (iLine < 0 || iLine >= lines.Count)
                throw new ArgumentOutOfRangeException("Line index out of range");
            if (string.IsNullOrEmpty(lines[iLine].FoldingStartMarker))
                throw new ArgumentOutOfRangeException("This line is not folding start line");
            //find end of block
            int i = FindEndOfFoldingBlock(iLine);
            //collapse
            if (i >= 0)
            {
                CollapseBlock(iLine, i);
                var id = this[iLine].UniqueId;
                FoldedBlocks[id] = id; //add folded state for line
            }
        }

        private int FindEndOfFoldingBlock(int iStartLine)
        {
            return FindEndOfFoldingBlock(iStartLine, int.MaxValue);
        }

        protected virtual int FindEndOfFoldingBlock(int iStartLine, int maxLines)
        {
            //find end of block
            int i;
            string marker = lines[iStartLine].FoldingStartMarker;
            var stack = new Stack<string>();

            switch (FindEndOfFoldingBlockStrategy)
            {
                case FindEndOfFoldingBlockStrategy.Strategy1:
                    for (i = iStartLine /*+1*/; i < LinesCount; i++)
                    {
                        if (lines.LineHasFoldingStartMarker(i))
                            stack.Push(lines[i].FoldingStartMarker);

                        if (lines.LineHasFoldingEndMarker(i))
                        {
                            string m = lines[i].FoldingEndMarker;
                            while (stack.Count > 0 && stack.Pop() != m) ;
                            if (stack.Count == 0)
                                return i;
                        }

                        maxLines--;
                        if (maxLines < 0)
                            return i;
                    }
                    break;

                case FindEndOfFoldingBlockStrategy.Strategy2:
                    for (i = iStartLine /*+1*/; i < LinesCount; i++)
                    {
                        if (lines.LineHasFoldingEndMarker(i))
                        {
                            string m = lines[i].FoldingEndMarker;
                            while (stack.Count > 0 && stack.Pop() != m) ;
                            if (stack.Count == 0)
                                return i;
                        }

                        if (lines.LineHasFoldingStartMarker(i))
                            stack.Push(lines[i].FoldingStartMarker);

                        maxLines--;
                        if (maxLines < 0)
                            return i;
                    }
                    break;
            }

            //return -1;
            return LinesCount - 1;
        }

        /// <summary>
        /// Start foilding marker for the line
        /// </summary>
        public string GetLineFoldingStartMarker(int iLine)
        {
            if (lines.LineHasFoldingStartMarker(iLine))
                return lines[iLine].FoldingStartMarker;
            return null;
        }

        /// <summary>
        /// End foilding marker for the line
        /// </summary>
        public string GetLineFoldingEndMarker(int iLine)
        {
            if (lines.LineHasFoldingEndMarker(iLine))
                return lines[iLine].FoldingEndMarker;
            return null;
        }

        protected virtual void RecalcFoldingLines()
        {
            if (!needRecalcFoldingLines)
                return;
            needRecalcFoldingLines = false;
            if (!ShowFoldingLines)
                return;

            foldingPairs.Clear();
            //
            Range range = VisibleRange;
            int startLine = Math.Max(range.Start.iLine - maxLinesForFolding, 0);
            int endLine = Math.Min(range.End.iLine + maxLinesForFolding, Math.Max(range.End.iLine, LinesCount - 1));
            var stack = new Stack<int>();
            for (int i = startLine; i <= endLine; i++)
            {
                bool hasStartMarker = lines.LineHasFoldingStartMarker(i);
                bool hasEndMarker = lines.LineHasFoldingEndMarker(i);

                if (hasEndMarker && hasStartMarker)
                    continue;

                if (hasStartMarker)
                {
                    stack.Push(i);
                }
                if (hasEndMarker)
                {
                    string m = lines[i].FoldingEndMarker;
                    while (stack.Count > 0)
                    {
                        int iStartLine = stack.Pop();
                        foldingPairs[iStartLine] = i;
                        if (m == lines[iStartLine].FoldingStartMarker)
                            break;
                    }
                }
            }

            while (stack.Count > 0)
                foldingPairs[stack.Pop()] = endLine + 1;
        }

        /// <summary>
        /// Collapse text block
        /// </summary>
        public virtual void CollapseBlock(int fromLine, int toLine)
        {
            int from = Math.Min(fromLine, toLine);
            int to = Math.Max(fromLine, toLine);
            if (from == to)
                return;

            //find first non empty line
            for (; from <= to; from++)
            {
                if (GetLineText(from).Trim().Length > 0)
                {
                    //hide lines
                    for (int i = from + 1; i <= to; i++)
                        SetVisibleState(i, VisibleState.Hidden);
                    SetVisibleState(from, VisibleState.StartOfHiddenBlock);
                    Invalidate();
                    break;
                }
            }
            //Move caret outside
            from = Math.Min(fromLine, toLine);
            to = Math.Max(fromLine, toLine);
            int newLine = FindNextVisibleLine(to);
            if (newLine == to)
                newLine = FindPrevVisibleLine(from);
            Selection.Start = new Place(0, newLine);
            //
            needRecalc = true;
            Invalidate();
            OnVisibleRangeChanged();
        }


        internal int FindNextVisibleLine(int iLine)
        {
            if (iLine >= lines.Count - 1) return iLine;
            int old = iLine;
            do
                iLine++; while (iLine < lines.Count - 1 && LineInfos[iLine].VisibleState != VisibleState.Visible);

            if (LineInfos[iLine].VisibleState != VisibleState.Visible)
                return old;
            else
                return iLine;
        }


        internal int FindPrevVisibleLine(int iLine)
        {
            if (iLine <= 0) return iLine;
            int old = iLine;
            do
                iLine--; while (iLine > 0 && LineInfos[iLine].VisibleState != VisibleState.Visible);

            if (LineInfos[iLine].VisibleState != VisibleState.Visible)
                return old;
            else
                return iLine;
        }

        private VisualMarker FindVisualMarkerForPoint(Point p)
        {
            foreach (VisualMarker m in visibleMarkers)
                if (m.rectangle.Contains(p))
                    return m;
            return null;
        }

        /// <summary>
        /// Insert TAB into front of seletcted lines.
        /// </summary>
        public virtual void IncreaseIndent()
        {
            if (Selection.Start == Selection.End)
            {
                if (!Selection.ReadOnly)
                {
                    Selection.Start = new Place(this[Selection.Start.iLine].StartSpacesCount, Selection.Start.iLine);
                    //insert tab as spaces
                    int spaces = TabLength - (Selection.Start.iChar % TabLength);
                    //replace mode? select forward chars
                    if (IsReplaceMode)
                    {
                        for (int i = 0; i < spaces; i++)
                            Selection.GoRight(true);
                        Selection.Inverse();
                    }

                    InsertText(new String(' ', spaces));
                }
                return;
            }

            bool carretAtEnd = (Selection.Start > Selection.End) && !Selection.ColumnSelectionMode;

            int startChar = 0; // Only move selection when in 'ColumnSelectionMode'
            if (Selection.ColumnSelectionMode)
                startChar = Math.Min(Selection.End.iChar, Selection.Start.iChar);

            BeginUpdate();
            Selection.BeginUpdate();
            lines.Manager.BeginAutoUndoCommands();

            var old = Selection.Clone();
            lines.Manager.ExecuteCommand(new SelectCommand(TextSource));//remember selection

            //
            Selection.Normalize();
            Range currentSelection = this.Selection.Clone();
            int from = Selection.Start.iLine;
            int to = Selection.End.iLine;

            if (!Selection.ColumnSelectionMode)
                if (Selection.End.iChar == 0) to--;

            for (int i = from; i <= to; i++)
            {
                if (lines[i].Count == 0) continue;
                Selection.Start = new Place(startChar, i);
                lines.Manager.ExecuteCommand(new InsertTextCommand(TextSource, new String(' ', TabLength)));
            }

            // Restore selection
            if (Selection.ColumnSelectionMode == false)
            {
                int newSelectionStartCharacterIndex = currentSelection.Start.iChar + this.TabLength;
                int newSelectionEndCharacterIndex = currentSelection.End.iChar + (currentSelection.End.iLine == to?this.TabLength : 0);
                this.Selection.Start = new Place(newSelectionStartCharacterIndex, currentSelection.Start.iLine);
                this.Selection.End = new Place(newSelectionEndCharacterIndex, currentSelection.End.iLine);
            }
            else
            {
                Selection = old;
            }
            lines.Manager.EndAutoUndoCommands();

            if (carretAtEnd)
                Selection.Inverse();

            needRecalc = true;
            Selection.EndUpdate();
            EndUpdate();
            Invalidate();
        }

        /// <summary>
        /// Remove TAB from front of seletcted lines.
        /// </summary>
        public virtual void DecreaseIndent()
        {
            if (Selection.Start.iLine == Selection.End.iLine)
            {
                DecreaseIndentOfSingleLine();
                return;
            }

            int startCharIndex = 0;
            if (Selection.ColumnSelectionMode)
                startCharIndex = Math.Min(Selection.End.iChar, Selection.Start.iChar);

            BeginUpdate();
            Selection.BeginUpdate();
            lines.Manager.BeginAutoUndoCommands();
            var old = Selection.Clone();
            lines.Manager.ExecuteCommand(new SelectCommand(TextSource));//remember selection

            // Remember current selection infos
            Range currentSelection = this.Selection.Clone();
            Selection.Normalize();
            int from = Selection.Start.iLine;
            int to = Selection.End.iLine;

            if (!Selection.ColumnSelectionMode)
                if (Selection.End.iChar == 0) to--;

            int numberOfDeletedWhitespacesOfFirstLine = 0;
            int numberOfDeletetWhitespacesOfLastLine = 0;

            for (int i = from; i <= to; i++)
            {
                if (startCharIndex > lines[i].Count)
                    continue;
                // Select first characters from the line
                int endIndex = Math.Min(this.lines[i].Count, startCharIndex + this.TabLength);
                string wasteText = this.lines[i].Text.Substring(startCharIndex, endIndex-startCharIndex);

                // Only select the first whitespace characters
                endIndex = Math.Min(endIndex, startCharIndex + wasteText.Length - wasteText.TrimStart().Length);

                // Select the characters to remove
                this.Selection = new Range(this, new Place(startCharIndex, i), new Place(endIndex, i));

                // Remember characters to remove for first and last line
                int numberOfWhitespacesToRemove = endIndex - startCharIndex;
                if (i == currentSelection.Start.iLine)
                {
                    numberOfDeletedWhitespacesOfFirstLine = numberOfWhitespacesToRemove;
                }
                if (i == currentSelection.End.iLine)
                {
                    numberOfDeletetWhitespacesOfLastLine = numberOfWhitespacesToRemove;
                }

                // Remove marked/selected whitespace characters
                if(!Selection.IsEmpty)
                    this.ClearSelected();
            }

            // Restore selection
            if (Selection.ColumnSelectionMode == false)
            {
                int newSelectionStartCharacterIndex = Math.Max(0, currentSelection.Start.iChar - numberOfDeletedWhitespacesOfFirstLine);
                int newSelectionEndCharacterIndex = Math.Max(0, currentSelection.End.iChar - numberOfDeletetWhitespacesOfLastLine);
                this.Selection.Start = new Place(newSelectionStartCharacterIndex, currentSelection.Start.iLine);
                this.Selection.End = new Place(newSelectionEndCharacterIndex, currentSelection.End.iLine);
            }
            else
            {
                Selection = old;
            }
            lines.Manager.EndAutoUndoCommands();

            needRecalc = true;
            Selection.EndUpdate();
            EndUpdate();
            Invalidate();
        }

        /// <summary>
        /// Remove TAB in front of the caret ot the selected line.
        /// </summary>
        protected virtual void DecreaseIndentOfSingleLine()
        {
            if (this.Selection.Start.iLine != this.Selection.End.iLine)
                return;

            // Remeber current selection infos
            Range currentSelection = this.Selection.Clone();
            int currentLineIndex = this.Selection.Start.iLine;
            int currentLeftSelectionStartIndex = Math.Min(this.Selection.Start.iChar, this.Selection.End.iChar);

            // Determine number of whitespaces to remove
            string lineText = this.lines[currentLineIndex].Text;
            Match whitespacesLeftOfSelectionStartMatch = new Regex(@"\s*", RegexOptions.RightToLeft).Match(lineText, currentLeftSelectionStartIndex);
            int leftOffset = whitespacesLeftOfSelectionStartMatch.Index;
            int countOfWhitespaces = whitespacesLeftOfSelectionStartMatch.Length;
            int numberOfCharactersToRemove = 0;
            if (countOfWhitespaces > 0)
            {
                int remainder = (this.TabLength > 0)
                    ? currentLeftSelectionStartIndex % this.TabLength
                    : 0;
                numberOfCharactersToRemove = (remainder != 0)
                    ? Math.Min(remainder, countOfWhitespaces)
                    : Math.Min(this.TabLength, countOfWhitespaces);
            }

            // Remove whitespaces if available
            if (numberOfCharactersToRemove > 0)
            {
                // Start selection update
                this.BeginUpdate();
                this.Selection.BeginUpdate();
                lines.Manager.BeginAutoUndoCommands();
                lines.Manager.ExecuteCommand(new SelectCommand(TextSource));//remember selection

                // Remove whitespaces
                this.Selection.Start = new Place(leftOffset, currentLineIndex);
                this.Selection.End = new Place(leftOffset + numberOfCharactersToRemove, currentLineIndex);
                ClearSelected();

                // Restore selection
                int newSelectionStartCharacterIndex = currentSelection.Start.iChar - numberOfCharactersToRemove;
                int newSelectionEndCharacterIndex = currentSelection.End.iChar - numberOfCharactersToRemove;
                this.Selection.Start = new Place(newSelectionStartCharacterIndex, currentLineIndex);
                this.Selection.End = new Place(newSelectionEndCharacterIndex, currentLineIndex);

                lines.Manager.ExecuteCommand(new SelectCommand(TextSource));//remember selection
                // End selection update
                lines.Manager.EndAutoUndoCommands();
                this.Selection.EndUpdate();
                this.EndUpdate();
            }

            Invalidate();
        }


        /// <summary>
        /// Insert autoindents into selected lines
        /// </summary>
        public virtual void DoAutoIndent()
        {
            if (Selection.ColumnSelectionMode)
                return;
            Range r = Selection.Clone();
            r.Normalize();
            //
            BeginUpdate();
            Selection.BeginUpdate();
            lines.Manager.BeginAutoUndoCommands();
            //
            for (int i = r.Start.iLine; i <= r.End.iLine; i++)
                DoAutoIndent(i);
            //
            lines.Manager.EndAutoUndoCommands();
            Selection.Start = r.Start;
            Selection.End = r.End;
            Selection.Expand();
            //
            Selection.EndUpdate();
            EndUpdate();
        }

        /// <summary>
        /// Insert prefix into front of seletcted lines
        /// </summary>
        public virtual void InsertLinePrefix(string prefix)
        {
            int from = Math.Min(Selection.Start.iLine, Selection.End.iLine);
            int to = Math.Max(Selection.Start.iLine, Selection.End.iLine);
            BeginUpdate();
            Selection.BeginUpdate();
            lines.Manager.BeginAutoUndoCommands();
            lines.Manager.ExecuteCommand(new SelectCommand(TextSource));
            int spaces = GetMinStartSpacesCount(from, to);
            for (int i = from; i <= to; i++)
            {
                Selection.Start = new Place(spaces, i);
                lines.Manager.ExecuteCommand(new InsertTextCommand(TextSource, prefix));
            }
            Selection.Start = new Place(0, from);
            Selection.End = new Place(lines[to].Count, to);
            needRecalc = true;
            lines.Manager.EndAutoUndoCommands();
            Selection.EndUpdate();
            EndUpdate();
            Invalidate();
        }

        /// <summary>
        /// Remove prefix from front of selected lines
        /// This method ignores forward spaces of the line
        /// </summary>
        public virtual void RemoveLinePrefix(string prefix)
        {
            int from = Math.Min(Selection.Start.iLine, Selection.End.iLine);
            int to = Math.Max(Selection.Start.iLine, Selection.End.iLine);
            BeginUpdate();
            Selection.BeginUpdate();
            lines.Manager.BeginAutoUndoCommands();
            lines.Manager.ExecuteCommand(new SelectCommand(TextSource));
            for (int i = from; i <= to; i++)
            {
                string text = lines[i].Text;
                string trimmedText = text.TrimStart();
                if (trimmedText.StartsWith(prefix))
                {
                    int spaces = text.Length - trimmedText.Length;
                    Selection.Start = new Place(spaces, i);
                    Selection.End = new Place(spaces + prefix.Length, i);
                    ClearSelected();
                }
            }
            Selection.Start = new Place(0, from);
            Selection.End = new Place(lines[to].Count, to);
            needRecalc = true;
            lines.Manager.EndAutoUndoCommands();
            Selection.EndUpdate();
            EndUpdate();
        }

        /// <summary>
        /// Begins AutoUndo block.
        /// All changes of text between BeginAutoUndo() and EndAutoUndo() will be canceled in one operation Undo.
        /// </summary>
        public void BeginAutoUndo()
        {
            lines.Manager.BeginAutoUndoCommands();
        }

        /// <summary>
        /// Ends AutoUndo block.
        /// All changes of text between BeginAutoUndo() and EndAutoUndo() will be canceled in one operation Undo.
        /// </summary>
        public void EndAutoUndo()
        {
            lines.Manager.EndAutoUndoCommands();
        }

        public virtual void OnVisualMarkerClick(MouseEventArgs args, StyleVisualMarker marker)
        {
            if (VisualMarkerClick != null)
                VisualMarkerClick(this, new VisualMarkerEventArgs(marker.Style, marker, args));
            marker.Style.OnVisualMarkerClick(this, new VisualMarkerEventArgs(marker.Style, marker, args));
        }

        protected virtual void OnMarkerClick(MouseEventArgs args, VisualMarker marker)
        {
            if (marker is StyleVisualMarker)
            {
                OnVisualMarkerClick(args, marker as StyleVisualMarker);
                return;
            }
            if (marker is CollapseFoldingMarker)
            {
                CollapseFoldingBlock((marker as CollapseFoldingMarker).iLine);
                return;
            }

            if (marker is ExpandFoldingMarker)
            {
                ExpandFoldedBlock((marker as ExpandFoldingMarker).iLine);
                return;
            }

            if (marker is FoldedAreaMarker)
            {
                //select folded block
                int iStart = (marker as FoldedAreaMarker).iLine;
                int iEnd = FindEndOfFoldingBlock(iStart);
                if (iEnd < 0)
                    return;
                Selection.BeginUpdate();
                Selection.Start = new Place(0, iStart);
                Selection.End = new Place(lines[iEnd].Count, iEnd);
                Selection.EndUpdate();
                Invalidate();
                return;
            }
        }

        protected virtual void OnMarkerDoubleClick(VisualMarker marker)
        {
            if (marker is FoldedAreaMarker)
            {
                ExpandFoldedBlock((marker as FoldedAreaMarker).iLine);
                Invalidate();
                return;
            }
        }

        private void ClearBracketsPositions()
        {
            leftBracketPosition = null;
            rightBracketPosition = null;
            leftBracketPosition2 = null;
            rightBracketPosition2 = null;
        }

        /// <summary>
        /// Highlights brackets around caret
        /// </summary>
        private void HighlightBrackets(char LeftBracket, char RightBracket, ref Range leftBracketPosition, ref Range rightBracketPosition)
        {
            switch(BracketsHighlightStrategy)
            {
                case BracketsHighlightStrategy.Strategy1: HighlightBrackets1(LeftBracket, RightBracket, ref leftBracketPosition, ref rightBracketPosition); break;
                case BracketsHighlightStrategy.Strategy2: HighlightBrackets2(LeftBracket, RightBracket, ref leftBracketPosition, ref rightBracketPosition); break;
            }
        }

        private void HighlightBrackets1(char LeftBracket, char RightBracket, ref Range leftBracketPosition, ref Range rightBracketPosition)
        {
            if (!Selection.IsEmpty)
                return;
            if (LinesCount == 0)
                return;
            //
            Range oldLeftBracketPosition = leftBracketPosition;
            Range oldRightBracketPosition = rightBracketPosition;
            var range = GetBracketsRange(Selection.Start, LeftBracket, RightBracket);

            if(range != null)
            {
                leftBracketPosition = new Range(this, range.Start, new Place(range.Start.iChar + 1, range.Start.iLine));
                rightBracketPosition = new Range(this, new Place(range.End.iChar - 1, range.End.iLine), range.End);
            }

            if (oldLeftBracketPosition != leftBracketPosition ||
                oldRightBracketPosition != rightBracketPosition)
                Invalidate();
        }

        /// <summary>
        /// Returns range between brackets (or null if not found)
        /// </summary>
        public Range GetBracketsRange(Place placeInsideBrackets, char leftBracket, char rightBracket)
        {
            var startRange = new Range(this, placeInsideBrackets, placeInsideBrackets);
            var range = startRange.Clone();

            Range leftBracketPosition = null;
            Range rightBracketPosition = null;

            int counter = 0;
            int maxIterations = maxBracketSearchIterations;
            while (range.GoLeftThroughFolded()) //move caret left
            {
                if (range.CharAfterStart == leftBracket) counter++;
                if (range.CharAfterStart == rightBracket) counter--;
                if (counter == 1)
                {
                    //highlighting
                    range.End = new Place(range.Start.iChar + 1, range.Start.iLine);
                    leftBracketPosition = range;
                    break;
                }
                //
                maxIterations--;
                if (maxIterations <= 0) break;
            }
            //
            range = startRange.Clone();
            counter = 0;
            maxIterations = maxBracketSearchIterations;
            do
            {
                if (range.CharAfterStart == leftBracket) counter++;
                if (range.CharAfterStart == rightBracket) counter--;
                if (counter == -1)
                {
                    //highlighting
                    range.End = new Place(range.Start.iChar + 1, range.Start.iLine);
                    rightBracketPosition = range;
                    break;
                }
                //
                maxIterations--;
                if (maxIterations <= 0) break;
            } while (range.GoRightThroughFolded()); //move caret right

            if (leftBracketPosition != null && rightBracketPosition != null)
                return new Range(this, leftBracketPosition.Start, rightBracketPosition.End);
            else
                return null;
        }

        private void HighlightBrackets2(char LeftBracket, char RightBracket, ref Range leftBracketPosition, ref Range rightBracketPosition)
        {
            if (!Selection.IsEmpty)
                return;
            if (LinesCount == 0)
                return;
            //
            Range oldLeftBracketPosition = leftBracketPosition;
            Range oldRightBracketPosition = rightBracketPosition;
            Range range = Selection.Clone(); //need clone because we will move caret

            bool found = false;
            int counter = 0;
            int maxIterations = maxBracketSearchIterations;
            if (range.CharBeforeStart == RightBracket)
            {
                rightBracketPosition = new Range(this, range.Start.iChar - 1, range.Start.iLine, range.Start.iChar, range.Start.iLine);
                while (range.GoLeftThroughFolded()) //move caret left
                {
                    if (range.CharAfterStart == LeftBracket) counter++;
                    if (range.CharAfterStart == RightBracket) counter--;
                    if (counter == 0)
                    {
                        //highlighting
                        range.End = new Place(range.Start.iChar + 1, range.Start.iLine);
                        leftBracketPosition = range;
                        found = true;
                        break;
                    }
                    //
                    maxIterations--;
                    if (maxIterations <= 0) break;
                }
            }
            //
            range = Selection.Clone(); //need clone because we will move caret
            counter = 0;
            maxIterations = maxBracketSearchIterations;
            if(!found)
            if (range.CharAfterStart == LeftBracket)
            {
                leftBracketPosition = new Range(this, range.Start.iChar, range.Start.iLine, range.Start.iChar + 1, range.Start.iLine);
                do
                {
                    if (range.CharAfterStart == LeftBracket) counter++;
                    if (range.CharAfterStart == RightBracket) counter--;
                    if (counter == 0)
                    {
                        //highlighting
                        range.End = new Place(range.Start.iChar + 1, range.Start.iLine);
                        rightBracketPosition = range;
                        found = true;
                        break;
                    }
                    //
                    maxIterations--;
                    if (maxIterations <= 0) break;
                } while (range.GoRightThroughFolded()); //move caret right
            }

            if (oldLeftBracketPosition != leftBracketPosition || oldRightBracketPosition != rightBracketPosition)
                Invalidate();
        }

        /// <summary>
        /// Selectes next fragment for given regex.
        /// </summary>
        public bool SelectNext(string regexPattern, bool backward = false, RegexOptions options = RegexOptions.None)
        {
            var sel = Selection.Clone();
            sel.Normalize();
            var range1 = backward ? new Range(this, Range.Start, sel.Start) : new Range(this, sel.End, Range.End);

            Range res = null;
            foreach(var r in range1.GetRanges(regexPattern, options))
            {
                res = r;
                if (!backward) break;
            }

            if (res == null) return false;
            Selection = res;
            Invalidate();
            return true;
        }

        public virtual void OnSyntaxHighlight(TextChangedEventArgs args)
        {
            #if debug
            Stopwatch sw = Stopwatch.StartNew();
            #endif

            Range range;

            switch (HighlightingRangeType)
            {
                case HighlightingRangeType.VisibleRange:
                    range = VisibleRange.GetUnionWith(args.ChangedRange);
                    break;
                case HighlightingRangeType.AllTextRange:
                    range = Range;
                    break;
                default:
                    range = args.ChangedRange;
                    break;
            }

            if (SyntaxHighlighter != null)
            {
                if (Language == Language.Custom && SyntaxDescriptor != null)
                    SyntaxHighlighter.HighlightSyntax(SyntaxDescriptor, range);
                else
                    SyntaxHighlighter.HighlightSyntax(Language, range);
            }

#if debug
            Console.WriteLine("OnSyntaxHighlight: "+ sw.ElapsedMilliseconds);
#endif
        }

        private void InitializeComponent()
        {
            SuspendLayout();
            // 
            // FastColoredTextBox
            // 
            Name = "FastColoredTextBox";
            ResumeLayout(false);
        }

        /// <summary>
        /// Prints range of text
        /// </summary>
        public virtual void Print(Range range, PrintDialogSettings settings)
        {
            //prepare export with wordwrapping
            var exporter = new ExportToHTML();
            exporter.UseBr = true;
            exporter.UseForwardNbsp = true;
            exporter.UseNbsp = true;
            exporter.UseStyleTag = false;
            exporter.IncludeLineNumbers = settings.IncludeLineNumbers;

            if (range == null)
                range = Range;

            if (range.Text == string.Empty)
                return;

            //change visible range
            visibleRange = range;
            try
            {
                //call handlers for VisibleRange
                if (VisibleRangeChanged != null)
                    VisibleRangeChanged(this, new EventArgs());
                if (VisibleRangeChangedDelayed != null)
                    VisibleRangeChangedDelayed(this, new EventArgs());
            }
            finally
            {
                //restore visible range
                visibleRange = null;
            }

            //generate HTML
            string HTML = exporter.GetHtml(range);
            HTML = "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\"><head><title>" +
                   PrepareHtmlText(settings.Title) + "</title></head>" + HTML +"<br>"+ SelectHTMLRangeScript();
            string tempFile = Path.GetTempPath() + "fctb.html";
            File.WriteAllText(tempFile, HTML);

            //clear wb page setup settings
            SetPageSetupSettings(settings);

            //create wb
            var wb = new WebBrowser();
            wb.Tag = settings;
            wb.Visible = false;
            wb.Location = new Point(-1000, -1000);
            wb.Parent = this;
            wb.StatusTextChanged += wb_StatusTextChanged;
            wb.Navigate(tempFile);
        }

        protected virtual string PrepareHtmlText(string s)
        {
            return s.Replace("<", "&lt;").Replace(">", "&gt;").Replace("&", "&amp;");
        }

        private void wb_StatusTextChanged(object sender, EventArgs e)
        {
            var wb = sender as WebBrowser;
            if (wb.StatusText.Contains("#print"))
            {
                var settings = wb.Tag as PrintDialogSettings;
                try
                {
                    //show print dialog
                    if (settings.ShowPrintPreviewDialog)
                        wb.ShowPrintPreviewDialog();
                    else
                    {
                        if (settings.ShowPageSetupDialog)
                            wb.ShowPageSetupDialog();

                        if (settings.ShowPrintDialog)
                            wb.ShowPrintDialog();
                        else
                            wb.Print();
                    }
                }
                finally
                {
                    //destroy webbrowser
                    wb.Parent = null;
                    wb.Dispose();
                }
            }
        }

        /// <summary>
        /// Prints all text
        /// </summary>
        public void Print(PrintDialogSettings settings)
        {
            Print(Range, settings);
        }

        /// <summary>
        /// Prints all text, without any dialog windows
        /// </summary>
        public void Print()
        {
            Print(Range,
                  new PrintDialogSettings
                      {ShowPageSetupDialog = false, ShowPrintDialog = false, ShowPrintPreviewDialog = false});
        }

        private string SelectHTMLRangeScript()
        {
            Range sel = Selection.Clone();
            sel.Normalize();
            int start = PlaceToPosition(sel.Start) - sel.Start.iLine;
            int len = sel.Text.Length - (sel.End.iLine - sel.Start.iLine);
            return string.Format(
                @"<script type=""text/javascript"">
try{{
    var sel = document.selection;
    var rng = sel.createRange();
    rng.moveStart(""character"", {0});
    rng.moveEnd(""character"", {1});
    rng.select();
}}catch(ex){{}}
window.status = ""#print"";
</script>",
                start, len);
        }

        private static void SetPageSetupSettings(PrintDialogSettings settings)
        {
            RegistryKey key = Registry.CurrentUser.OpenSubKey(@"Software\Microsoft\Internet Explorer\PageSetup", true);
            if (key != null)
            {
                key.SetValue("footer", settings.Footer);
                key.SetValue("header", settings.Header);
            }
        }

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing)
            {
                if (SyntaxHighlighter != null)
                    SyntaxHighlighter.Dispose();
                timer.Dispose();
                timer2.Dispose();
                middleClickScrollingTimer.Dispose();

                if (findForm != null)
                    findForm.Dispose();

                if (replaceForm != null)
                    replaceForm.Dispose();
                /*
                if (Font != null)
                    Font.Dispose();

                if (originalFont != null)
                    originalFont.Dispose();*/

                if (TextSource != null)
                    TextSource.Dispose();

                if (ToolTip != null)
                    ToolTip.Dispose();

                if (SyntaxDescriptor != null)
                    SyntaxDescriptor.Dispose();
            }
        }

        protected virtual void OnPaintLine(PaintLineEventArgs e)
        {
            if (PaintLine != null)
                PaintLine(this, e);
        }

        internal void OnLineInserted(int index)
        {
            OnLineInserted(index, 1);
        }

        internal void OnLineInserted(int index, int count)
        {
            if (LineInserted != null)
                LineInserted(this, new LineInsertedEventArgs(index, count));
        }

        internal void OnLineRemoved(int index, int count, List<int> removedLineIds)
        {
            if (count > 0)
                if (LineRemoved != null)
                    LineRemoved(this, new LineRemovedEventArgs(index, count, removedLineIds));
        }

        /// <summary>
        /// Open text file
        /// </summary>
        public void OpenFile(string fileName, Encoding enc)
        {
            var ts = CreateTextSource();
            try
            {
                InitTextSource(ts);
                Text = File.ReadAllText(fileName, enc);
                ClearUndo();
                IsChanged = false;
                OnVisibleRangeChanged();
            }
            catch
            {
                InitTextSource(CreateTextSource());
                lines.InsertLine(0, TextSource.CreateLine());
                IsChanged = false;
                throw;
            }
            Selection.Start = Place.Empty;
            DoSelectionVisible();
        }

        /// <summary>
        /// Open text file (with automatic encoding detector)
        /// </summary>
        public void OpenFile(string fileName)
        {
            try
            {
                var enc = EncodingDetector.DetectTextFileEncoding(fileName);
                if (enc != null)
                    OpenFile(fileName, enc);
                else
                    OpenFile(fileName, Encoding.Default);
            }
            catch
            {
                InitTextSource(CreateTextSource());
                lines.InsertLine(0, TextSource.CreateLine());
                IsChanged = false;
                throw;
            }
        }

        /// <summary>
        /// Open file binding mode
        /// </summary>
        /// <param name="fileName"></param>
        /// <param name="enc"></param>
        public void OpenBindingFile(string fileName, Encoding enc)
        {
            var fts = new FileTextSource(this);
            try
            {
                InitTextSource(fts);
                fts.OpenFile(fileName, enc);
                IsChanged = false;
                OnVisibleRangeChanged();
            }
            catch
            {
                fts.CloseFile();
                InitTextSource(CreateTextSource());
                lines.InsertLine(0, TextSource.CreateLine());
                IsChanged = false;
                throw;
            }
            Invalidate();
        }

        /// <summary>
        /// Close file binding mode
        /// </summary>
        public void CloseBindingFile()
        {
            if (lines is FileTextSource)
            {
                var fts = lines as FileTextSource;
                fts.CloseFile();

                InitTextSource(CreateTextSource());
                lines.InsertLine(0, TextSource.CreateLine());
                IsChanged = false;
                Invalidate();
            }
        }

        /// <summary>
        /// Save text to the file
        /// </summary>
        /// <param name="fileName"></param>
        /// <param name="enc"></param>
        public void SaveToFile(string fileName, Encoding enc)
        {
            lines.SaveToFile(fileName, enc);
            IsChanged = false;
            OnVisibleRangeChanged();
            UpdateScrollbars();
        }

        /// <summary>
        /// Set VisibleState of line
        /// </summary>
        public void SetVisibleState(int iLine, VisibleState state)
        {
            LineInfo li = LineInfos[iLine];
            li.VisibleState = state;
            LineInfos[iLine] = li;
            needRecalc = true;
        }

        /// <summary>
        /// Returns VisibleState of the line
        /// </summary>
        public VisibleState GetVisibleState(int iLine)
        {
            return LineInfos[iLine].VisibleState;
        }

        /// <summary>
        /// Shows Goto dialog form
        /// </summary>
        public void ShowGoToDialog()
        {
            var form = new GoToForm();
            form.StartPosition = FormStartPosition.CenterParent;
            form.TotalLineCount = LinesCount;
            form.SelectedLineNumber = Selection.Start.iLine + 1;

            if (form.ShowDialog(this) == DialogResult.OK)
            {
                int line = Math.Min(LinesCount - 1, Math.Max(0, form.SelectedLineNumber - 1));
                Selection = new Range(this, 0, line, 0, line);
                DoSelectionVisible();
            }
        }

        /// <summary>
        /// Occurs when undo/redo stack is changed
        /// </summary>
        public void OnUndoRedoStateChanged()
        {
            if (UndoRedoStateChanged != null)
                UndoRedoStateChanged(this, EventArgs.Empty);
        }

        /// <summary>
        /// Search lines by regex pattern
        /// </summary>
        public List<int> FindLines(string searchPattern, RegexOptions options)
        {
            var iLines = new List<int>();
            foreach (Range r in Range.GetRangesByLines(searchPattern, options))
                iLines.Add(r.Start.iLine);

            return iLines;
        }

        /// <summary>
        /// Removes given lines
        /// </summary>
        public void RemoveLines(List<int> iLines)
        {
            TextSource.Manager.ExecuteCommand(new RemoveLinesCommand(TextSource, iLines));
            if (iLines.Count > 0)
                IsChanged = true;
            if (LinesCount == 0)
                Text = "";
            NeedRecalc();
            Invalidate();
        }

        void ISupportInitialize.BeginInit()
        {
            //
        }

        void ISupportInitialize.EndInit()
        {
            OnTextChanged();
            Selection.Start = Place.Empty;
            DoCaretVisible();
            IsChanged = false;
            ClearUndo();
        }

        #region Drag and drop

        private bool IsDragDrop { get; set; }


        protected override void OnDragEnter(DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.Text) && AllowDrop)
            {
                e.Effect = DragDropEffects.Copy;
                IsDragDrop = true;
            }
            base.OnDragEnter(e);
        }

        protected override void OnDragDrop(DragEventArgs e)
        {
            if (ReadOnly || !AllowDrop)
            {
                IsDragDrop = false;
                return;
            }

            if (e.Data.GetDataPresent(DataFormats.Text))
            {
                if (ParentForm != null)
                    ParentForm.Activate();
                Focus();
                Point p = PointToClient(new Point(e.X, e.Y));
                var text = e.Data.GetData(DataFormats.Text).ToString();
                var place = PointToPlace(p);
                DoDragDrop(place, text);
                IsDragDrop = false;
            }
            base.OnDragDrop(e);
        }

        private void DoDragDrop_old(Place place, string text)
        {
            Range insertRange = new Range(this, place, place);

            // Abort, if insertRange is read only
            if (insertRange.ReadOnly)
                return;

            // Abort, if dragged range contains target place
            if ((draggedRange != null) && (draggedRange.Contains(place) == true))
                return;

            // Determine, if the dragged string should be copied or moved
            bool copyMode =
                (draggedRange == null) ||       // drag from outside
                (draggedRange.ReadOnly) ||      // dragged range is read only
                ((ModifierKeys & Keys.Control) != Keys.None);

            //drag from outside?
            if (draggedRange == null)
            {
                Selection.BeginUpdate();
                // Insert text
                Selection.Start = place;
                InsertText(text);
                // Select inserted text
                Selection = new Range(this, place, Selection.Start);
                Selection.EndUpdate();
                return;
            }

            //drag from me
            Place caretPositionAfterInserting;
            BeginAutoUndo();
            Selection.BeginUpdate();

            //remember dragged selection for undo/redo
            Selection = draggedRange;
            lines.Manager.ExecuteCommand(new SelectCommand(lines));
            //
            if (draggedRange.ColumnSelectionMode)
            {
                draggedRange.Normalize();
                insertRange = new Range(this, place, new Place(place.iChar, place.iLine + draggedRange.End.iLine - draggedRange.Start.iLine)) { ColumnSelectionMode = true };
                for (int i = LinesCount; i <= insertRange.End.iLine; i++)
                {
                    Selection.GoLast(false);
                    InsertChar('\n');
                }
            }

            if (!insertRange.ReadOnly)
            {
                if (place < draggedRange.Start)
                {
                    // Delete dragged range if not in copy mode
                    if (copyMode == false)
                    {
                        Selection = draggedRange;
                        ClearSelected();
                    }

                    // Insert text
                    Selection = insertRange;
                    Selection.ColumnSelectionMode = insertRange.ColumnSelectionMode;
                    InsertText(text);
                    caretPositionAfterInserting = Selection.Start;
                }
                else
                {
                    // Insert text
                    Selection = insertRange;
                    Selection.ColumnSelectionMode = insertRange.ColumnSelectionMode;
                    InsertText(text);
                    caretPositionAfterInserting = Selection.Start;
                    var lineLength = this[caretPositionAfterInserting.iLine].Count;

                    // Delete dragged range if not in copy mode
                    if (copyMode == false)
                    {
                        Selection = draggedRange;
                        ClearSelected();
                    }

                    var shift = lineLength - this[caretPositionAfterInserting.iLine].Count;
                    caretPositionAfterInserting.iChar = caretPositionAfterInserting.iChar - shift;
                    place.iChar = place.iChar - shift;
                }

                // Select inserted text
                if (!draggedRange.ColumnSelectionMode)
                {
                    Selection = new Range(this, place, caretPositionAfterInserting);
                }
                else
                {
                    draggedRange.Normalize();
                    Selection = new Range(this, place,
                                            new Place(place.iChar + draggedRange.End.iChar - draggedRange.Start.iChar,
                                                    place.iLine + draggedRange.End.iLine - draggedRange.Start.iLine)) { ColumnSelectionMode = true };
                }
            }

            Selection.EndUpdate();
            EndAutoUndo();
            draggedRange = null;
        }

        protected virtual void DoDragDrop(Place place, string text)
        {
            Range insertRange = new Range(this, place, place);

            // Abort, if insertRange is read only
            if (insertRange.ReadOnly)
                return;

            // Abort, if dragged range contains target place
            if ((draggedRange != null) && (draggedRange.Contains(place) == true))
                return;

            // Determine, if the dragged string should be copied or moved
            bool copyMode =
                (draggedRange == null) ||       // drag from outside
                (draggedRange.ReadOnly) ||      // dragged range is read only
                ((ModifierKeys & Keys.Control) != Keys.None);

            if (draggedRange == null)//drag from outside
            {
                Selection.BeginUpdate();
                // Insert text
                Selection.Start = place;
                InsertText(text);
                // Select inserted text
                Selection = new Range(this, place, Selection.Start);
                Selection.EndUpdate();
            }
            else//drag from me
            {
                if (!draggedRange.Contains(place))
                {
                    BeginAutoUndo();

                    //remember dragged selection for undo/redo
                    Selection = draggedRange;
                    lines.Manager.ExecuteCommand(new SelectCommand(lines));
                    //
                    if (draggedRange.ColumnSelectionMode)
                    {
                        draggedRange.Normalize();
                        insertRange = new Range(this, place, new Place(place.iChar, place.iLine + draggedRange.End.iLine - draggedRange.Start.iLine)) { ColumnSelectionMode = true };
                        for (int i = LinesCount; i <= insertRange.End.iLine; i++)
                        {
                            Selection.GoLast(false);
                            InsertChar('\n');
                        }
                    }

                    if (!insertRange.ReadOnly)
                    {
                        if (place < draggedRange.Start)
                        {
                            // Delete dragged range if not in copy mode
                            if (copyMode == false)
                            {
                                Selection = draggedRange;
                                ClearSelected();
                            }

                            // Insert text
                            Selection = insertRange;
                            Selection.ColumnSelectionMode = insertRange.ColumnSelectionMode;
                            InsertText(text);
                        }
                        else
                        {
                            // Insert text
                            Selection = insertRange;
                            Selection.ColumnSelectionMode = insertRange.ColumnSelectionMode;
                            InsertText(text);

                            // Delete dragged range if not in copy mode
                            if (copyMode == false)
                            {
                                Selection = draggedRange;
                                ClearSelected();
                            }
                        }
                    }

                    // Selection start and end position
                    Place startPosition = place;
                    Place endPosition = Selection.Start;

                    // Correct selection
                    Range dR = (draggedRange.End > draggedRange.Start)  // dragged selection
                        ? this.GetRange(draggedRange.Start, draggedRange.End)
                        : this.GetRange(draggedRange.End, draggedRange.Start);
                    Place tP = place; // targetPlace
                    int tS_S_Line;  // targetSelection.Start.iLine
                    int tS_S_Char;  // targetSelection.Start.iChar
                    int tS_E_Line;  // targetSelection.End.iLine
                    int tS_E_Char;  // targetSelection.End.iChar
                    if ((place > draggedRange.Start) && (copyMode == false))
                    {
                        if (draggedRange.ColumnSelectionMode == false)
                        {
                            // Normal selection mode:

                            // Determine character/column position of target selection
                            if (dR.Start.iLine != dR.End.iLine) // If more then one line was selected/dragged ...
                            {
                                tS_S_Char = (dR.End.iLine != tP.iLine)
                                    ? tP.iChar
                                    : dR.Start.iChar + (tP.iChar - dR.End.iChar);
                                tS_E_Char = dR.End.iChar;
                            }
                            else // only one line was selected/dragged
                            {
                                if (dR.End.iLine == tP.iLine)
                                {
                                    tS_S_Char = tP.iChar - dR.Text.Length;
                                    tS_E_Char = tP.iChar;
                                }
                                else
                                {
                                    tS_S_Char = tP.iChar;
                                    tS_E_Char = tP.iChar + dR.Text.Length;
                                }
                            }

                            // Determine line/row of target selection
                            if (dR.End.iLine != tP.iLine)
                            {
                                tS_S_Line = tP.iLine - (dR.End.iLine - dR.Start.iLine);
                                tS_E_Line = tP.iLine;
                            }
                            else
                            {
                                tS_S_Line = dR.Start.iLine;
                                tS_E_Line = dR.End.iLine;
                            }

                            startPosition = new Place(tS_S_Char, tS_S_Line);
                            endPosition = new Place(tS_E_Char, tS_E_Line);
                        }
                    }


                    // Select inserted text
                    if (!draggedRange.ColumnSelectionMode)
                        Selection = new Range(this, startPosition, endPosition);
                    else
                    {
                        if ((copyMode == false) &&
                            (place.iLine >= dR.Start.iLine) && (place.iLine <= dR.End.iLine) &&
                            (place.iChar >= dR.End.iChar))
                        {
                            tS_S_Char = tP.iChar - (dR.End.iChar - dR.Start.iChar);
                            tS_E_Char = tP.iChar;
                        }
                        else
                        {
                            tS_S_Char = tP.iChar;
                            tS_E_Char = tP.iChar + (dR.End.iChar - dR.Start.iChar);
                        }
                        tS_S_Line = tP.iLine;
                        tS_E_Line = tP.iLine + (dR.End.iLine - dR.Start.iLine);

                        startPosition = new Place(tS_S_Char, tS_S_Line);
                        endPosition = new Place(tS_E_Char, tS_E_Line);
                        Selection = new Range(this, startPosition, endPosition)
                        {
                            ColumnSelectionMode = true
                        };
                    }

                    EndAutoUndo();
                }

                this.selection.Inverse();
                OnSelectionChanged();
            }
            draggedRange = null;
        }

        protected override void OnDragOver(DragEventArgs e)
        {
            if (e.Data.GetDataPresent(DataFormats.Text))
            {
                Point p = PointToClient(new Point(e.X, e.Y));
                Selection.Start = PointToPlace(p);
                if (p.Y < 6 && VerticalScroll.Visible && VerticalScroll.Value > 0)
                    VerticalScroll.Value = Math.Max(0, VerticalScroll.Value - charHeight);

                DoCaretVisible();
                Invalidate();
            }
            base.OnDragOver(e);
        }

        protected override void OnDragLeave(EventArgs e)
        {
            IsDragDrop = false;
            base.OnDragLeave(e);
        }

        #endregion

        #region MiddleClickScrolling

        private bool middleClickScrollingActivated;
        private Point middleClickScrollingOriginPoint;
        private Point middleClickScrollingOriginScroll;
        private readonly Timer middleClickScrollingTimer = new Timer();
        private ScrollDirection middleClickScollDirection = ScrollDirection.None;
        private const int WM_SETREDRAW = 0xB;

        /// <summary>
        /// Activates the scrolling mode (middle click button).
        /// </summary>
        /// <param name="e">MouseEventArgs</param>
        private void ActivateMiddleClickScrollingMode(MouseEventArgs e)
        {
            if (!middleClickScrollingActivated)
            {
                if ((!HorizontalScroll.Visible) && (!VerticalScroll.Visible))
                if (ShowScrollBars)
                    return;
                middleClickScrollingActivated = true;
                middleClickScrollingOriginPoint = e.Location;
                middleClickScrollingOriginScroll = new Point(HorizontalScroll.Value, VerticalScroll.Value);
                middleClickScrollingTimer.Interval = 50;
                middleClickScrollingTimer.Enabled = true;
                Capture = true;
                // Refresh the control 
                Refresh();
                // Disable drawing
                NativeMethodsWrapper.SendMessage(Handle, WM_SETREDRAW, 0, 0);
            }
        }

        /// <summary>
        /// Deactivates the scrolling mode (middle click button).
        /// </summary>
        private void DeactivateMiddleClickScrollingMode()
        {
            if (middleClickScrollingActivated)
            {
                middleClickScrollingActivated = false;
                middleClickScrollingTimer.Enabled = false;
                Capture = false;
                base.Cursor = defaultCursor;
                // Enable drawing
                NativeMethodsWrapper.SendMessage(Handle, WM_SETREDRAW, 1, 0);
                Invalidate();
            }
        }

        /// <summary>
        /// Restore scrolls
        /// </summary>
        private void RestoreScrollsAfterMiddleClickScrollingMode()
        {
            var xea = new ScrollEventArgs(ScrollEventType.ThumbPosition,
                HorizontalScroll.Value,
                middleClickScrollingOriginScroll.X,
                ScrollOrientation.HorizontalScroll);
            OnScroll(xea);

            var yea = new ScrollEventArgs(ScrollEventType.ThumbPosition,
                VerticalScroll.Value,
                middleClickScrollingOriginScroll.Y,
                ScrollOrientation.VerticalScroll);
            OnScroll(yea);
        }

        void middleClickScrollingTimer_Tick(object sender, EventArgs e)
        {
            if (IsDisposed)
                return;

            if (!middleClickScrollingActivated)
                return;

            Point currentMouseLocation = PointToClient(Cursor.Position);

            Capture = true;

            // Calculate angle and distance between current position point and origin point
            int distanceX = this.middleClickScrollingOriginPoint.X - currentMouseLocation.X;
            int distanceY = this.middleClickScrollingOriginPoint.Y - currentMouseLocation.Y;

            if (!VerticalScroll.Visible && ShowScrollBars) distanceY = 0;
            if (!HorizontalScroll.Visible && ShowScrollBars) distanceX = 0;

            double angleInDegree = 180 - Math.Atan2(distanceY, distanceX) * 180 / Math.PI;
            double distance = Math.Sqrt(Math.Pow(distanceX, 2) + Math.Pow(distanceY, 2));

            // determine scrolling direction depending on the angle
            if (distance > 10)
            {
                if (angleInDegree >= 325 || angleInDegree <= 35)
                    this.middleClickScollDirection = ScrollDirection.Right;
                else if (angleInDegree <= 55)
                    this.middleClickScollDirection = ScrollDirection.Right | ScrollDirection.Up;
                else if (angleInDegree <= 125)
                    this.middleClickScollDirection = ScrollDirection.Up;
                else if (angleInDegree <= 145)
                    this.middleClickScollDirection = ScrollDirection.Up | ScrollDirection.Left;
                else if (angleInDegree <= 215)
                    this.middleClickScollDirection = ScrollDirection.Left;
                else if (angleInDegree <= 235)
                    this.middleClickScollDirection = ScrollDirection.Left | ScrollDirection.Down;
                else if (angleInDegree <= 305)
                    this.middleClickScollDirection = ScrollDirection.Down;
                else
                    this.middleClickScollDirection = ScrollDirection.Down | ScrollDirection.Right;
            }
            else
            {
                this.middleClickScollDirection = ScrollDirection.None;
            }

            // Set mouse cursor
            switch (this.middleClickScollDirection)
            {
                case ScrollDirection.Right: base.Cursor = Cursors.PanEast; break;
                case ScrollDirection.Right | ScrollDirection.Up: base.Cursor = Cursors.PanNE; break;
                case ScrollDirection.Up: base.Cursor = Cursors.PanNorth; break;
                case ScrollDirection.Up | ScrollDirection.Left: base.Cursor = Cursors.PanNW; break;
                case ScrollDirection.Left: base.Cursor = Cursors.PanWest; break;
                case ScrollDirection.Left | ScrollDirection.Down: base.Cursor = Cursors.PanSW; break;
                case ScrollDirection.Down: base.Cursor = Cursors.PanSouth; break;
                case ScrollDirection.Down | ScrollDirection.Right: base.Cursor = Cursors.PanSE; break;
                default: base.Cursor = defaultCursor; return;
            }

            var xScrollOffset = (int)(-distanceX / 5.0);
            var yScrollOffset = (int)(-distanceY / 5.0);

            var xea = new ScrollEventArgs(xScrollOffset < 0 ? ScrollEventType.SmallIncrement : ScrollEventType.SmallDecrement,
                HorizontalScroll.Value,
                HorizontalScroll.Value + xScrollOffset,
                ScrollOrientation.HorizontalScroll);

            var yea = new ScrollEventArgs(yScrollOffset < 0 ? ScrollEventType.SmallDecrement : ScrollEventType.SmallIncrement,
                VerticalScroll.Value,
                VerticalScroll.Value + yScrollOffset,
                ScrollOrientation.VerticalScroll);

            if ((middleClickScollDirection & (ScrollDirection.Down | ScrollDirection.Up)) > 0)
                //DoScrollVertical(1 + Math.Abs(yScrollOffset), Math.Sign(distanceY));
                OnScroll(yea, false);

            if ((middleClickScollDirection & (ScrollDirection.Right | ScrollDirection.Left)) > 0)
                OnScroll(xea);

            // Enable drawing
            NativeMethodsWrapper.SendMessage(Handle, WM_SETREDRAW, 1, 0);
            // Refresh the control 
            Refresh();
            // Disable drawing
            NativeMethodsWrapper.SendMessage(Handle, WM_SETREDRAW, 0, 0);
        }

        private void DrawMiddleClickScrolling(Graphics gr)
        {
            // If mouse scrolling mode activated draw the scrolling cursor image
            bool ableToScrollVertically = this.VerticalScroll.Visible || !ShowScrollBars;
            bool ableToScrollHorizontally = this.HorizontalScroll.Visible || !ShowScrollBars;

            // Calculate inverse color
            Color inverseColor = Color.FromArgb(100, (byte)~this.BackColor.R, (byte)~this.BackColor.G, (byte)~this.BackColor.B);
            using (SolidBrush inverseColorBrush = new SolidBrush(inverseColor))
            {
                var p = middleClickScrollingOriginPoint;

                var state = gr.Save();

                gr.SmoothingMode = SmoothingMode.HighQuality;
                gr.TranslateTransform(p.X, p.Y);
                gr.FillEllipse(inverseColorBrush, -2, -2, 4, 4);

                if (ableToScrollVertically) DrawTriangle(gr, inverseColorBrush);
                gr.RotateTransform(90);
                if (ableToScrollHorizontally) DrawTriangle(gr, inverseColorBrush);
                gr.RotateTransform(90);
                if (ableToScrollVertically) DrawTriangle(gr, inverseColorBrush);
                gr.RotateTransform(90);
                if (ableToScrollHorizontally) DrawTriangle(gr, inverseColorBrush);

                gr.Restore(state);
            }
        }

        private void DrawTriangle(Graphics g, Brush brush)
        {
            const int size = 5;
            var points = new Point[] { new Point(size, 2 * size), new Point(0, 3 * size), new Point(-size, 2 * size) };
            g.FillPolygon(brush, points);
        }

        #endregion


        #region Nested type: LineYComparer

        private class LineYComparer : IComparer<LineInfo>
        {
            private readonly int Y;

            public LineYComparer(int Y)
            {
                this.Y = Y;
            }

            #region IComparer<LineInfo> Members

            public int Compare(LineInfo x, LineInfo y)
            {
                if (x.startY == -10)
                    return -y.startY.CompareTo(Y);
                else
                    return x.startY.CompareTo(Y);
            }

            #endregion
        }

        #endregion
    }

    public class PaintLineEventArgs : PaintEventArgs
    {
        public PaintLineEventArgs(int iLine, Rectangle rect, Graphics gr, Rectangle clipRect) : base(gr, clipRect)
        {
            LineIndex = iLine;
            LineRect = rect;
        }

        public int LineIndex { get; private set; }
        public Rectangle LineRect { get; private set; }
    }

    public class LineInsertedEventArgs : EventArgs
    {
        public LineInsertedEventArgs(int index, int count)
        {
            Index = index;
            Count = count;
        }

        /// <summary>
        /// Inserted line index
        /// </summary>
        public int Index { get; private set; }

        /// <summary>
        /// Count of inserted lines
        /// </summary>
        public int Count { get; private set; }
    }

    public class LineRemovedEventArgs : EventArgs
    {
        public LineRemovedEventArgs(int index, int count, List<int> removedLineIds)
        {
            Index = index;
            Count = count;
            RemovedLineUniqueIds = removedLineIds;
        }

        /// <summary>
        /// Removed line index
        /// </summary>
        public int Index { get; private set; }

        /// <summary>
        /// Count of removed lines
        /// </summary>
        public int Count { get; private set; }

        /// <summary>
        /// UniqueIds of removed lines
        /// </summary>
        public List<int> RemovedLineUniqueIds { get; private set; }
    }

    /// <summary>
    /// TextChanged event argument
    /// </summary>
    public class TextChangedEventArgs : EventArgs
    {
        /// <summary>
        /// Constructor
        /// </summary>
        public TextChangedEventArgs(Range changedRange)
        {
            ChangedRange = changedRange;
        }

        /// <summary>
        /// This range contains changed area of text
        /// </summary>
        public Range ChangedRange { get; set; }
    }

    public class TextChangingEventArgs : EventArgs
    {
        public string InsertingText { get; set; }

        /// <summary>
        /// Set to true if you want to cancel text inserting
        /// </summary>
        public bool Cancel { get; set; }
    }

    public class WordWrapNeededEventArgs : EventArgs
    {
        public List<int> CutOffPositions { get; private set;}
        public bool ImeAllowed { get; private set;}
        public Line Line { get; private set; }

        public WordWrapNeededEventArgs(List<int> cutOffPositions, bool imeAllowed, Line line)
        {
            this.CutOffPositions = cutOffPositions;
            this.ImeAllowed = imeAllowed;
            this.Line = line;
        }
    }

    public enum WordWrapMode
    {
        /// <summary>
        /// Word wrapping by control width
        /// </summary>
        WordWrapControlWidth,

        /// <summary>
        /// Word wrapping by preferred line width (PreferredLineWidth)
        /// </summary>
        WordWrapPreferredWidth,

        /// <summary>
        /// Char wrapping by control width
        /// </summary>
        CharWrapControlWidth,

        /// <summary>
        /// Char wrapping by preferred line width (PreferredLineWidth)
        /// </summary>
        CharWrapPreferredWidth,

        /// <summary>
        /// Custom wrap (by event WordWrapNeeded)
        /// </summary>
        Custom
    }

    public class PrintDialogSettings
    {
        public PrintDialogSettings()
        {
            ShowPrintPreviewDialog = true;
            Title = "";
            Footer = "";
            Header = "";
        }

        public bool ShowPageSetupDialog { get; set; }
        public bool ShowPrintDialog { get; set; }
        public bool ShowPrintPreviewDialog { get; set; }

        /// <summary>
        /// Title of page. If you want to print Title on the page, insert code &amp;w in Footer or Header.
        /// </summary>
        public string Title { get; set; }

        /// <summary>
        /// Footer of page.
        /// Here you can use special codes: &amp;w (Window title), &amp;D, &amp;d (Date), &amp;t(), &amp;4 (Time), &amp;p (Current page number), &amp;P (Total number of pages),  &amp;&amp; (A single ampersand), &amp;b (Right justify text, Center text. If &amp;b occurs once, then anything after the &amp;b is right justified. If &amp;b occurs twice, then anything between the two &amp;b is centered, and anything after the second &amp;b is right justified).
        /// More detailed see <see cref="http://msdn.microsoft.com/en-us/library/aa969429(v=vs.85).aspx">here</see>
        /// </summary>
        public string Footer { get; set; }

        /// <summary>
        /// Header of page
        /// Here you can use special codes: &amp;w (Window title), &amp;D, &amp;d (Date), &amp;t(), &amp;4 (Time), &amp;p (Current page number), &amp;P (Total number of pages),  &amp;&amp; (A single ampersand), &amp;b (Right justify text, Center text. If &amp;b occurs once, then anything after the &amp;b is right justified. If &amp;b occurs twice, then anything between the two &amp;b is centered, and anything after the second &amp;b is right justified).
        /// More detailed see <see cref="http://msdn.microsoft.com/en-us/library/aa969429(v=vs.85).aspx">here</see>
        /// </summary>
        public string Header { get; set; }

        /// <summary>
        /// Prints line numbers
        /// </summary>
        public bool IncludeLineNumbers { get; set; }
    }

    public class AutoIndentEventArgs : EventArgs
    {
        public AutoIndentEventArgs(int iLine, string lineText, string prevLineText, int tabLength, int currentIndentation)
        {
            this.iLine = iLine;
            LineText = lineText;
            PrevLineText = prevLineText;
            TabLength = tabLength;
            AbsoluteIndentation = currentIndentation;
        }

        public int iLine { get; internal set; }
        public int TabLength { get; internal set; }
        public string LineText { get; internal set; }
        public string PrevLineText { get; internal set; }

        /// <summary>
        /// Additional spaces count for this line, relative to previous line
        /// </summary>
        public int Shift { get; set; }

        /// <summary>
        /// Additional spaces count for next line, relative to previous line
        /// </summary>
        public int ShiftNextLines { get; set; }

        /// <summary>
        /// Absolute indentation of current line. You can change this property if you want to set absolute indentation.
        /// </summary>
        public int AbsoluteIndentation { get; set; }
    }

    /// <summary>
    /// Type of highlighting
    /// </summary>
    public enum HighlightingRangeType
    {
        /// <summary>
        /// Highlight only changed range of text. Highest performance.
        /// </summary>
        ChangedRange,

        /// <summary>
        /// Highlight visible range of text. Middle performance.
        /// </summary>
        VisibleRange,

        /// <summary>
        /// Highlight all (visible and invisible) text. Lowest performance.
        /// </summary>
        AllTextRange
    }

    /// <summary>
    /// Strategy of search of end of folding block
    /// </summary>
    public enum FindEndOfFoldingBlockStrategy
    {
        Strategy1,
        Strategy2
    }

    /// <summary>
    /// Strategy of search of brackets to highlighting
    /// </summary>
    public enum BracketsHighlightStrategy
    {
        Strategy1,
        Strategy2
    }

    /// <summary>
    /// ToolTipNeeded event args
    /// </summary>
    public class ToolTipNeededEventArgs : EventArgs
    {
        public ToolTipNeededEventArgs(Place place, string hoveredWord)
        {
            HoveredWord = hoveredWord;
            Place = place;
        }

        public Place Place { get; private set; }
        public string HoveredWord { get; private set; }
        public string ToolTipTitle { get; set; }
        public string ToolTipText { get; set; }
        public ToolTipIcon ToolTipIcon { get; set; }
    }

    /// <summary>
    /// HintClick event args
    /// </summary>
    public class HintClickEventArgs : EventArgs
    {
        public HintClickEventArgs(Hint hint)
        {
            Hint = hint;
        }

        public Hint Hint { get; private set; }
    }

    /// <summary>
    /// CustomAction event args
    /// </summary>
    public class CustomActionEventArgs : EventArgs
    {
        public FCTBAction Action { get; private set; }

        public CustomActionEventArgs(FCTBAction action)
        {
            Action = action;
        }
    }

    public enum TextAreaBorderType
    {
        None,
        Single,
        Shadow
    }

    [Flags]
    public enum ScrollDirection : ushort
    {
        None = 0,
        Left = 1,
        Right = 2,
        Up = 4,
        Down = 8
    }

    [Serializable]
    public class ServiceColors
    {
        public Color CollapseMarkerForeColor { get; set; }
        public Color CollapseMarkerBackColor { get; set; }
        public Color CollapseMarkerBorderColor { get; set; }
        public Color ExpandMarkerForeColor { get; set; }
        public Color ExpandMarkerBackColor { get; set; }
        public Color ExpandMarkerBorderColor { get; set; }

        public ServiceColors()
        {
            CollapseMarkerForeColor = Color.Silver;
            CollapseMarkerBackColor = Color.White;
            CollapseMarkerBorderColor = Color.Silver;
            ExpandMarkerForeColor = Color.Red;
            ExpandMarkerBackColor = Color.White;
            ExpandMarkerBorderColor = Color.Silver;
        }
    }

#if Styles32
    /// <summary>
    /// Style index mask (32 styles)
    /// </summary>
    [Flags]
    public enum StyleIndex : uint
    {
        None = 0,
        Style0 = 0x1,
        Style1 = 0x2,
        Style2 = 0x4,
        Style3 = 0x8,
        Style4 = 0x10,
        Style5 = 0x20,
        Style6 = 0x40,
        Style7 = 0x80,
        Style8 = 0x100,
        Style9 = 0x200,
        Style10 = 0x400,
        Style11 = 0x800,
        Style12 = 0x1000,
        Style13 = 0x2000,
        Style14 = 0x4000,
        Style15 = 0x8000,

        Style16 = 0x10000,
        Style17 = 0x20000,
        Style18 = 0x40000,
        Style19 = 0x80000,
        Style20 = 0x100000,
        Style21 = 0x200000,
        Style22 = 0x400000,
        Style23 = 0x800000,
        Style24 = 0x1000000,
        Style25 = 0x2000000,
        Style26 = 0x4000000,
        Style27 = 0x8000000,
        Style28 = 0x10000000,
        Style29 = 0x20000000,
        Style30 = 0x40000000,
        Style31 = 0x80000000,

        All = 0xffffffff
    }
#else
    /// <summary>
    /// Style index mask (16 styles)
    /// </summary>
    [Flags]
    public enum StyleIndex : ushort
    {
        None = 0,
        Style0 = 0x1,
        Style1 = 0x2,
        Style2 = 0x4,
        Style3 = 0x8,
        Style4 = 0x10,
        Style5 = 0x20,
        Style6 = 0x40,
        Style7 = 0x80,
        Style8 = 0x100,
        Style9 = 0x200,
        Style10 = 0x400,
        Style11 = 0x800,
        Style12 = 0x1000,
        Style13 = 0x2000,
        Style14 = 0x4000,
        Style15 = 0x8000,
        All = 0xffff
    }
#endif

}
