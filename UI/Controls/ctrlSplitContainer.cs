using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	class ctrlSplitContainer : SplitContainer
	{
		public event EventHandler PanelCollapsed;
		public event EventHandler PanelExpanded;
		private int _originalDistance = 0;
		private int _originalMinSize = 0;
		private bool _collapsed = false;
		private bool _preventExpand = false;
		private bool _hidePanel2 = false;

		public ctrlSplitContainer()
		{
			this.DoubleBuffered = true;
			this.SplitterMoving += ctrlSplitContainer_SplitterMoving;
		}

		public bool HidePanel2
		{
			get { return _hidePanel2; }
			set { _hidePanel2 = value; this.Invalidate(); }
		}

		private void ctrlSplitContainer_SplitterMoving(object sender, SplitterCancelEventArgs e)
		{
			if(_originalMinSize > 0) {
				e.Cancel = true;
				this.BeginInvoke((MethodInvoker)(() => {
					this.ExpandPanel();
				}));
			}
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			if(_hidePanel2) {
				return;
			}

			if(this.Orientation == Orientation.Horizontal) {
				int top = this.SplitterDistance + 1;
				int center = this.Width / 2;
				e.Graphics.DrawLine(Pens.DarkGray, center - 100, top, center + 100, top);
				top+=2;
				e.Graphics.DrawLine(Pens.DarkGray, center - 100, top, center + 100, top);
			} else {
				int center = this.Height / 2;
				int left = this.SplitterDistance + 1;
				e.Graphics.DrawLine(Pens.DarkGray, left, center - 100, left, center + 100);
				left+=2;
				e.Graphics.DrawLine(Pens.DarkGray, left, center - 100, left, center + 100);
			}
		}

		protected override void OnDoubleClick(EventArgs e)
		{
			base.OnDoubleClick(e);

			if(_hidePanel2) {
				return;
			}

			if(_originalMinSize == 0) {
				this.CollapsePanel();
				_preventExpand = true;
			} else {
				this.ExpandPanel();
			}
		}

		public void ExpandPanel()
		{
			if(_hidePanel2 || !_collapsed) {
				return;
			}

			if(this.FixedPanel == FixedPanel.Panel1) {
				throw new Exception("Not implemented");
			} else if(this.FixedPanel == FixedPanel.Panel2) {
				if(_originalMinSize > 0) {
					this.Panel2MinSize = _originalMinSize;
					this.SplitterDistance = _originalDistance;
					_originalMinSize = 0;
				}
				this.PanelExpanded?.Invoke(this, EventArgs.Empty);
				_collapsed = false;
			}
		}

		public void CollapsePanel()
		{
			if(_collapsed) {
				return;
			}

			if(this.FixedPanel == FixedPanel.Panel1) {
				throw new Exception("Not implemented");
			} else if(this.FixedPanel == FixedPanel.Panel2) {
				_collapsed = true;
				_originalDistance = this.SplitterDistance;
				_originalMinSize = this.Panel2MinSize;
				this.Panel2MinSize = 2;
				this.SplitterDistance = this.Orientation == Orientation.Horizontal ? this.Height : this.Width;

				this.PanelCollapsed?.Invoke(this, EventArgs.Empty);
			}
		}

		public int GetSplitterDistance()
		{
			return _originalMinSize > 0 ? _originalDistance : this.SplitterDistance;
		}

		DateTime _lastResize = DateTime.MinValue;
		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);

			if(_hidePanel2) {
				return;
			}

			//Horrible patch to fix what looks like a resize bug in SplitContainers
			//Resizing the window sometimes doesn't resize the inner panels properly
			//causing their content to go partially offscreen.
			//This code alters SplitterDistance approx 100ms after the last resize event
			//to fix the display bug
			if(this.IsHandleCreated) {
				bool firstResize;
				lock(this) {
					 firstResize = _lastResize == DateTime.MinValue;
					_lastResize = DateTime.Now;
				}
				if(firstResize) {
					Task.Run(() => {
						while((DateTime.Now - _lastResize).Milliseconds < 100) {
							System.Threading.Thread.Sleep(100);
						}

						this.BeginInvoke((MethodInvoker)(() => {
							if(_originalMinSize == 0) {
								this.SuspendLayout();
								this.Panel1.SuspendLayout();
								this.Panel2.SuspendLayout();
								try {
									if(this.Width > 0 && this.Height > 0) {
										this.SplitterDistance++;
										this.SplitterDistance--;
									}
								} catch {
								} finally {
									this.ResumeLayout();
									this.Panel1.ResumeLayout();
									this.Panel2.ResumeLayout();
								}
							}
							this.Invalidate();
							lock(this) {
								_lastResize = DateTime.MinValue;
							}
						}));
					});
				}
			}
		}

		protected override void OnMouseUp(MouseEventArgs e)
		{
			base.OnMouseUp(e);
			this.Panel1.Focus();

			if(_hidePanel2) {
				return;
			}

			if(!_preventExpand && _originalMinSize > 0) {
				this.ExpandPanel();
			}

			_preventExpand = false;
		}
	}
}
