using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlTextbox : Control
	{
		private Regex _codeRegex = new Regex("^(\\s*)([a-z]{3})([*]{0,1})($|[ ]){1}([(]{0,1})(([$][0-9a-f]*)|(#[@$:_0-9a-z]*)|([@_a-z]([@_a-z0-9])*){0,1}(\\+(\\d+)){0,1}){0,1}([)]{0,1})(,X|,Y){0,1}([)]{0,1})(.*)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		public event EventHandler ScrollPositionChanged;
		public event EventHandler SelectedLineChanged;
		private bool _disableScrollPositionChangedEvent;

		private const float HorizontalScrollFactor = 8;
		private const int CommentSpacingCharCount = 25;

		private TextboxHistory _history = new TextboxHistory();

		private string[] _contents = new string[0];
		private string[] _contentNotes = new string[0];
		private string[] _compareContents = null;
		private int[] _lineNumbers = new int[0];
		private string[] _lineNumberNotes = new string[0];
		private Dictionary<int, int> _lineNumberIndex = new Dictionary<int,int>();
		private bool _showLineNumbers = false;
		private bool _showLineInHex = false;
		private bool _showLineNumberNotes = false;
		private bool _showSingleLineLineNumberNotes = false;
		private bool _showContentNotes = false;
		private bool _showSingleLineContentNotes = true;
		private bool _showCompactPrgAddresses = false;
		private int _selectionStart = 0;
		private int _selectionLength = 0;
		private int _scrollPosition = 0;
		private int _horizontalScrollPosition = 0;
		private string _searchString = null;
		private string _header = null;
		private Font _noteFont = null;
		private int _marginWidth = 9;
		private int _extendedMarginWidth = 16;
		private float _maxLineWidth = 0;
		private int _maxLineWidthIndex = 0;
		private TextboxMessageInfo _message;

		public ctrlTextbox()
		{
			InitializeComponent();
			this.ResizeRedraw = true;
			this.DoubleBuffered = true;
			this.UpdateFont();
		}
		
		public string[] Comments;
		public string[] Addressing;
		public int[] LineIndentations;

		public string[] TextLines
		{
			set
			{
				int maxLength = 0;

				_maxLineWidthIndex = 0;
				
				_contents = value;
				for(int i = 0, len = value.Length; i < len; i++) {
					int length = _contents[i].Length + (Addressing != null ? Addressing[i].Length : 0);
					if(Comments?[i].Length > 0) {
						length = Math.Max(length, length > 0 ? CommentSpacingCharCount : 0) + Comments[i].Length;
					}
					if(length > maxLength) {
						maxLength = length;
						_maxLineWidthIndex = i;
					}
				}

				UpdateHorizontalScrollWidth();

				if(_lineNumbers.Length != _contents.Length) {
					_lineNumbers = new int[_contents.Length];
					_lineNumberIndex.Clear();
					for(int i = _contents.Length - 1; i >=0; i--) {
						_lineNumbers[i] = i;
						_lineNumberIndex[i] = i;
					}
				}
				this.Invalidate();
			}
		}

		//Cache Font.Height value because accessing it is slow
		private new int FontHeight { get; set; }

		private Font _baseFont = new Font(BaseControl.MonospaceFontFamily, BaseControl.DefaultFontSize);
		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public Font BaseFont
		{
			get { return _baseFont; }
			set
			{
				_baseFont = value;
				this.UpdateFont();
				this.Invalidate();
			}
		}

		private Font _font = new Font(BaseControl.MonospaceFontFamily, BaseControl.DefaultFontSize);
		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public override Font Font
		{
			get { return this._font; }
			set { throw new Exception("Do not use"); }
		}

		private int _textZoom = 100;
		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int TextZoom
		{
			get { return this._textZoom; }
			set
			{
				if(value != _textZoom && value >= 30 && value <= 500) {
					this._textZoom = value;
					UpdateFont();
					this.Invalidate();
				}
			}
		}

		private void UpdateFont()
		{
			_font = new Font(this.BaseFont.FontFamily, this.BaseFont.Size * this.TextZoom / 100f, this.BaseFont.Style);
			_noteFont = new Font(this.BaseFont.FontFamily, this.BaseFont.Size * this.TextZoom * 0.75f / 100f);
			FontHeight = this._font.Height;
			UpdateHorizontalScrollWidth();
		}

		public bool ShowSingleContentLineNotes
		{
			get { return _showSingleLineContentNotes; }
			set
			{
				_showSingleLineContentNotes = value;
				this.Invalidate();
			}
		}

		public bool ShowCompactPrgAddresses
		{
			get { return _showCompactPrgAddresses; }
			set
			{
				_showCompactPrgAddresses = value;
				this.Invalidate();
			}
		}

		public bool ShowContentNotes
		{
			get { return _showContentNotes; }
			set 
			{
				_showContentNotes = value;
				this.Invalidate();
			}
		}

		public bool ShowSingleLineLineNumberNotes
		{
			get { return _showSingleLineLineNumberNotes; }
			set
			{
				_showSingleLineLineNumberNotes = value;
				this.Invalidate();
			}
		}

		private bool _showMemoryValues = false;
		public bool ShowMemoryValues
		{
			get { return _showMemoryValues; }
			set
			{
				_showMemoryValues = value;
				this.Invalidate();
			}
		}

		private bool _hideSelection = false;
		public bool HideSelection
		{
			get { return _hideSelection; }
			set
			{
				_hideSelection = value;
				this.Invalidate();
			}
		}

		public bool ShowLineNumberNotes
		{
			get { return this._showLineNumberNotes; }
			set 
			{ 
				this._showLineNumberNotes = value;
				this.Invalidate();
			}
		}

		public string[] TextLineNotes
		{
			set
			{
				this._contentNotes = value;
				this.Invalidate();
			}
		}

		public string[] CompareLines
		{
			set
			{
				_compareContents = value;
			}
		}

		public int LineCount
		{
			get
			{
				return _contents.Length;
			}
		}

		public int[] LineNumbers
		{
			set
			{
				_lineNumbers = value;
				_lineNumberIndex.Clear();
				int i = 0;
				foreach(int line in _lineNumbers) {
					_lineNumberIndex[line] = i;
					i++;
				}
				this.Invalidate();
			}
		}

		public string[] LineNumberNotes
		{
			set
			{
				_lineNumberNotes = value;
				this.Invalidate();
			}
			get
			{
				return _lineNumberNotes;
			}
		}

		public string Header
		{
			set
			{
				this._header = value;
				this.Invalidate();
			}
		}

		public int MarginWidth
		{
			set
			{
				this._marginWidth = value;
				this.Invalidate();
			}
		}

		public bool CodeHighlightingEnabled { get; set; } = true;

		public bool Search(string searchString, bool searchBackwards, bool isNewSearch)
		{
			if(string.IsNullOrWhiteSpace(searchString)) {
				this._searchString = null;
				this.Invalidate();
				return true;
			} else {
				int startPosition;
				int endPosition;

				this._searchString = searchString.ToLowerInvariant();
				int searchOffset = (searchBackwards ? -1 : 1);
				if(isNewSearch) {
					startPosition = this.SelectionStart;
					endPosition = this.SelectionStart - searchOffset;
					if(endPosition < 0) {
						endPosition = _contents.Length - 1;
					} else if(endPosition >= _contents.Length) {
						endPosition = 0;
					}

				} else {
					startPosition = this.SelectionStart + searchOffset;
					endPosition = this.SelectionStart;
					if(startPosition < 0) {
						startPosition = _contents.Length - 1;
					} else if(startPosition >= _contents.Length) {
						startPosition = 0;
					}
				}

				for(int i = startPosition; i != endPosition; i += searchOffset) {
					string line = _contents[i].ToLowerInvariant();
					if(line.Contains(this._searchString)) {
						this.ScrollToLineIndex(i);
						this.Invalidate();
						return true;
					}

					//Continue search from start/end of document
					if(!searchBackwards && i == this._contents.Length - 1) {
						i = 0;
					} else if(searchBackwards && i == 0) {
						i = this._contents.Length - 1;
					}
				}
				this.Invalidate();
				return _contents[_selectionStart].ToLowerInvariant().Contains(this._searchString);
			}
		}

		public interface ILineStyleProvider
		{
			LineProperties GetLineStyle(int cpuAddress, int lineIndex);
			string GetLineComment(int lineIndex);
		}

		private ILineStyleProvider _styleProvider;
		public ILineStyleProvider StyleProvider
		{
			get { return _styleProvider; }
			set
			{
				_styleProvider = value;
				this.Invalidate();
			}
		}

		public LineProperties GetLineStyle(int lineIndex)
		{
			if(StyleProvider != null) {
				return StyleProvider.GetLineStyle(_lineNumbers[lineIndex], lineIndex);
			} else {
				return null;
			}
		}

		public int GetLineIndex(int lineNumber)
		{
			if(_lineNumberIndex.ContainsKey(lineNumber)) {
				return _lineNumberIndex[lineNumber];
			} else {
				foreach(int line in _lineNumbers) {
					if(line > lineNumber) {
						return Math.Max(0, GetLineIndex(line) - 1);
					}
				}
			}
			return -1;
		}

		public int GetLineNumber(int lineIndex)
		{
			if(_lineNumbers.Length <= lineIndex) {
				return 0;
			} else {
				return _lineNumbers[lineIndex];
			}
		}

		public void ScrollToLineIndex(int lineIndex, eHistoryType historyType = eHistoryType.Always, bool scrollToTop = false, bool forceScroll = false)
		{
			bool scrolled = false;

			//TODO
			/*if(!scrollToTop && ConfigManager.Config.DebugInfo.AlwaysScrollToCenter) {
				forceScroll = true;
			}*/

			if(forceScroll || lineIndex < this.ScrollPosition || lineIndex > this.GetLastVisibleLineIndex()) {
				//Line isn't currently visible, scroll it to the middle of the viewport
				if(scrollToTop) {
					int scrollPos = lineIndex;
					while(scrollPos > 0 && _lineNumbers[scrollPos - 1] < 0 && string.IsNullOrWhiteSpace(_lineNumberNotes[scrollPos - 1])) {
						//Make sure any comment for the line is in scroll view
						bool emptyLine = string.IsNullOrWhiteSpace(_contents[scrollPos]) && string.IsNullOrWhiteSpace(this.Comments[scrollPos]);
						if(emptyLine) {
							//If there's a blank line, stop scrolling up
							scrollPos++;
							break;
						}

						scrollPos--;
						if(emptyLine || _contents[scrollPos].StartsWith("--") || _contents[scrollPos].StartsWith("__")) {
							//Reached the start of a block, stop going back up
							break;
						}
					}
					this.ScrollPosition = scrollPos;
				} else {
					this.ScrollPosition = lineIndex - this.GetNumberVisibleLines() / 2;
				}
				scrolled = true;
			}

			if(this.SelectionStart != lineIndex) {
				if(historyType == eHistoryType.Always || scrolled && historyType == eHistoryType.OnScroll) {
					_history.AddHistory(this.SelectionStart);
				}
				this.SelectionStart = lineIndex;
				this.SelectionLength = 0;
				if(historyType == eHistoryType.Always || scrolled && historyType == eHistoryType.OnScroll) {
					_history.AddHistory(this.SelectionStart);
				}
			}
		}

		public void ScrollToLineNumber(int lineNumber, eHistoryType historyType = eHistoryType.Always, bool scrollToTop = false, bool forceScroll = false)
		{
			int lineIndex = this.GetLineIndex(lineNumber);
			if(lineIndex >= 0) {
				ScrollToLineIndex(lineIndex, historyType, scrollToTop, forceScroll);
			}
		}

		public int CodeMargin
		{
			get
			{
				using(Graphics g = Graphics.FromHwnd(this.Handle)) {
					return this.GetMargin(g, false);
				}
			}
		}
		
		private int GetMargin(Graphics g, bool getExtendedMargin)
		{
			int marginWidth = getExtendedMargin && this.ShowContentNotes && this.ShowSingleContentLineNotes ? _marginWidth + _extendedMarginWidth : _marginWidth;
			if(ShowCompactPrgAddresses) {
				marginWidth += 4;
			}
			return (this.ShowLineNumbers ? (int)(g.MeasureString("".PadLeft(marginWidth, 'W'), this.Font, int.MaxValue, StringFormat.GenericTypographic).Width) : 0) - 1;
		}

		public int GetLineIndexAtPosition(int yPos)
		{
			int lineIndex = this.ScrollPosition + this.GetLineAtPosition(yPos);
			if(lineIndex > _contents.Length && _contents.Length != 0) {
				lineIndex = _contents.Length - 1;
			}
			return lineIndex;
		}

		public string GetFullWidthString(int lineIndex)
		{
			string text = _contents[lineIndex] + Addressing?[lineIndex];
			if(Comments?[lineIndex].Length > 0) {
				return text.PadRight(text.Length > 0 ? CommentSpacingCharCount : 0) + Comments[lineIndex];
			}
			return text;
		}

		private bool GetCharIndex(Point position, out int charIndex, out int lineIndex)
		{
			charIndex = int.MaxValue;
			using(Graphics g = Graphics.FromHwnd(this.Handle)) {
				int marginLeft = this.GetMargin(g, true);
				lineIndex = this.GetLineIndexAtPosition(position.Y);
				if(lineIndex >= _contents.Length) {
					return false;
				}

				int positionX = position.X - marginLeft;
				positionX -= (LineIndentations != null ? LineIndentations[lineIndex] : 0);
				if(positionX >= 0) {
					float charWidth = g.MeasureString("W", this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
					charIndex = (int)(positionX / charWidth);
					return true;
				}
			}
			return false;
		}

		char[] _wordDelimiters = new char[] { ' ', ',', '|', ';', '(', ')', '.', '-', ':', '<', '>', '#', '*', '/', '&', '[', ']', '~', '%' };
		public string GetWordUnderLocation(Point position)
		{
			int charIndex; 
			int lineIndex;
			if(this.GetCharIndex(position, out charIndex, out lineIndex)) {
				string text = this.GetFullWidthString(lineIndex);
				if(charIndex < text.Length) {
					if(_wordDelimiters.Contains(text[charIndex])) {
						return string.Empty;
					} else {
						int endIndex = text.IndexOfAny(_wordDelimiters, charIndex);
						if(endIndex == -1) {
							endIndex = text.Length;
						}
						int startIndex = text.LastIndexOfAny(_wordDelimiters, charIndex);

						if(startIndex >= 0 && text[startIndex] == '#' && text.Length > startIndex && text[startIndex + 1] == '$') {
							//Special case for immediate values. e.g: we want to show a tooltip for #MyLabel, but not for #$EF
							return text.Substring(startIndex, endIndex - startIndex);
						} else {
							return text.Substring(startIndex + 1, endIndex - startIndex - 1);
						}
					}
				}
			}
			return string.Empty;
		}

		private int GetLineAtPosition(int yPos)
		{
			if(!string.IsNullOrWhiteSpace(this._header)) {
				yPos -= this.LineHeight;
			}
			return Math.Max(0, yPos / this.LineHeight);
		}

		private int GetLastVisibleLineIndex()
		{
			return this.ScrollPosition + this.GetNumberVisibleLines() - 1;
		}

		public int GetNumberVisibleLines()
		{
			Rectangle rect = this.ClientRectangle;
			return this.GetLineAtPosition(rect.Height);
		}

		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int SelectionStart
		{
			get { return Math.Min(this._contents.Length - 1, Math.Max(0, _selectionStart)); }
			set
			{
				int selectionStart = Math.Max(0, Math.Min(this._contents.Length - 1, Math.Max(0, value)));
				
				_selectionStart = selectionStart;

				if(this.SelectionLength == 0) {
					this.SelectedLine = this.SelectionStart;
				}

				this.Invalidate();
			}
		}

		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int SelectionLength
		{
			get { return (this.SelectionStart + _selectionLength) > this._contents.Length - 1 ? this._contents.Length - this.SelectionStart - 1 : _selectionLength; }
			set
			{
				_selectionLength = value;

				if(this.SelectionStart + _selectionLength > this._contents.Length - 1) {
					_selectionLength = this._contents.Length - this.SelectionStart - 1;
				}

				if(value == 0) {
					this.SelectedLine = this.SelectionStart;
				}

				this.Invalidate();
			}
		}
		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int SelectedLine
		{
			get { return this._selectedLine; }
			set
			{
				this._selectedLine = value;
				if(_selectedLine < this.ScrollPosition) {
					this.ScrollPosition = _selectedLine;
				} else if(_selectedLine > this.GetLastVisibleLineIndex()) {
					this.ScrollPosition = _selectedLine - this.GetNumberVisibleLines() + 1;
				}
				this.SelectedLineChanged?.Invoke(this, EventArgs.Empty);
				this.Invalidate();
			}
		}
		private int _selectedLine = 0;

		public void MoveSelectionDown(int lines = 1)
		{
			_disableScrollPositionChangedEvent = true;
			while(lines > 0) {
				bool singleLineSelection = this.SelectionLength == 0;

				if(singleLineSelection) {
					if(this.SelectionStart + this.SelectionLength >= this._contents.Length - 1) {
						//End of document reached
						break;
					}
					this.SelectedLine = this.SelectionStart + 1;
					this.SelectionLength++;
				} else if(this.SelectionStart + this.SelectionLength == this.SelectedLine) {
					if(this.SelectionStart + this.SelectionLength >= this._contents.Length - 1) {
						//End of document reached
						break;
					}
					this.SelectedLine++;
					this.SelectionLength++;
				} else {
					this.SelectionStart++;
					this.SelectedLine++;
					this.SelectionLength--;
				}
				lines--;
			}
			_disableScrollPositionChangedEvent = false;
			ScrollPositionChanged?.Invoke(this, null);
		}

		public void MoveSelectionUp(int lines = 1)
		{
			_disableScrollPositionChangedEvent = true;
			while(lines > 0) {
				bool singleLineSelection = this.SelectionLength == 0;

				if(singleLineSelection) {
					if(this.SelectionStart == 0) {
						//Top of document reached
						break;
					}
					this.SelectionStart--;
					this.SelectedLine = this.SelectionStart;
					this.SelectionLength++;
				} else if(this.SelectionStart == this.SelectedLine) {
					if(this.SelectionStart == 0) {
						//Top of document reached
						break;
					}
					this.SelectionStart--;
					this.SelectedLine--;
					this.SelectionLength++;
				} else {
					this.SelectedLine--;
					this.SelectionLength--;
				}
				lines--;
			}
			_disableScrollPositionChangedEvent = false;
			ScrollPositionChanged?.Invoke(this, null);
		}

		public int CurrentLine
		{
			get { return _lineNumbers.Length > _selectionStart ? _lineNumbers[_selectionStart] : 0; }
		}

		public int LastSelectedLine
		{
			get { return _lineNumbers.Length > _selectionStart + this.SelectionLength ? _lineNumbers[_selectionStart + this.SelectionLength] : 0; }
		}

		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int ScrollPosition
		{
			get { return _scrollPosition; }
			set 
			{
				value = Math.Max(0, Math.Min(value, this._contents.Length-this.GetNumberVisibleLines()));
				_scrollPosition = value;
				if(!_disableScrollPositionChangedEvent && this.ScrollPositionChanged != null) {
					ScrollPositionChanged(this, null);
				}

				//Clear message if scroll position changes
				this.SetMessage(null);

				this.Invalidate();
			}
		}

		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int HorizontalScrollPosition
		{
			get { return _horizontalScrollPosition; }
			set
			{
				_horizontalScrollPosition = value;
				if(!_disableScrollPositionChangedEvent && this.ScrollPositionChanged != null) {
					ScrollPositionChanged(this, null);
				}
				this.Invalidate();
			}
		}

		public int HorizontalScrollWidth { get; set; } = 0;

		private void UpdateHorizontalScrollWidth()
		{
			if(LineIndentations != null && LineIndentations.Length > _maxLineWidthIndex) {
				using(Graphics g = this.CreateGraphics()) {
					_maxLineWidth = (LineIndentations != null ? LineIndentations[_maxLineWidthIndex] : 0) + g.MeasureString(GetFullWidthString(_maxLineWidthIndex), this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
					HorizontalScrollWidth = (int)(Math.Max(0, HorizontalScrollFactor + _maxLineWidth - (this.Width - GetMargin(g, true))) / HorizontalScrollFactor);
				}
			} else {
				_maxLineWidth = 0;
				HorizontalScrollPosition = 0;
				HorizontalScrollWidth = 0;
			}
		}

		public void SetMessage(TextboxMessageInfo message)
		{
			if(_message != message) {
				_message = message;
				Invalidate();
			}
		}

		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);
			this.ScrollPosition = this.ScrollPosition;
			UpdateHorizontalScrollWidth();
			ScrollPositionChanged?.Invoke(this, e);
		}

		public bool ShowLineNumbers
		{
			get { return _showLineNumbers; }
			set { _showLineNumbers = value; }
		}

		public bool ShowLineInHex
		{
			get { return _showLineInHex; }
			set { _showLineInHex = value; }
		}

		private int LineHeight
		{
			get 
			{
				if(this.ShowLineNumberNotes && !this.ShowSingleLineLineNumberNotes || this.ShowContentNotes && !this.ShowSingleContentLineNotes) {
					return (int)(this.FontHeight * 1.60);
				} else {
					return this.FontHeight - 1;
				}
			}
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			//Clear message if a key is pressed
			this.SetMessage(null);

			return base.ProcessCmdKey(ref msg, keyData);
		}

		int _clickedLine;
		bool _mouseDragging;
		protected override void OnMouseDown(MouseEventArgs e)
		{
			this.Focus();

			//Clear message if a mouse button is clicked
			this.SetMessage(null);

			if(e.Button == MouseButtons.XButton1) {
				this.NavigateBackward();
			} else if(e.Button == MouseButtons.XButton2) {
				this.NavigateForward();
			} else {
				_clickedLine = this.ScrollPosition + this.GetLineAtPosition(e.Y);

				if(e.Button == MouseButtons.Right) {
					if(_clickedLine >= this.SelectionStart && _clickedLine <= this.SelectionStart + this.SelectionLength) {
						//Right-clicking on selection should not change it
						return;
					}
				}

				if(Control.ModifierKeys.HasFlag(Keys.Shift)) {
					if(_clickedLine > this.SelectedLine) {
						MoveSelectionDown(_clickedLine - this.SelectedLine);
					} else {
						MoveSelectionUp(this.SelectedLine - _clickedLine);
					}
				} else {
					_mouseDragging = true;
					this.SelectedLine = _clickedLine;
					this.SelectionStart = _clickedLine;
					this.SelectionLength = 0;
				}
			}
			base.OnMouseDown(e);
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			_mouseDragging = false;
			base.OnMouseUp(e);
		}

		protected override void OnMouseMove(MouseEventArgs e)
		{
			if(_mouseDragging) {
				int lineUnderMouse = this.ScrollPosition + this.GetLineAtPosition(e.Y);
				this.SelectedLine = lineUnderMouse;
				this.SelectedLine = lineUnderMouse;
				if(lineUnderMouse > _clickedLine) {
					this.SelectionLength = lineUnderMouse - _clickedLine;
				} else {
					this.SelectedLine = lineUnderMouse;
					this.SelectionStart = lineUnderMouse;
					this.SelectionLength = _clickedLine - lineUnderMouse;
				}
			}
			base.OnMouseMove(e);
		}

		public void CopySelection(bool copyLineNumbers, bool copyContentNotes, bool copyComments)
		{
			StringBuilder sb = new StringBuilder();
			for(int i = this.SelectionStart, end = this.SelectionStart + this.SelectionLength; i <= end; i++) {
				string indent = "";
				if(LineIndentations != null) {
					indent = "".PadLeft(LineIndentations[i] / 10);
				}

				string codeString = _contents[i].Trim();
				if(codeString.StartsWith("__") || codeString.StartsWith("--")) {
					codeString = "--------" + codeString.Substring(2, codeString.Length - 4) + "--------";
				}

				int padding = Math.Max(CommentSpacingCharCount, codeString.Length);
				if(codeString.Length == 0) {
					padding = 0;
				}

				codeString = codeString.PadRight(padding);

				string line = indent + codeString;
				if(copyContentNotes && _contentNotes[i].Length > 0) {
					line = _contentNotes[i].PadRight(13) + line;
				}
				if(copyLineNumbers && _lineNumbers[i] >= 0) {
					line = _lineNumbers[i].ToString("X4") + "  " + line;
				}
				if(copyComments && !string.IsNullOrWhiteSpace(Comments[i])) {
					line = line + Comments[i];
				}
				sb.AppendLine(line);
			}
			Clipboard.SetText(sb.ToString());
		}

		public void NavigateForward()
		{
			this.ScrollToLineIndex(_history.GoForward(), eHistoryType.None);
		}

		public void NavigateBackward()
		{
			this.ScrollToLineIndex(_history.GoBack(), eHistoryType.None);
		}

		private void DrawLine(Graphics g, int currentLine, int marginLeft, int positionY, int lineHeight)
		{
			string codeString = _contents[currentLine];
			string addressString = this.Addressing?[currentLine];
			string commentString = this.Comments?[currentLine];

			float codeStringLength = g.MeasureString(codeString, this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
			float addressStringLength = g.MeasureString(addressString, this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;

			//Adjust background color highlights based on number of spaces in front of content
			int originalMargin = marginLeft;
			marginLeft += (LineIndentations != null ? LineIndentations[currentLine] : 0);

			bool isBlockStart = codeString.StartsWith("__") && codeString.EndsWith("__");
			bool isBlockEnd = codeString.StartsWith("--") && codeString.EndsWith("--");

			Color? textColor = null;
			LineProperties lineProperties = GetLineStyle(currentLine);

			//Setup text and bg color (only if the line is not the start/end of a block)
			if(lineProperties != null) {
				//Process background, foreground, outline color and line symbol
				textColor = lineProperties.FgColor;

				if(lineProperties.LineBgColor.HasValue) {
					using(Brush bgBrush = new SolidBrush(lineProperties.LineBgColor.Value)) {
						int yOffset = Program.IsMono ? 2 : 1;
						if(isBlockStart) {
							g.FillRectangle(bgBrush, originalMargin, positionY + yOffset + lineHeight / 2, Math.Max(_maxLineWidth + 10, this.ClientRectangle.Width - originalMargin), lineHeight / 2 + 1);
						} else if(isBlockEnd) {
							g.FillRectangle(bgBrush, originalMargin, positionY + yOffset, Math.Max(_maxLineWidth + 10, this.ClientRectangle.Width - originalMargin), lineHeight / 2 - 3);
						} else {
							g.FillRectangle(bgBrush, originalMargin, positionY + yOffset, Math.Max(_maxLineWidth + 10, this.ClientRectangle.Width - originalMargin), lineHeight);
						}
					}
				}
			}

			if(!this.HideSelection && currentLine >= this.SelectionStart && currentLine <= this.SelectionStart + this.SelectionLength) {
				//Highlight current line
				using(Brush brush = new SolidBrush(Color.FromArgb(150, 185, 210, 255))) {
					int offset = currentLine - 1 == this.SelectedLine ? 1 : 0;
					g.FillRectangle(brush, originalMargin, positionY + offset, Math.Max(_maxLineWidth, this.ClientRectangle.Width), lineHeight - offset);
				}
				if(currentLine == this.SelectedLine) {
					g.DrawRectangle(Pens.Blue, originalMargin + 1, positionY + 1, Math.Max(_maxLineWidth, this.ClientRectangle.Width - originalMargin) - 1, lineHeight);
				}
			}

			if(lineProperties != null) {
				if(!isBlockStart && !isBlockEnd && lineProperties.TextBgColor.HasValue) {
					using(Brush bgBrush = new SolidBrush(lineProperties.TextBgColor.Value)) {
						int yOffset = Program.IsMono ? 2 : 1;
						g.FillRectangle(bgBrush, marginLeft, positionY + yOffset, codeStringLength, lineHeight - 1);
					}
				}

				if(!isBlockStart && !isBlockEnd && lineProperties.OutlineColor.HasValue) {
					using(Pen outlinePen = new Pen(lineProperties.OutlineColor.Value, 1)) {
						g.DrawRectangle(outlinePen, marginLeft, positionY + 1, codeStringLength, lineHeight - 1);
					}
				}
			}

			this.DrawLineText(g, currentLine, marginLeft, positionY, codeString, addressString, commentString, codeStringLength, addressStringLength, textColor, lineHeight);
		}

		private void DrawLineNumber(Graphics g, int currentLine, int marginLeft, int positionY, Color addressColor)
		{
			using(Brush numberBrush = new SolidBrush(addressColor)) {
				if(this.ShowLineNumberNotes && this.ShowSingleLineLineNumberNotes) {
					//Display line note instead of line number
					string lineNumber;
					if(string.IsNullOrEmpty(_lineNumberNotes[currentLine])) {
						lineNumber = _lineNumbers[currentLine] >= 0 ? _lineNumbers[currentLine].ToString(_showLineInHex ? "X4" : "") : "..";
					} else {
						lineNumber = _lineNumberNotes[currentLine];
					}
					float width = g.MeasureString(lineNumber, this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
					g.DrawString(lineNumber, this.Font, numberBrush, marginLeft - width, positionY, StringFormat.GenericTypographic);
				} else {
					//Display line number
					string lineNumber = _lineNumbers[currentLine] >= 0 ? _lineNumbers[currentLine].ToString(_showLineInHex ? "X4" : "") : "..";

					if(ShowCompactPrgAddresses) {
						string lineNumberNote = _lineNumberNotes[currentLine];
						if(lineNumberNote.Length > 3) {
							string compactView = lineNumberNote.Substring(0, lineNumberNote.Length - 3).TrimStart('0');
							if(compactView.Length == 0) {
								compactView = "0";
							}
							lineNumber += " [" + compactView + "]";
						}
					}

					float width = g.MeasureString(lineNumber, this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
					g.DrawString(lineNumber, this.Font, numberBrush, marginLeft - width, positionY, StringFormat.GenericTypographic);
					
					if(this.ShowLineNumberNotes && !this.ShowSingleLineLineNumberNotes) {
						//Display line note below line number
						width = g.MeasureString(_lineNumberNotes[currentLine], _noteFont, int.MaxValue, StringFormat.GenericTypographic).Width;
						g.DrawString(_lineNumberNotes[currentLine], _noteFont, numberBrush, marginLeft - width, positionY+this.Font.Size+3, StringFormat.GenericTypographic);
					}
				}
			}
		}

		private void DrawLineText(Graphics g, int currentLine, int marginLeft, int positionY, string codeString, string addressString, string commentString, float codeStringLength, float addressStringLength, Color? textColor, int lineHeight)
		{
			DebugInfo info = ConfigManager.Config.Debug;
			
			if(codeString.StartsWith("__") && codeString.EndsWith("__") || codeString.StartsWith("--") && codeString.EndsWith("--")) {
				//Draw block start/end
				g.TranslateTransform(HorizontalScrollPosition * HorizontalScrollFactor, 0);
				string text = codeString.Substring(2, codeString.Length - 4);
				float yOffset = codeString.StartsWith("__") ? 2 : -2;
				if(text.Length > 0) {
					SizeF size = g.MeasureString(text, this._noteFont, int.MaxValue, StringFormat.GenericTypographic);
					float textLength = size.Width;
					float textHeight = size.Height;
					float positionX = (marginLeft + this.Width - textLength) / 2;
					g.DrawLine(Pens.Black, marginLeft, yOffset + positionY + lineHeight / 2, marginLeft + this.Width, yOffset + positionY + lineHeight / 2);
					yOffset = codeString.StartsWith("__") ? 3 : 2;
					g.FillRectangle(Brushes.White, positionX - 4, yOffset + positionY, textLength + 8, textHeight);
					g.DrawRectangle(Pens.Black, positionX - 4, yOffset + positionY, textLength + 8, textHeight);
					g.DrawString(text, this._noteFont, Brushes.Black, positionX, yOffset + positionY, StringFormat.GenericTypographic);
				} else {
					g.DrawLine(Pens.Black, marginLeft, yOffset + positionY + lineHeight / 2, marginLeft + this.Width, yOffset + positionY + lineHeight / 2);
				}
				g.TranslateTransform(-HorizontalScrollPosition * HorizontalScrollFactor, 0);
			} else {
				if(StyleProvider != null) {
					string symbolComment = StyleProvider.GetLineComment(currentLine);
					if(symbolComment != null) {
						symbolComment = symbolComment.Replace("\t", "  ");
					}

					if(symbolComment != _lastSymbolComment) {
						commentString = symbolComment ?? commentString;
						if(symbolComment != null) {
							_lastSymbolComment = symbolComment;
						}
					}
				}

				//Draw line content
				int characterCount = 0;
				Color defaultColor = Color.FromArgb(60, 60, 60);
				if(codeString.Length > 0) {
					Match match = CodeHighlightingEnabled ? _codeRegex.Match(codeString) : null;
					if(match != null && match.Success && !codeString.EndsWith(":")) {
						string padding = match.Groups[1].Value;
						string opcode = match.Groups[2].Value;
						string invalidStar = match.Groups[3].Value;
						string paren1 = match.Groups[5].Value;
						string operand = match.Groups[6].Value;
						string arrayPosition = match.Groups[12].Value;
						string paren2 = match.Groups[13].Value;
						string indirect = match.Groups[14].Value;
						string paren3 = match.Groups[15].Value;
						string rest = match.Groups[16].Value;
						Color operandColor = operand.Length > 0 ? (operand[0] == '#' ? (Color)info.CodeImmediateColor : (operand[0] == '$' ? (Color)info.CodeAddressColor : (Color)info.CodeLabelDefinitionColor)) : Color.Black;
						List<Color> colors = new List<Color>() { defaultColor, info.CodeOpcodeColor, defaultColor, defaultColor, defaultColor, operandColor, defaultColor, defaultColor, defaultColor };
						int codePartCount = colors.Count;

						List<string> parts = new List<string>() { padding, opcode, invalidStar, " ", paren1, operand, paren2, indirect, paren3 };
						string memoryAddress = "";
						if(!string.IsNullOrWhiteSpace(addressString)) {
							colors.Add(info.CodeEffectiveAddressColor);
							parts.Add(addressString);
							memoryAddress = addressString.Substring(3).Trim();

							//Update the contents of "arrayPosition" based on the address string, rather than use the one from the original address
							int plusIndex = memoryAddress.IndexOf("+");
							arrayPosition = plusIndex >= 0 ? memoryAddress.Substring(plusIndex+1) : "";
						} else if(operand.Length > 0 && operand[0] != '#') {
							memoryAddress = operand.Trim();
						}

						if(this.ShowMemoryValues && memoryAddress.Length > 0) {
							int address = -1;
							if(memoryAddress[0] == '$') {
								Int32.TryParse(memoryAddress.Substring(1), System.Globalization.NumberStyles.AllowHexSpecifier, null, out address);
							} else {
								//Label
								UInt32 arrayOffset = 0;
								if(!string.IsNullOrWhiteSpace(arrayPosition)) {
									memoryAddress = memoryAddress.Substring(0, memoryAddress.Length - arrayPosition.Length - 1);
									arrayOffset = UInt32.Parse(arrayPosition);
								}

								//TODO
								/*CodeLabel label = LabelManager.GetLabel(memoryAddress);
								if(label != null) {
									address = InteropEmu.DebugGetRelativeAddress(label.Address + arrayOffset, label.AddressType);
								}*/
							}

							//TODO
							/*
							if(address >= 0) {
								colors.Add(defaultColor);
								parts.Add(" = $" + InteropEmu.DebugGetMemoryValue(DebugMemoryType.CpuMemory, (UInt32)address).ToString("X2"));
							}
							*/
						}

						//Display the rest of the line (used by trace logger)
						colors.Add(defaultColor);
						parts.Add(rest);

						float xOffset = 0;
						for(int i = 0; i < parts.Count; i++) {
							using(Brush b = new SolidBrush(textColor.HasValue && (i < codePartCount || i == parts.Count - 1) ? textColor.Value : colors[i])) {
								g.DrawString(parts[i], this.Font, b, marginLeft + xOffset, positionY, StringFormat.GenericTypographic);
								xOffset += g.MeasureString("".PadLeft(parts[i].Length, 'w'), this.Font, Point.Empty, StringFormat.GenericTypographic).Width;
								characterCount += parts[i].Length;
							}
						}
						codeStringLength = xOffset;
					} else {
						using(Brush fgBrush = new SolidBrush(codeString.EndsWith(":") ? (Color)info.CodeLabelDefinitionColor : (textColor ?? defaultColor))) {
							g.DrawString(codeString, this.Font, fgBrush, marginLeft, positionY, StringFormat.GenericTypographic);
						}
						characterCount = codeString.Trim().Length;
					}
				}

				if(!string.IsNullOrWhiteSpace(commentString)) {
					using(Brush commentBrush = new SolidBrush(info.CodeCommentColor)) {
						int padding = Math.Max(CommentSpacingCharCount, characterCount + 1);
						if(characterCount == 0) {
							//Draw comment left-aligned, next to the margin when there is no code on the line
							padding = 0;
						}
						g.DrawString(commentString.PadLeft(padding+commentString.Length), this.Font, commentBrush, marginLeft, positionY, StringFormat.GenericTypographic);
					}
				}

				if(this.ShowContentNotes && !this.ShowSingleContentLineNotes) {
					g.DrawString(_contentNotes[currentLine], _noteFont, Brushes.Gray, marginLeft, positionY + this.Font.Size+3, StringFormat.GenericTypographic);
				}
				this.DrawHighlightedSearchString(g, codeString, marginLeft, positionY);
				this.DrawHighlightedCompareString(g, codeString, currentLine, marginLeft, positionY);
			}
		}
		string _lastSymbolComment = null;

		private void DrawLineSymbols(Graphics g, int positionY, LineProperties lineProperties, int lineHeight)
		{
			int circleSize = lineHeight - 3;
			if((circleSize % 2) == 1) {
				circleSize++;
			}
			int circleOffsetY = positionY + 4;
			int circleOffsetX = 3;

			Action<Brush> drawPlus = (Brush b) => {
				float barWidth = 2;
				float centerPoint = circleSize / 2.0f - barWidth / 2.0f;
				float barLength = circleSize - 6;
				if((barLength % 2) == 1) {
					barLength++;
				}
				float startOffset = (circleSize - barLength) / 2.0f;

				g.FillRectangle(b, circleOffsetX + startOffset, circleOffsetY + centerPoint, barLength, barWidth);
				g.FillRectangle(b, circleOffsetX + centerPoint, circleOffsetY + startOffset, barWidth, barLength);
			};

			if(lineProperties.Symbol.HasFlag(LineSymbol.Circle)) {
				using(Brush brush = new SolidBrush(lineProperties.OutlineColor.Value)) {
					g.FillEllipse(brush, 3, circleOffsetY, circleSize, circleSize);
				}
				if(lineProperties.Symbol.HasFlag(LineSymbol.Plus)) {
					drawPlus(Brushes.White);
				}
			} else if(lineProperties.Symbol.HasFlag(LineSymbol.CircleOutline) && lineProperties.OutlineColor.HasValue) {
				using(Pen pen = new Pen(lineProperties.OutlineColor.Value, 1)) {
					g.DrawEllipse(pen, 3, circleOffsetY, circleSize, circleSize);
					if(lineProperties.Symbol.HasFlag(LineSymbol.Plus)) {
						using(Brush b = new SolidBrush(lineProperties.OutlineColor.Value)) {
							drawPlus(b);
						}
					}
				}
			}

			if(lineProperties.Symbol.HasFlag(LineSymbol.Mark)) {
				using(Brush b = new SolidBrush(ConfigManager.Config.Debug.EventViewerBreakpointColor)) {
					g.FillEllipse(b, circleOffsetX + circleSize * 3 / 4, positionY + 1, lineHeight / 2.0f, lineHeight / 2.0f);
				}
				g.DrawEllipse(Pens.Black, circleOffsetX + circleSize * 3 / 4, positionY + 1, lineHeight / 2.0f, lineHeight / 2.0f);
			}

			if(lineProperties.Symbol.HasFlag(LineSymbol.Arrow)) {
				if(Program.IsMono) {
					int arrowY = positionY + lineHeight / 2 + 1;
					using(Brush brush = new SolidBrush(lineProperties.TextBgColor.Value)) {
						g.FillRectangle(brush, 1, arrowY - lineHeight * 0.25f / 2, lineHeight - 1, lineHeight * 0.35f); 
					}
					g.DrawRectangle(Pens.Black, 1, arrowY - lineHeight * 0.25f / 2, lineHeight - 1, lineHeight * 0.35f); 
				} else {
					float arrowY = circleOffsetY + circleSize / 2.0f;
					g.TranslateTransform(2, 0);
					using(Pen pen = new Pen(Color.Black, lineHeight * 0.33f)) {
						//Outline
						g.DrawLine(pen, 3, arrowY, 3 + lineHeight * 0.25f, arrowY);
						pen.EndCap = System.Drawing.Drawing2D.LineCap.ArrowAnchor;
						g.DrawLine(pen, 3 + lineHeight * 0.25f, arrowY, 3 + lineHeight * 0.75f, arrowY);

						//Fill
						pen.Width-=2f;
						pen.Color = lineProperties.TextBgColor.Value;
						pen.EndCap = System.Drawing.Drawing2D.LineCap.Square;
						g.DrawLine(pen, 4, arrowY, 3 + lineHeight * 0.25f - 1, arrowY);
						pen.EndCap = System.Drawing.Drawing2D.LineCap.ArrowAnchor;
						g.DrawLine(pen, 3 + lineHeight * 0.25f, arrowY, lineHeight * 0.75f + 1, arrowY);
					}
					g.TranslateTransform(-2, 0);
				}
			}
		}

		private void DrawLineProgress(Graphics g, int positionY, LineProgress progress, int lineHeight)
		{
			if(progress != null) {
				int currentStep = progress.Current + 1;
				int stepCount = Math.Max(progress.Maxixum, currentStep);

				string display = currentStep.ToString() + "/" + stepCount.ToString() + " " + progress.Text;
				SizeF size = g.MeasureString(display, this._noteFont, int.MaxValue, StringFormat.GenericTypographic);

				float width = size.Width + 16;
				float left = this.ClientSize.Width - 5 - width;
				float height = size.Height;
				float top = positionY + (lineHeight - height) / 2;

				g.FillRectangle(Brushes.White, left, top, width, height);
				using(SolidBrush brush = new SolidBrush(progress.Color)) {
					g.FillRectangle(brush, left, top, width * currentStep / stepCount, height);
				}
				g.DrawRectangle(Pens.Black, left, top, width, height);

				g.DrawString(display, this._noteFont, Brushes.Black, left + (width - size.Width) / 2, top, StringFormat.GenericTypographic);
			}
		}

		private void DrawHighlightedCompareString(Graphics g, string lineText, int currentLine, int marginLeft, int positionY)
		{
			if(_compareContents != null && _compareContents.Length > currentLine) {
				string compareText = _compareContents[currentLine];

				if(compareText != lineText) {
					StringBuilder sb = new StringBuilder();
					for(int i = 0, len = lineText.Length; i < len; i++) {
						if(lineText[i] == compareText[i]) {
							sb.Append(" ");
						} else {
							sb.Append(lineText[i]);
						}
					}

					g.DrawString(sb.ToString(), new Font(this.Font, FontStyle.Bold), Brushes.Red, marginLeft, positionY, StringFormat.GenericTypographic);
				}
			}
		}

		private void DrawHighlightedSearchString(Graphics g, string lineText, int marginLeft, int positionY)
		{
			int searchIndex;
			if(!string.IsNullOrWhiteSpace(this._searchString) && (searchIndex = lineText.ToLowerInvariant().IndexOf(this._searchString)) >= 0) {
				//Draw colored search string
				string lowerCaseText = lineText.ToLowerInvariant();

				Action<bool> draw = (bool forBackground) => {
					int index = searchIndex;
					do {
						string padding = string.Empty.PadLeft(index, 'A');
						string highlightedText = lineText.Substring(index, this._searchString.Length);
						index = lowerCaseText.IndexOf(this._searchString, index + this._searchString.Length);

						SizeF size = g.MeasureString(highlightedText, this.Font, int.MaxValue, StringFormat.GenericTypographic);
						SizeF offsetSize = g.MeasureString(padding, this.Font, int.MaxValue, StringFormat.GenericTypographic);
						if(forBackground) {
							g.FillRectangle(Brushes.CornflowerBlue, marginLeft + offsetSize.Width, positionY + 1, size.Width + 1, size.Height - 2);
						} else {
							g.DrawString(highlightedText, this.Font, Brushes.White, marginLeft + offsetSize.Width + 1, positionY, StringFormat.GenericTypographic);
						}
					} while(index >= 0);
				};

				draw(true);
				draw(false);
			}
		}

		private void DrawMargin(Graphics g, int currentLine, int marginLeft, int regularMargin, int positionY, int lineHeight)
		{
			LineProperties lineProperties = GetLineStyle(currentLine);

			//Draw instruction progress here to avoid it being scrolled horizontally when window is small (or comments/etc exist)
			if(lineProperties?.Progress != null) {
				this.DrawLineProgress(g, positionY, lineProperties?.Progress, lineHeight);
			} else {
				this.DrawSelectionLength(g, currentLine, positionY, lineHeight);
			}

			if(this.ShowLineNumbers) {
				//Show line number
				Color lineNumberColor = lineProperties != null && lineProperties.AddressColor.HasValue ? lineProperties.AddressColor.Value : Color.Gray;
				this.DrawLineNumber(g, currentLine, regularMargin, positionY, lineNumberColor);
			}
			if(this.ShowContentNotes && this.ShowSingleContentLineNotes) {
				g.DrawString(_contentNotes[currentLine], this.Font, Brushes.Gray, regularMargin + 6, positionY, StringFormat.GenericTypographic);
			}

			//Adjust background color highlights based on number of spaces in front of content
			marginLeft += (LineIndentations != null ? LineIndentations[currentLine] : 0);

			if(lineProperties != null) {
				this.DrawLineSymbols(g, positionY, lineProperties, lineHeight);
			}
		}

		private void DrawSelectionLength(Graphics g, int currentLine, int positionY, int lineHeight)
		{
			if(ConfigManager.Config.Debug.ShowSelectionLength && currentLine == this.SelectedLine && this.SelectionLength > 0) {
				int startAddress = -1;
				int endAddress = -1;

				int end = this.SelectionStart + this.SelectionLength + 1;
				while(endAddress < 0 && end < _lineNumbers.Length) {
					endAddress = _lineNumbers[end];
					end++;
				}

				int start = this.SelectionStart;
				while(startAddress < 0 && start < _lineNumbers.Length && start < end) {
					startAddress = _lineNumbers[start];
					start++;
				}

				if(startAddress >= 0 && endAddress > startAddress) {
					string text = (endAddress - startAddress).ToString() + " bytes";
					SizeF textSize = g.MeasureString(text, this._noteFont);
					PointF position = new PointF(this.ClientSize.Width - textSize.Width - 5, positionY + 3);
					g.FillRectangle(Brushes.White, position.X, position.Y, textSize.Width, lineHeight - 4);
					g.DrawRectangle(Pens.Black, position.X, position.Y, textSize.Width, lineHeight - 4);
					g.DrawString(text, this._noteFont, Brushes.Black, position.X, position.Y - 1);
				}
			}
		}

		private void DrawMessage(Graphics g)
		{
			if(this._message != null && !string.IsNullOrWhiteSpace(this._message.Message)) {
				//Display message if one is active
				SizeF textSize = g.MeasureString(this._message.Message, this.Font, int.MaxValue, StringFormat.GenericTypographic);

				using(SolidBrush brush = new SolidBrush(Color.FromArgb(255, 246, 168))) {
					g.FillRectangle(brush, ClientRectangle.Width - textSize.Width - 10, ClientRectangle.Bottom - textSize.Height - 10, textSize.Width + 4, textSize.Height + 1);
				}
				g.DrawRectangle(Pens.Black, ClientRectangle.Width - textSize.Width - 10, ClientRectangle.Bottom - textSize.Height - 10, textSize.Width + 4, textSize.Height + 1);
				g.DrawString(this._message.Message, this.Font, Brushes.Black, ClientRectangle.Width - textSize.Width - 8, ClientRectangle.Bottom - textSize.Height - 10, StringFormat.GenericTypographic);
			}
		}

		protected override void OnPaint(PaintEventArgs pe)
		{
			_lastSymbolComment = null;
			int lineHeight = this.LineHeight;
			pe.Graphics.PixelOffsetMode = PixelOffsetMode.HighQuality;
			Rectangle rect = this.ClientRectangle;
			pe.Graphics.FillRectangle(Brushes.White, rect);

			pe.Graphics.TranslateTransform(-HorizontalScrollPosition * HorizontalScrollFactor, 0);

			int marginLeft = this.GetMargin(pe.Graphics, true);
			int regularMargin = this.GetMargin(pe.Graphics, false);
			int currentLine = this.ScrollPosition;
			int positionY = 0;

			if(!string.IsNullOrWhiteSpace(this._header)) {
				using(Brush lightGrayBrush = new SolidBrush(Color.FromArgb(240, 240, 240))) {
					pe.Graphics.FillRectangle(lightGrayBrush, marginLeft, 0, Math.Max(_maxLineWidth, rect.Right), lineHeight);
				}
				pe.Graphics.DrawString(_header, this.Font, Brushes.Gray, marginLeft, positionY, StringFormat.GenericTypographic);
				positionY += lineHeight;
			}

			while(positionY < rect.Bottom && currentLine < _contents.Length) {
				this.DrawLine(pe.Graphics, currentLine, marginLeft, positionY, lineHeight);
				positionY += lineHeight;
				currentLine++;
			}

			pe.Graphics.TranslateTransform(HorizontalScrollPosition * HorizontalScrollFactor, 0);

			if(this.ShowLineNumbers) {
				using(Brush brush = new SolidBrush(Color.FromArgb(235, 235, 235))) {
					pe.Graphics.FillRectangle(brush, 0, 0, regularMargin, rect.Bottom);
				}
				using(Brush brush = new SolidBrush(Color.FromArgb(251, 251, 251))) {
					pe.Graphics.FillRectangle(brush, regularMargin, 0, marginLeft - regularMargin, rect.Bottom);
				}

				using(Pen pen = new Pen(Color.LightGray)) {
					pe.Graphics.DrawLine(pen, regularMargin, rect.Top, regularMargin, rect.Bottom);
					pe.Graphics.DrawLine(pen, marginLeft, rect.Top, marginLeft, rect.Bottom);
				}
			}

			currentLine = this.ScrollPosition;
			positionY = string.IsNullOrWhiteSpace(this._header) ? 0 : lineHeight;
			while(positionY < rect.Bottom && currentLine < _contents.Length) {
				this.DrawMargin(pe.Graphics, currentLine, marginLeft, regularMargin, positionY, lineHeight);
				positionY += lineHeight;
				currentLine++;
			}

			this.DrawMessage(pe.Graphics);
		}
	}

	[Flags]
	public enum LineSymbol
	{
		None = 0,
		Circle = 1,
		CircleOutline = 2,
		Arrow = 4,
		Mark = 8,
		Plus = 16
	}

	public enum eHistoryType
	{
		Always,
		OnScroll,
		None
	}

	public class LineProperties
	{
		public Color? LineBgColor;
		public Color? TextBgColor;
		public Color? FgColor;
		public Color? OutlineColor;
		public Color? AddressColor;
		public LineSymbol Symbol;

		public LineProgress Progress;
	}

	public class LineProgress
	{
		public int Current;
		public int Maxixum;
		public string Text;
		public Color Color;
	}

	public class TextboxMessageInfo
	{
		public string Message;
	}
}
