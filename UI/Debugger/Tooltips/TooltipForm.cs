using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public class TooltipForm : BaseForm
	{
		protected Form _parentForm;
		private Point _requestedLocation;
		private bool _parentContainedFocus = false;

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(!DesignMode) {
				UpdateLocation();
			}
		}

		private void UpdateLocation()
		{
			Point p = _requestedLocation;
			if(p.Y + this.Height > _parentForm.ClientSize.Height) {
				this.Location = new Point(p.X, _parentForm.ClientSize.Height - this.Height);
			} else {
				this.Location = p;
			}
		}

		public bool NeedRestoreFocus
		{
			get { return _parentContainedFocus; }
		}

		public void SetFormLocation(Point screenLocation, Control focusTarget)
		{
			_requestedLocation = _parentForm.PointToClient(screenLocation);

			if(!this.Visible) {
				this._parentContainedFocus = focusTarget.ContainsFocus;
				UpdateLocation();
				this.Show();
			} else {
				UpdateLocation();
			}

			if(Program.IsMono) {
				focusTarget.Focus();
			}
		}
	}
}
