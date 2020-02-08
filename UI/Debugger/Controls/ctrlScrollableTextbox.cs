using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Config;
using System.Globalization;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlScrollableTextbox : BaseControl
	{
		public event EventHandler ScrollPositionChanged;

		private string _noMatchString = null;
		private bool _showScrollbars = true;

		public new event MouseEventHandler MouseUp
		{
			add { this.ctrlTextbox.MouseUp += value; }
			remove { this.ctrlTextbox.MouseUp -= value; }
		}

		public new event MouseEventHandler MouseMove
		{
			add { this.ctrlTextbox.MouseMove += value; }
			remove { this.ctrlTextbox.MouseMove -= value; }
		}

		public new event MouseEventHandler MouseClick
		{
			add { this.ctrlTextbox.MouseClick += value; }
			remove { this.ctrlTextbox.MouseClick -= value; }
		}

		public new event MouseEventHandler MouseDown
		{
			add { this.ctrlTextbox.MouseDown += value; }
			remove { this.ctrlTextbox.MouseDown -= value; }
		}

		public new event MouseEventHandler MouseDoubleClick
		{
			add { this.ctrlTextbox.MouseDoubleClick += value; }
			remove { this.ctrlTextbox.MouseDoubleClick -= value; }
		}

		public new event EventHandler MouseLeave
		{
			add { this.ctrlTextbox.MouseLeave += value; }
			remove { this.ctrlTextbox.MouseLeave -= value; }
		}
		
		public event EventHandler TextZoomChanged;

		public ctrlScrollableTextbox()
		{
			InitializeComponent();

			bool designMode = (LicenseManager.UsageMode == LicenseUsageMode.Designtime);
			if(!designMode) {
				this.panelSearch.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)));
				this.panelSearch.Location = new System.Drawing.Point(this.Width - this.panelSearch.Width - 20, -1);

				this.ctrlTextbox.ShowLineNumbers = true;

				this.hScrollBar.ValueChanged += hScrollBar_ValueChanged;
				this.vScrollBar.ValueChanged += vScrollBar_ValueChanged;
				this.ctrlTextbox.ScrollPositionChanged += ctrlTextbox_ScrollPositionChanged;
				this.ctrlTextbox.SelectedLineChanged += ctrlTextbox_SelectedLineChanged;

				new ToolTip().SetToolTip(picCloseSearch, "Close");
				new ToolTip().SetToolTip(picSearchNext, "Find Next (F3)");
				new ToolTip().SetToolTip(picSearchPrevious, "Find Previous (Shift-F3)");
			}
		}

		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);
			this.panelSearch.Location = new System.Drawing.Point(this.Width - this.panelSearch.Width - 20, -1);
		}

		protected override void OnInvalidated(InvalidateEventArgs e)
		{
			base.OnInvalidated(e);
			ctrlTextbox.Invalidate();
		}

		public bool ShowScrollbars
		{
			get
			{
				return this._showScrollbars;
			}
			set
			{
				this._showScrollbars = value;
				this.hScrollBar.Visible = value;
				this.vScrollBar.Visible = value;
			}
		}

		public IScrollbarColorProvider ScrollbarColorProvider
		{
			set { this.vScrollBar.ColorProvider = value; }
		}

		private void ctrlTextbox_SelectedLineChanged(object sender, EventArgs e)
		{
			this.vScrollBar.Invalidate();
		}

		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public Font BaseFont
		{
			get { return this.ctrlTextbox.BaseFont; }
			set
			{
				this.ctrlTextbox.BaseFont = value;
				UpdateHorizontalScrollbar();
				this.ctrlTextbox.Invalidate();
			}
		}

		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public int TextZoom
		{
			get { return this.ctrlTextbox.TextZoom; }
			set {
				if(this.ctrlTextbox.TextZoom != value) {
					this.ctrlTextbox.TextZoom = value;
					UpdateHorizontalScrollbar();
					if(this.TextZoomChanged != null) {
						this.TextZoomChanged(this, null);
					}
				}
			}
		}

		public string GetWordUnderLocation(Point position)
		{
			return this.ctrlTextbox.GetWordUnderLocation(position);
		}

		public Point GetWordEndPosition(Point position)
		{
			return this.ctrlTextbox.GetWordEndPosition(position);
		}

		private void ctrlTextbox_ScrollPositionChanged(object sender, EventArgs e)
		{
			this.vScrollBar.Value = this.ctrlTextbox.ScrollPosition;
			this.hScrollBar.Value = this.ctrlTextbox.HorizontalScrollPosition;
			UpdateHorizontalScrollbar();
			UpdateVerticalScrollbar();

			ScrollPositionChanged?.Invoke(null, null);
		}

		private void UpdateVerticalScrollbar()
		{
			this.vScrollBar.Maximum = this.ctrlTextbox.LineCount;
			this.vScrollBar.VisibleLineCount = this.ctrlTextbox.GetNumberVisibleLines();
		}

		private void UpdateHorizontalScrollbar()
		{
			this.hScrollBar.Visible = this.ctrlTextbox.HorizontalScrollWidth > 0 && _showScrollbars;
			int newMax = this.ctrlTextbox.HorizontalScrollWidth + this.hScrollBar.LargeChange - 1;
			if(this.hScrollBar.Value > this.ctrlTextbox.HorizontalScrollWidth) {
				this.hScrollBar.Value = this.ctrlTextbox.HorizontalScrollWidth;
			}
			this.hScrollBar.Maximum = newMax;
		}

		public ILineStyleProvider StyleProvider { set { this.ctrlTextbox.StyleProvider = value; } }

		public ICodeDataProvider DataProvider
		{
			set
			{
				this.ctrlTextbox.DataProvider = value;
				UpdateHorizontalScrollbar();
				UpdateVerticalScrollbar();
			}
		}

		public int GetLineIndexAtPosition(int yPos)
		{
			return this.ctrlTextbox.GetLineIndexAtPosition(yPos);
		}
		
		public int GetNumberVisibleLines()
		{
			return this.ctrlTextbox.GetNumberVisibleLines();
		}

		public void ScrollToLineIndex(int lineIndex, eHistoryType historyType = eHistoryType.Always, bool scrollToTop = false, bool forceScroll = false)
		{
			this.ctrlTextbox.ScrollToLineIndex(lineIndex, historyType, scrollToTop, forceScroll);
		}

		public void ScrollToAddress(int lineNumber, eHistoryType historyType = eHistoryType.Always, bool scrollToTop = false, bool forceScroll = false)
		{
			this.ctrlTextbox.ScrollToAddress(lineNumber, historyType, scrollToTop, forceScroll);
		}

		public void CopySelection(bool copyLineNumbers, bool copyContentNotes, bool copyComments)
		{
			this.ctrlTextbox.CopySelection(copyLineNumbers, copyContentNotes, copyComments);
		}

		public void SetMessage(TextboxMessageInfo message)
		{
			this.ctrlTextbox.SetMessage(message);
		}
		
		public int CurrentLine
		{
			get { return this.ctrlTextbox.CurrentLine; }
		}
		
		public int SelectedLine
		{
			get { return this.ctrlTextbox.SelectedLine; }
		}

		public int LastSelectedLine
		{
			get { return this.ctrlTextbox.LastSelectedLine; }
		}

		public int CodeMargin
		{
			get { return this.ctrlTextbox.CodeMargin; }
		}

		protected override void OnMouseWheel(MouseEventArgs e)
		{
			base.OnMouseWheel(e);
			this.vScrollBar.Value = Math.Min(this.vScrollBar.Maximum, Math.Max(0, this.vScrollBar.Value - e.Delta / 40));
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(!this.cboSearch.Focused) {
				if(keyData == ConfigManager.Config.Debug.Shortcuts.SelectAll) {
					this.SelectAll();
					return true;
				}

				switch(keyData) {
					case Keys.Right | Keys.Shift:
					case Keys.Down | Keys.Shift:
						this.ctrlTextbox.MoveSelectionDown();
						return true;

					case Keys.Down:
					case Keys.Right:
						this.ctrlTextbox.SelectionStart = this.ctrlTextbox.SelectedLine + 1;
						this.ctrlTextbox.SelectionLength = 0;
						return true;

					case Keys.Up | Keys.Shift:
					case Keys.Left | Keys.Shift:
						this.ctrlTextbox.MoveSelectionUp();
						return true;

					case Keys.Up:
					case Keys.Left:
						this.ctrlTextbox.SelectionStart = this.ctrlTextbox.SelectedLine - 1;
						this.ctrlTextbox.SelectionLength = 0;
						return true;

					case Keys.Home | Keys.Shift:
						this.ctrlTextbox.MoveSelectionUp(this.ctrlTextbox.LineCount);
						break;

					case Keys.End | Keys.Shift:
						this.ctrlTextbox.MoveSelectionDown(this.ctrlTextbox.LineCount);
						break;

					case Keys.Home:
						this.ctrlTextbox.SelectionStart = 0;
						this.ctrlTextbox.SelectionLength = 0;
						return true;

					case Keys.End:
						this.ctrlTextbox.SelectionStart = this.ctrlTextbox.LineCount - 1;
						this.ctrlTextbox.SelectionLength = 0;
						return true;
				}
			}

			if(keyData == ConfigManager.Config.Debug.Shortcuts.IncreaseFontSize) {
				this.TextZoom += 10;
				return true;
			} else if(keyData == ConfigManager.Config.Debug.Shortcuts.DecreaseFontSize) {
				this.TextZoom -= 10;
				return true;
			} else if(keyData == ConfigManager.Config.Debug.Shortcuts.ResetFontSize) {
				this.TextZoom = 100;
				return true;
			} else if(keyData == ConfigManager.Config.Debug.Shortcuts.Find) {
				this.OpenSearchBox(true);
				return true;
			} else if(keyData == ConfigManager.Config.Debug.Shortcuts.FindNext) {
				this.FindNext();
				return true;
			} else if(keyData == ConfigManager.Config.Debug.Shortcuts.FindPrev) {
				this.FindPrevious();
				return true;
			}

			if(keyData == (Keys.Control | Keys.F)) {
				this.OpenSearchBox(true);
				return true;
			}

			switch(keyData) {
				case Keys.PageUp | Keys.Shift:
					this.ctrlTextbox.MoveSelectionUp(20);
					return true;

				case Keys.PageUp:
					this.ctrlTextbox.SelectionStart-=20;
					this.ctrlTextbox.SelectionLength = 0;
					return true;

				case Keys.PageDown | Keys.Shift:
					this.ctrlTextbox.MoveSelectionDown(20);
					return true;

				case Keys.PageDown:
					this.ctrlTextbox.SelectionStart+=20;
					this.ctrlTextbox.SelectionLength = 0;
					return true;

				case Keys.Escape:
					if(this.cboSearch.Focused) {
						this.CloseSearchBox();
						return true;
					}
					break;
			}

			return base.ProcessCmdKey(ref msg, keyData);
		}

		public void SelectAll()
		{
			this.ctrlTextbox.SelectionStart = 0;
			this.ctrlTextbox.SelectionLength = this.ctrlTextbox.LineCount;
		}

		private void vScrollBar_ValueChanged(object sender, EventArgs e)
		{
			this.ctrlTextbox.ScrollPosition = this.vScrollBar.Value;
		}
		
		private void hScrollBar_ValueChanged(object sender, EventArgs e)
		{
			this.ctrlTextbox.HorizontalScrollPosition = this.hScrollBar.Value;
		}

		public bool ShowSingleContentLineNotes
		{
			get { return this.ctrlTextbox.ShowSingleContentLineNotes; }
			set { this.ctrlTextbox.ShowSingleContentLineNotes = value; }
		}

		public bool ShowContentNotes
		{
			get { return this.ctrlTextbox.ShowByteCode; }
			set { this.ctrlTextbox.ShowByteCode = value; }
		}

		public bool ShowCompactPrgAddresses { get { return this.ctrlTextbox.ShowCompactPrgAddresses; } set { this.ctrlTextbox.ShowCompactPrgAddresses = value; } }

		public bool ShowLineNumberNotes
		{
			get { return this.ctrlTextbox.ShowAbsoluteAddreses; }
			set { this.ctrlTextbox.ShowAbsoluteAddreses = value; }
		}

		public bool ShowSingleLineLineNumberNotes
		{
			get { return this.ctrlTextbox.ShowSingleLineLineNumberNotes; }
			set { this.ctrlTextbox.ShowSingleLineLineNumberNotes = value; }
		}
		
		public bool ShowMemoryValues
		{
			get { return this.ctrlTextbox.ShowMemoryValues; }
			set { this.ctrlTextbox.ShowMemoryValues = value; }
		}

		public bool HideSelection
		{
			get { return this.ctrlTextbox.HideSelection; }
			set { this.ctrlTextbox.HideSelection = value; }
		}

		public bool CodeHighlightingEnabled
		{
			get { return this.ctrlTextbox.CodeHighlightingEnabled; }
			set { this.ctrlTextbox.CodeHighlightingEnabled = value; }
		}

		public int LineCount { get { return this.ctrlTextbox.LineCount; } }
		public int SelectionStart { get { return this.ctrlTextbox.SelectionStart; } }
		public int SelectionLength { get { return this.ctrlTextbox.SelectionLength; } }

		public int MarginWidth { set { this.ctrlTextbox.MarginWidth = value; } }
		public int ExtendedMarginWidth { set { this.ctrlTextbox.ExtendedMarginWidth = value; } }
		public int AddressSize { set { this.ctrlTextbox.AddressSize = value; } }

		public void OpenSearchBox(bool forceFocus = false)
		{
			bool focus = !this.panelSearch.Visible;
			this.panelSearch.Visible = true;
			if(focus || forceFocus) {
				this.cboSearch.Focus();
				this.cboSearch.SelectAll();
			}
		}

		private void CloseSearchBox()
		{
			this.ctrlTextbox.Search(null, false, false);
			this.panelSearch.Visible = false;
			this.Focus();
		}

		public void FindNext()
		{
			this.OpenSearchBox();
			this.ctrlTextbox.Search(this.cboSearch.Text, false, false);
		}

		public void FindPrevious()
		{
			this.OpenSearchBox();
			this.ctrlTextbox.Search(this.cboSearch.Text, true, false);
		}

		private void picCloseSearch_Click(object sender, EventArgs e)
		{
			this.CloseSearchBox();
		}

		private void picSearchPrevious_MouseUp(object sender, MouseEventArgs e)
		{
			this.FindPrevious();
		}

		private void picSearchNext_MouseUp(object sender, MouseEventArgs e)
		{
			this.FindNext();
		}

		private void cboSearch_TextUpdate(object sender, EventArgs e)
		{
			if(_noMatchString != null && this.cboSearch.Text.StartsWith(_noMatchString)) {
				return;
			}

			if(!this.ctrlTextbox.Search(this.cboSearch.Text, false, true)) {
				_noMatchString = this.cboSearch.Text;
				this.cboSearch.BackColor = Color.Coral;
			} else {
				_noMatchString = null;
				this.cboSearch.BackColor = Color.Empty;
			}
		}

		private void cboSearch_Leave(object sender, EventArgs e)
		{
			_noMatchString = null;
		}

		private void cboSearch_KeyDown(object sender, KeyEventArgs e)
		{
			if(e.KeyCode == Keys.Enter) {
				this.FindNext();
				if(this.cboSearch.Items.Contains(this.cboSearch.Text)) {
					this.cboSearch.Items.Remove(this.cboSearch.Text);
				}
				this.cboSearch.Items.Insert(0, this.cboSearch.Text);

				e.Handled = true;
				e.SuppressKeyPress = true;
			}
		}
		
		public void NavigateForward()
		{
			this.ctrlTextbox.NavigateForward();
		}

		public void NavigateBackward()
		{
			this.ctrlTextbox.NavigateBackward();
		}
	}
}
