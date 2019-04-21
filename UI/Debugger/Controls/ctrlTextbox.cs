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
		private Regex _codeRegex = new Regex("^(\\s*)([a-z]{3})([*]{0,1})($|[ ]){1}([(\\[]{0,1})(([$][0-9a-f]*)|(#[@$:_0-9a-z]*)|([@_a-z]([@_a-z0-9])*){0,1}(\\+(\\d+)){0,1}){0,1}([)\\]]{0,1})(,X|,Y){0,1}([)\\]]{0,1})(.*)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		public event EventHandler ScrollPositionChanged;
		public event EventHandler SelectedLineChanged;
		private bool _disableScrollPositionChangedEvent;

		private const float HorizontalScrollFactor = 8;
		private const int CommentSpacingCharCount = 25;

		private TextboxHistory _history = new TextboxHistory();

		private ICodeDataProvider _dataProvider;

		private bool _showLineNumbers = false;
		private bool _showAbsoluteAddresses = false;
		private bool _showSingleLineLineNumberNotes = false;
		private bool _showContentNotes = false;
		private bool _showSingleLineContentNotes = true;
		private bool _showCompactPrgAddresses = false;

		private int _selectionStart = 0;
		private int _selectionLength = 0;

		private int _scrollPosition = 0;
		private int _horizontalScrollPosition = 0;

		private string _searchString = null;
		private Font _noteFont = null;
		private int _marginWidth = 9;
		private int _extendedMarginWidth = 16;
		private string _addressFormat = "X6";
		private float _maxLineWidth = 0;
		private TextboxMessageInfo _message;

		public ctrlTextbox()
		{
			InitializeComponent();
			this.ResizeRedraw = true;
			this.DoubleBuffered = true;
			this.UpdateFont();
		}

		public ICodeDataProvider DataProvider
		{
			set
			{
				this._dataProvider = value;

				int lineCount = this._dataProvider.GetLineCount();
				if(this.SelectedLine >= lineCount) {
					this.SelectedLine = lineCount - 1;
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

		public bool ShowByteCode
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

		public bool ShowAbsoluteAddreses
		{
			get { return this._showAbsoluteAddresses; }
			set 
			{ 
				this._showAbsoluteAddresses = value;
				this.Invalidate();
			}
		}

		public int LineCount
		{
			get
			{
				return _dataProvider.GetLineCount();
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

		public int ExtendedMarginWidth
		{
			set
			{
				this._extendedMarginWidth = value;
				this.Invalidate();
			}
		}
		
		public int AddressSize
		{
			set
			{
				this._addressFormat = "X" + value.ToString();
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
						endPosition = this.LineCount - 1;
					} else if(endPosition >= this.LineCount) {
						endPosition = 0;
					}

				} else {
					startPosition = this.SelectionStart + searchOffset;
					endPosition = this.SelectionStart;
					if(startPosition < 0) {
						startPosition = this.LineCount - 1;
					} else if(startPosition >= this.LineCount) {
						startPosition = 0;
					}
				}

				if(_dataProvider.UseOptimizedSearch) {
					Int32 lineIndex = _dataProvider.GetNextResult(searchString, startPosition, endPosition, searchBackwards);
					if(lineIndex >= 0) {
						this.ScrollToLineIndex(lineIndex);
						this.Invalidate();
						return true;
					}
					return false;
				} else {
					for(int i = startPosition; i != endPosition; i += searchOffset) {
						string line = _dataProvider.GetCodeLineData(i).Text.ToLowerInvariant();
						if(line.Contains(this._searchString)) {
							this.ScrollToLineIndex(i);
							this.Invalidate();
							return true;
						}

						//Continue search from start/end of document
						if(!searchBackwards && i == this.LineCount - 1) {
							i = 0;
						} else if(searchBackwards && i == 0) {
							i = this.LineCount - 1;
						}
					}
					this.Invalidate();
					return _dataProvider.GetCodeLineData(_selectionStart).Text.ToLowerInvariant().Contains(this._searchString);
				}
			}
		}

		public interface ILineStyleProvider
		{
			LineProperties GetLineStyle(CodeLineData lineData, int lineIndex);
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
				return StyleProvider.GetLineStyle(_dataProvider.GetCodeLineData(lineIndex), lineIndex);
			} else {
				return null;
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
					CodeLineData lineData = _dataProvider.GetCodeLineData(scrollPos);
					while(scrollPos > 0 && lineData.Address < 0 && lineData.AbsoluteAddress < 0) {
						//Make sure any comment for the line is in scroll view
						bool emptyLine = string.IsNullOrWhiteSpace(lineData.Text) && string.IsNullOrWhiteSpace(lineData.Comment);
						if(emptyLine) {
							//If there's a blank line, stop scrolling up
							scrollPos++;
							break;
						}

						scrollPos--;
						_dataProvider.GetCodeLineData(scrollPos);
						if(emptyLine || lineData.Flags.HasFlag(LineFlags.BlockEnd) || lineData.Flags.HasFlag(LineFlags.BlockStart)) {
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

		public void ScrollToAddress(int address, eHistoryType historyType = eHistoryType.Always, bool scrollToTop = false, bool forceScroll = false)
		{
			int lineIndex = _dataProvider.GetLineIndex((UInt32)address);
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
			int marginWidth = getExtendedMargin && this.ShowByteCode && this.ShowSingleContentLineNotes ? _marginWidth + _extendedMarginWidth : _marginWidth;
			if(ShowCompactPrgAddresses) {
				marginWidth += 4;
			}
			return (this.ShowLineNumbers ? (int)(g.MeasureString("".PadLeft(marginWidth, 'W'), this.Font, int.MaxValue, StringFormat.GenericTypographic).Width) : 0) - 1;
		}

		public int GetLineIndexAtPosition(int yPos)
		{
			if(this._dataProvider == null) {
				return 0;
			}

			int lineIndex = this.ScrollPosition + this.GetLineAtPosition(yPos);
			if(lineIndex > this.LineCount && this.LineCount != 0) {
				lineIndex = this.LineCount - 1;
			}
			return lineIndex;
		}

		public string GetFullWidthString(int lineIndex)
		{
			CodeLineData lineData = _dataProvider.GetCodeLineData(lineIndex);
			string text = lineData.Text + lineData.GetEffectiveAddressString(_addressFormat);
			if(lineData.Comment.Length > 0) {
				return text.PadRight(text.Length > 0 ? CommentSpacingCharCount : 0) + lineData.Comment;
			}
			return text;
		}

		private bool GetCharIndex(Point position, out int charIndex, out int lineIndex)
		{
			charIndex = int.MaxValue;
			using(Graphics g = Graphics.FromHwnd(this.Handle)) {
				int marginLeft = this.GetMargin(g, true);
				lineIndex = this.GetLineIndexAtPosition(position.Y);
				if(lineIndex >= this.LineCount) {
					return false;
				}

				int positionX = position.X - marginLeft;
				//TODO
				//positionX -= (LineIndentations != null ? LineIndentations[lineIndex] : 0);
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
			get { return Math.Min(this.LineCount - 1, Math.Max(0, _selectionStart)); }
			set
			{
				int selectionStart = Math.Max(0, Math.Min(this.LineCount - 1, Math.Max(0, value)));
				
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
			get { return (this.SelectionStart + _selectionLength) > this.LineCount - 1 ? this.LineCount - this.SelectionStart - 1 : _selectionLength; }
			set
			{
				_selectionLength = value;

				if(this.SelectionStart + _selectionLength > this.LineCount - 1) {
					_selectionLength = this.LineCount - this.SelectionStart - 1;
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
					if(this.SelectionStart + this.SelectionLength >= this.LineCount - 1) {
						//End of document reached
						break;
					}
					this.SelectedLine = this.SelectionStart + 1;
					this.SelectionLength++;
				} else if(this.SelectionStart + this.SelectionLength == this.SelectedLine) {
					if(this.SelectionStart + this.SelectionLength >= this.LineCount - 1) {
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
			get { return this.LineCount > _selectionStart ? _dataProvider.GetLineAddress(_selectionStart) : 0; }
		}

		public int LastSelectedLine
		{
			get { return this.LineCount > _selectionStart + this.SelectionLength ? _dataProvider.GetLineAddress(_selectionStart + this.SelectionLength) : 0; }
		}

		[Browsable(false)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int ScrollPosition
		{
			get { return _scrollPosition; }
			set 
			{
				value = Math.Max(0, Math.Min(value, this.LineCount-this.GetNumberVisibleLines()));
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
			//TODO
			/*if(LineIndentations != null && LineIndentations.Length > _maxLineWidthIndex) {
				using(Graphics g = this.CreateGraphics()) {
					_maxLineWidth = (LineIndentations != null ? LineIndentations[_maxLineWidthIndex] : 0) + g.MeasureString(GetFullWidthString(_maxLineWidthIndex), this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
					HorizontalScrollWidth = (int)(Math.Max(0, HorizontalScrollFactor + _maxLineWidth - (this.Width - GetMargin(g, true))) / HorizontalScrollFactor);
				}
			} else {
				_maxLineWidth = 0;
				HorizontalScrollPosition = 0;
				HorizontalScrollWidth = 0;
			}*/
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
			if(_dataProvider == null) {
				return;
			}

			this.ScrollPosition = this.ScrollPosition;
			UpdateHorizontalScrollWidth();
			ScrollPositionChanged?.Invoke(this, e);
		}

		public bool ShowLineNumbers
		{
			get { return _showLineNumbers; }
			set { _showLineNumbers = value; }
		}

		private int LineHeight
		{
			get 
			{
				if(this.ShowAbsoluteAddreses && !this.ShowSingleLineLineNumberNotes || this.ShowByteCode && !this.ShowSingleContentLineNotes) {
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
				CodeLineData lineData = _dataProvider.GetCodeLineData(i);

				string indent = "";
				indent = "".PadLeft(lineData.Indentation / 10);

				string codeString = lineData.Text.Trim();
				if(lineData.Flags.HasFlag(LineFlags.BlockEnd) || lineData.Flags.HasFlag(LineFlags.BlockStart)) {
					codeString = "--------" + codeString + "--------";
				}

				int padding = Math.Max(CommentSpacingCharCount, codeString.Length);
				if(codeString.Length == 0) {
					padding = 0;
				}

				codeString = codeString.PadRight(padding);

				string line = indent + codeString;
				if(copyContentNotes && lineData.ByteCode.Length > 0) {
					line = lineData.ByteCode.PadRight(13) + line;
				}
				if(copyLineNumbers && lineData.Address >= 0) {
					line = lineData.Address.ToString("X4") + "  " + line;
				}
				if(copyComments && !string.IsNullOrWhiteSpace(lineData.Comment)) {
					line = line + lineData.Comment;
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
			CodeLineData lineData = _dataProvider.GetCodeLineData(currentLine);
			string codeString = lineData.Text;
			float codeStringLength = g.MeasureString(codeString, this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;

			//Adjust background color highlights based on number of spaces in front of content
			int originalMargin = marginLeft;
			marginLeft += lineData.Indentation;

			bool isBlockStart = lineData.Flags.HasFlag(LineFlags.BlockStart);
			bool isBlockEnd = lineData.Flags.HasFlag(LineFlags.BlockEnd);

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

			this.DrawLineText(g, currentLine, marginLeft, positionY, lineData, codeStringLength, textColor, lineHeight);
		}

		private void DrawLineNumber(Graphics g, CodeLineData lineData, int marginLeft, int positionY, Color addressColor)
		{
			using(Brush numberBrush = new SolidBrush(addressColor)) {
				if(this.ShowAbsoluteAddreses && this.ShowSingleLineLineNumberNotes) {
					//Display line note instead of line number
					string lineNumber;
					if(lineData.AbsoluteAddress == 0) {
						lineNumber = lineData.Address >= 0 ? lineData.Address.ToString("X6") : "..";
					} else {
						lineNumber = lineData.AbsoluteAddress.ToString("X6");
					}
					float width = g.MeasureString(lineNumber, this.Font, int.MaxValue, StringFormat.GenericTypographic).Width;
					g.DrawString(lineNumber, this.Font, numberBrush, marginLeft - width, positionY, StringFormat.GenericTypographic);
				} else {
					//Display line number
					string lineNumber = lineData.Address >= 0 ? lineData.Address.ToString(_addressFormat) : "..";

					if(ShowCompactPrgAddresses) {
						string lineNumberNote = lineData.AbsoluteAddress.ToString("X6");
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
					
					if(this.ShowAbsoluteAddreses && !this.ShowSingleLineLineNumberNotes) {
						//Display line note below line number
						string absAddress = lineData.AbsoluteAddress.ToString("X6");
						width = g.MeasureString(absAddress, _noteFont, int.MaxValue, StringFormat.GenericTypographic).Width;
						g.DrawString(absAddress, _noteFont, numberBrush, marginLeft - width, positionY+this.Font.Size+3, StringFormat.GenericTypographic);
					}
				}
			}
		}

		private void DrawLineText(Graphics g, int currentLine, int marginLeft, int positionY, CodeLineData lineData, float codeStringLength, Color? textColor, int lineHeight)
		{
			string codeString = lineData.Text;
			string commentString = lineData.Comment;

			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;
			
			if(lineData.Flags.HasFlag(LineFlags.BlockEnd) || lineData.Flags.HasFlag(LineFlags.BlockStart)) {
				//Draw block start/end
				g.TranslateTransform(HorizontalScrollPosition * HorizontalScrollFactor, 0);
				float yOffset = lineData.Flags.HasFlag(LineFlags.BlockStart) ? 2 : -2;
				if(codeString.Length > 0) {
					SizeF size = g.MeasureString(codeString, this._noteFont, int.MaxValue, StringFormat.GenericTypographic);
					float textLength = size.Width;
					float textHeight = size.Height;
					float positionX = (marginLeft + this.Width - textLength) / 2;
					g.DrawLine(Pens.Black, marginLeft, yOffset + positionY + lineHeight / 2, marginLeft + this.Width, yOffset + positionY + lineHeight / 2);
					yOffset = lineData.Flags.HasFlag(LineFlags.BlockStart) ? 3 : 2;
					g.FillRectangle(Brushes.White, positionX - 4, yOffset + positionY, textLength + 8, textHeight);
					g.DrawRectangle(Pens.Black, positionX - 4, yOffset + positionY, textLength + 8, textHeight);
					g.DrawString(codeString, this._noteFont, Brushes.Black, positionX, yOffset + positionY, StringFormat.GenericTypographic);
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
						Color operandColor = operand.Length > 0 ? (operand[0] == '#' ? (Color)cfg.CodeImmediateColor : (operand[0] == '$' ? (Color)cfg.CodeAddressColor : (Color)cfg.CodeLabelDefinitionColor)) : Color.Black;
						List<Color> colors = new List<Color>() { defaultColor, cfg.CodeOpcodeColor, defaultColor, defaultColor, defaultColor, operandColor, defaultColor, defaultColor, defaultColor };
						int codePartCount = colors.Count;

						List<string> parts = new List<string>() { padding, opcode, invalidStar, " ", paren1, operand, paren2, indirect, paren3 };

						if(lineData.EffectiveAddress >= 0) {
							colors.Add(cfg.CodeEffectiveAddressColor);
							parts.Add(" [" + lineData.EffectiveAddress.ToString(_addressFormat) + "]");
						}

						if(this.ShowMemoryValues && lineData.ValueSize > 0) {
							colors.Add(defaultColor);
							parts.Add(lineData.GetValueString());
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
						using(Brush fgBrush = new SolidBrush(codeString.EndsWith(":") ? (Color)cfg.CodeLabelDefinitionColor : (textColor ?? defaultColor))) {
							g.DrawString(codeString, this.Font, fgBrush, marginLeft, positionY, StringFormat.GenericTypographic);
						}
						characterCount = codeString.Trim().Length;
					}
				}

				if(!string.IsNullOrWhiteSpace(commentString)) {
					using(Brush commentBrush = new SolidBrush(cfg.CodeCommentColor)) {
						int padding = Math.Max(CommentSpacingCharCount, characterCount + 1);
						if(characterCount == 0) {
							//Draw comment left-aligned, next to the margin when there is no code on the line
							padding = 0;
						}
						g.DrawString(commentString.PadLeft(padding+commentString.Length), this.Font, commentBrush, marginLeft, positionY, StringFormat.GenericTypographic);
					}
				}

				if(this.ShowByteCode && !this.ShowSingleContentLineNotes) {
					g.DrawString(lineData.ByteCode, _noteFont, Brushes.Gray, marginLeft, positionY + this.Font.Size+3, StringFormat.GenericTypographic);
				}
				this.DrawHighlightedSearchString(g, codeString, marginLeft, positionY);
			}
		}
		string _lastSymbolComment = null;

		private void DrawLineSymbols(Graphics g, int positionY, LineProperties lineProperties, int lineHeight)
		{
			int circleSize = lineHeight - 3;
			if((circleSize % 2) == 1) {
				circleSize++;
			}
			int circleOffsetY = positionY + 2;
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
				using(Brush b = new SolidBrush(ConfigManager.Config.Debug.EventViewer.BreakpointColor)) {
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
			CodeLineData lineData = _dataProvider.GetCodeLineData(currentLine);
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
				this.DrawLineNumber(g, lineData, regularMargin, positionY, lineNumberColor);
			}
			if(this.ShowByteCode && this.ShowSingleContentLineNotes) {
				g.DrawString(lineData.ByteCode, this.Font, Brushes.Gray, regularMargin + 6, positionY, StringFormat.GenericTypographic);
			}

			//Adjust background color highlights based on number of spaces in front of content
			marginLeft += lineData.Indentation;

			if(lineProperties != null) {
				this.DrawLineSymbols(g, positionY, lineProperties, lineHeight);
			}
		}

		private void DrawSelectionLength(Graphics g, int currentLine, int positionY, int lineHeight)
		{
			if(ConfigManager.Config.Debug.Debugger.ShowSelectionLength && currentLine == this.SelectedLine && this.SelectionLength > 0) {
				int startAddress = -1;
				int endAddress = -1;

				int end = this.SelectionStart + this.SelectionLength + 1;
				while(endAddress < 0 && end < this.LineCount) {
					endAddress = _dataProvider.GetLineAddress(end);
					end++;
				}

				int start = this.SelectionStart;
				while(startAddress < 0 && start < this.LineCount && start < end) {
					startAddress = _dataProvider.GetLineAddress(start);
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
			if(_dataProvider == null) {
				return;
			}

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

			while(positionY < rect.Bottom && currentLine < this.LineCount) {
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
			positionY = 0;
			while(positionY < rect.Bottom && currentLine < this.LineCount) {
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

	public interface ICodeDataProvider
	{
		CodeLineData GetCodeLineData(int lineIndex);
		int GetLineCount();
		int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards);
		bool UseOptimizedSearch { get; }

		int GetLineAddress(int lineIndex);
		int GetLineIndex(UInt32 address);
	}
}
