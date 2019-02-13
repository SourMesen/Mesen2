using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public class ctrlCodeScrollbar : Control
	{
		public event EventHandler ValueChanged;

		private Timer _tmrScroll;
		private bool _scrollUp = false;

		private const int _buttonSize = 15;

		private IScrollbarColorProvider _colorProvider = null;
		public IScrollbarColorProvider ColorProvider
		{
			get { return _colorProvider; }
			set { _colorProvider = value; this.Invalidate(); }
		}

		public ctrlCodeScrollbar()
		{
			this.DoubleBuffered = true;
			this.ResizeRedraw = true;
			this._tmrScroll = new Timer();
			this._tmrScroll.Tick += tmrScroll_Tick;			
		}

		private void tmrScroll_Tick(object sender, EventArgs e)
		{
			_tmrScroll.Interval = 50;
			if(this._scrollUp) {
				if(this.Value > 0) {
					this.Value--;
				}
			} else {
				if(this.Value < this.Maximum) {
					this.Value++;
				}
			}
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			Rectangle rect = this.ClientRectangle;

			int left = rect.Left;
			int width = rect.Width;
			e.Graphics.FillRectangle(Brushes.DimGray, rect);

			int barTop = rect.Top + _buttonSize;
			int barHeight = this.Height - _buttonSize * 2;

			float startPos = (float)this.Value / this.Maximum;
			if(this.ColorProvider != null) {
				Color prevBgColor = Color.White;
				int drawHeight = 0;
				for(int i = 0; i < barHeight; i++) {
					float top = barTop + i;
					float position = (float)i / barHeight;
					Color bgColor = this.ColorProvider.GetBackgroundColor(position);
					if(bgColor != prevBgColor && i > 0) {
						using(Brush brush = new SolidBrush(prevBgColor)) {
							e.Graphics.FillRectangle(brush, left + 1, top - drawHeight, width - 1, drawHeight);
							drawHeight = 0;
						}
					}
					drawHeight++;

					if(bgColor != Color.Transparent) {
						prevBgColor = bgColor;
					}
				}
				using(Brush brush = new SolidBrush(prevBgColor)) {
					e.Graphics.FillRectangle(brush, left + 1, barTop + barHeight- drawHeight, width - 1, drawHeight);
				}
			} else {
				e.Graphics.FillRectangle(Brushes.White, left + 1, barTop, width - 1, barHeight);
			}

			float highlightTop = barTop + barHeight * startPos - HighlightOffset;
			using(SolidBrush brush = new SolidBrush(Color.FromArgb(120, 220, 220, 255))) {
				e.Graphics.FillRectangle(brush, left + 1, highlightTop, width, HighlightHeight - 2);
				e.Graphics.DrawRectangle(Pens.DarkSlateGray, left + 1, highlightTop, width - 2, HighlightHeight - 2);
				e.Graphics.DrawRectangle(Pens.Gray, left + 2, highlightTop + 1, width - 4, HighlightHeight - 4);
			}

			if(this.ColorProvider != null) {
				int selectedIndex = this.ColorProvider.GetSelectedLine();
				if(selectedIndex >= 0) {
					int selectedTop = selectedIndex * barHeight / this.Maximum + barTop;
					e.Graphics.FillRectangle(Brushes.LightBlue, left + 1, selectedTop - 1, width - 2, 3);
					e.Graphics.DrawRectangle(Pens.DimGray, left + 1, selectedTop - 1, width - 2, 3);
				}

				int activeIndex = this.ColorProvider.GetActiveLine();
				if(activeIndex >= 0) {
					int activeTop = activeIndex * barHeight / this.Maximum + barTop;
					e.Graphics.FillRectangle(Brushes.Yellow, left + 1, activeTop - 1, width - 2, 3);
					e.Graphics.DrawRectangle(Pens.DimGray, left + 1, activeTop - 1, width - 2, 3);
				}

				int linesPerPixel = (int)Math.Ceiling((float)this.Maximum / barHeight);
				for(int i = 0; i < barHeight; i++) {
					float top = barTop + i;
					float position = (float)i / barHeight;
					Color markerColor = this.ColorProvider.GetMarkerColor(position, linesPerPixel);
					if(markerColor != Color.Transparent) {
						using(Brush brush = new SolidBrush(markerColor)) {
							e.Graphics.FillRectangle(brush, left + 3, top - 2, 5, 5);
						}
					}
				}
			}

			int arrowWidth = 10;
			int arrowHeight = 5;
			int bottom = barTop + barHeight + _buttonSize;
			e.Graphics.PixelOffsetMode = PixelOffsetMode.HighQuality;
			e.Graphics.FillRectangle(Brushes.Gainsboro, rect.Left + 1, rect.Top, this.Width - 1, _buttonSize);
			e.Graphics.FillRectangle(Brushes.Gainsboro, rect.Left + 1, bottom - _buttonSize, this.Width - 1, _buttonSize);
			e.Graphics.DrawLine(Pens.DimGray, rect.Left + 1, rect.Top + _buttonSize, rect.Left + width, rect.Top + _buttonSize);
			e.Graphics.DrawLine(Pens.DimGray, rect.Left + 1, bottom - _buttonSize, rect.Left + width, bottom - _buttonSize);
			e.Graphics.DrawLine(Pens.DimGray, rect.Left + 1, bottom, rect.Left + width, bottom);
			e.Graphics.TranslateTransform(5, (_buttonSize - arrowHeight) / 2);
			e.Graphics.FillPolygon(Brushes.DimGray, new Point[] { new Point(left, rect.Top + arrowHeight), new Point(left + arrowWidth, rect.Top + arrowHeight), new Point(left + arrowWidth / 2, rect.Top) }, FillMode.Winding);
			e.Graphics.TranslateTransform(0, -(_buttonSize - arrowHeight));
			e.Graphics.FillPolygon(Brushes.DimGray, new Point[] { new Point(left, bottom - arrowHeight), new Point(left + arrowWidth, bottom - arrowHeight), new Point(left + arrowWidth / 2, bottom) }, FillMode.Winding);
		}

		//private bool _mouseDown = false;
		protected override void OnMouseDown(MouseEventArgs e)
		{
			base.OnMouseDown(e);

			if(e.Y > _buttonSize && e.Y < (this.Height - _buttonSize)) {
				this.UpdatePosition(e.Y);
				//this._mouseDown = true;
			} else {
				if(e.Y <= _buttonSize) {
					if(this.Value > 0) {
						this._scrollUp = true;
						this.Value--;
						this._tmrScroll.Interval = 350;
						this._tmrScroll.Start();
					}
				} else {
					if(this.Value < this.Maximum) {
						this._scrollUp = false;
						this.Value++;
						this._tmrScroll.Interval = 350;
						this._tmrScroll.Start();
					}
				}
			}

			//TODO
			/*if(_codeTooltip != null) {
				_codeTooltip.Close();
				_codeTooltip = null;
			}

			_lastPreviewScrollPosition = -1;*/
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			base.OnMouseUp(e);
			//this._mouseDown = false;
			this._tmrScroll.Stop();
		}

		//TODO
		/*
		frmCodePreviewTooltip _codeTooltip = null;
		int _lastPreviewScrollPosition = -1;
		protected override void OnMouseMove(MouseEventArgs e)
		{
			base.OnMouseMove(e);
			if(this._mouseDown) {
				this.UpdatePosition(e.Y);
			} else {
				if(this.ColorProvider != null && e.Y > _buttonSize && e.Y < (this.Height - _buttonSize)) {
					int scrollPosition = Math.Max(0, (e.Y - this.Top - _buttonSize) * this.Maximum / (this.Height - _buttonSize * 2));
					if(_lastPreviewScrollPosition != scrollPosition) {
						Point p = this.PointToScreen(new Point(this.ClientRectangle.Right, e.Y));
						if(_codeTooltip == null) {
							_codeTooltip = this.ColorProvider.GetPreview(scrollPosition);
							if(_codeTooltip != null) {
								_codeTooltip.FormClosed += (s, evt) => { _codeTooltip = null; };
							}
						} else {
							_codeTooltip.ScrollToLineIndex(scrollPosition);
						}
						if(_codeTooltip != null) {
							_codeTooltip.SetFormLocation(new Point(p.X + 5, p.Y), this.Parent);
						}
						_lastPreviewScrollPosition = scrollPosition;
					}
				}
			}
		}
		

		protected override void OnMouseLeave(EventArgs e)
		{
			base.OnMouseLeave(e);
			if(_codeTooltip != null) {
				bool restoreFocus = _codeTooltip.NeedRestoreFocus;
				_codeTooltip.Close();
				if(restoreFocus) {
					this.Parent.Focus();
				}
				_codeTooltip = null;
			}
			_lastPreviewScrollPosition = -1;
			_tmrScroll.Stop();
		}*/

		private int HighlightHeight { get { return Math.Max(8, (int)(((float)this.VisibleLineCount / this.Maximum) * this.Height)); } }
		private int HighlightOffset
		{
			get
			{
				int highlightHeight = (int)(((float)this.VisibleLineCount / this.Maximum) * this.Height);
				if(HighlightHeight - highlightHeight > 0) {
					return (HighlightHeight - highlightHeight) / 2;
				} else {
					return 0;
				}
			}
		}

		private void UpdatePosition(int y)
		{
			this.Value = Math.Max(0 , y - HighlightHeight / 2 + HighlightOffset - this.Top - _buttonSize) * this.Maximum / (this.Height - _buttonSize * 2);
		}

		private int _value = 0;
		public int Value
		{
			get
			{
				return this._value;
			}
			set
			{
				if(this._value != value) {
					this._value = value;
					this.ValueChanged?.Invoke(this, EventArgs.Empty);

					this.Invalidate();
				}
			}
		}

		private int _maximum = 1;
		public int Maximum
		{
			get { return Math.Max(1, this._maximum); }
			set { this._maximum = value; this.Invalidate(); }
		}

		public int VisibleLineCount { get; set; } = 1;
		public int LargeChange { get; set; } = 1;
	}

	public interface IScrollbarColorProvider
	{
		Color GetBackgroundColor(float position);
		Color GetMarkerColor(float position, int linesPerPixel);
		int GetActiveLine();
		int GetSelectedLine();

		//TODO
		//frmCodePreviewTooltip GetPreview(int lineIndex);
	}
}
