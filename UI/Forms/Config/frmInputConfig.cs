using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmInputConfig : BaseConfigForm
	{
		public frmInputConfig()
		{
			InitializeComponent();
			if(DesignMode) {
				return;
			}

			InputConfig cfg = ConfigManager.Config.Input.Clone();
			Entity = cfg;

			BaseConfigForm.InitializeComboBox((ComboBox)cboPlayer1, typeof(ControllerType));
			BaseConfigForm.InitializeComboBox((ComboBox)cboPlayer2, typeof(ControllerType));
			BaseConfigForm.InitializeComboBox((ComboBox)cboPlayer3, typeof(ControllerType));
			BaseConfigForm.InitializeComboBox((ComboBox)cboPlayer4, typeof(ControllerType));
			BaseConfigForm.InitializeComboBox((ComboBox)cboPlayer5, typeof(ControllerType));

			//Remove super scope for now
			cboPlayer1.Items.RemoveAt(3);
			cboPlayer2.Items.RemoveAt(3);

			cboPlayer1.SetEnumValue(cfg.Controllers[0].Type);
			cboPlayer2.SetEnumValue(cfg.Controllers[1].Type);
			cboPlayer3.SetEnumValue(cfg.Controllers[2].Type);
			cboPlayer4.SetEnumValue(cfg.Controllers[3].Type);
			cboPlayer5.SetEnumValue(cfg.Controllers[4].Type);
		}

		protected override void OnApply()
		{
			ConfigManager.Config.Input = (InputConfig)this.Entity;

			ConfigManager.Config.Input.Controllers[0].Type = cboPlayer1.GetEnumValue<ControllerType>();
			ConfigManager.Config.Input.Controllers[1].Type = cboPlayer2.GetEnumValue<ControllerType>();
			ConfigManager.Config.Input.Controllers[2].Type = cboPlayer3.GetEnumValue<ControllerType>();
			ConfigManager.Config.Input.Controllers[3].Type = cboPlayer4.GetEnumValue<ControllerType>();
			ConfigManager.Config.Input.Controllers[4].Type = cboPlayer5.GetEnumValue<ControllerType>();

			ConfigManager.ApplyChanges();
		}

		protected override bool ValidateInput()
		{
			btnSetupP1.Enabled = cboPlayer1.GetEnumValue<ControllerType>() == ControllerType.SnesController;
			btnSetupP2.Enabled = cboPlayer2.GetEnumValue<ControllerType>() == ControllerType.SnesController;
			return base.ValidateInput();
		}

		private void btnSetup_Click(object sender, EventArgs e)
		{
			int index = 0;
			ControllerType type = ControllerType.None;
			string selectedText = "";
			if(sender == btnSetupP1) {
				type = cboPlayer1.GetEnumValue<ControllerType>();
				selectedText = cboPlayer1.SelectedItem.ToString();
				index = 0;
			} else if(sender == btnSetupP2) {
				type = cboPlayer2.GetEnumValue<ControllerType>();
				selectedText = cboPlayer2.SelectedItem.ToString();
				index = 1;
			} else if(sender == btnSetupP3) {
				type = cboPlayer3.GetEnumValue<ControllerType>();
				selectedText = cboPlayer3.SelectedItem.ToString();
				index = 2;
			} else if(sender == btnSetupP4) {
				type = cboPlayer4.GetEnumValue<ControllerType>();
				selectedText = cboPlayer4.SelectedItem.ToString();
				index = 3;
			} else if(sender == btnSetupP5) {
				type = cboPlayer4.GetEnumValue<ControllerType>();
				selectedText = cboPlayer5.SelectedItem.ToString();
				index = 4;
			}

			BaseInputConfigForm frm = null;
			InputConfig cfg = (InputConfig)Entity;
			switch(type) {
				case ControllerType.SnesController:
					frm = new frmControllerConfig(cfg.Controllers[index], index);
					break;
						
				case ControllerType.SuperScope:
					//frm = new frmSuperScopeConfig();
					break;

				case ControllerType.SnesMouse:
					//frm = new frmMouseConfig();
					break;
			}

			if(frm != null) {
				OpenSetupWindow(frm, (Button)sender, selectedText, index);
			}
		}

		private void OpenSetupWindow(BaseInputConfigForm frm, Button btn, string title, int port)
		{
			Point point = btn.PointToScreen(new Point(0, btn.Height));
			Rectangle screen = Screen.FromControl(btn).Bounds;

			if(frm.Height + point.Y > screen.Bottom) {
				//Show on top instead
				point.Y -= btn.Height + frm.Height;

				if(point.Y < 0) {
					point.Y = 0;
				}
			}

			if(frm.Width + point.X > screen.Right) {
				//Show on left instead
				point.X -= frm.Width - btn.Width;
				if(point.X < 0) {
					point.X = 0;
				}
			}

			frm.Text = title;
			frm.StartPosition = FormStartPosition.Manual;
			frm.Top = point.Y;
			frm.Left = point.X;

			if(frm.ShowDialog(this) == DialogResult.OK) {
				InputConfig cfg = (InputConfig)Entity;
				cfg.Controllers[port] = frm.Config;
			}
			frm.Dispose();
		}
	}
}
