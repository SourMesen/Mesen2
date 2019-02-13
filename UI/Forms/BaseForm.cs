using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	public class BaseForm : Form
	{
		public delegate void ProcessCmdKeyHandler(Keys keyData, ref bool processed);
		public event ProcessCmdKeyHandler OnProcessCmdKey;

		protected ToolTip toolTip;
		private System.ComponentModel.IContainer components;
		private bool _iconSet = false;
		protected int _inMenu = 0;
		private static Timer _tmrUpdateBackground;
		//private static bool _needResume = false;

		public BaseForm()
		{
			InitializeComponent();
		}

		protected virtual bool IsConfigForm { get { return false; } }

		public static void StartBackgroundTimer()
		{
			_tmrUpdateBackground = new Timer();
			_tmrUpdateBackground.Start();
			_tmrUpdateBackground.Tick += tmrUpdateBackground_Tick;
		}

		public static void StopBackgroundTimer()
		{
			_tmrUpdateBackground?.Stop();
		}

		private static void tmrUpdateBackground_Tick(object sender, EventArgs e)
		{
			/*Form focusedForm = null;
			foreach(Form form in Application.OpenForms) {
				if(form.ContainsFocus) {
					focusedForm = form;
					break;
				}
			}

			bool needPause = focusedForm == null && ConfigManager.Config.PreferenceInfo.PauseWhenInBackground;
			if(focusedForm != null) {
				needPause |= ConfigManager.Config.PreferenceInfo.PauseWhenInMenusAndConfig && focusedForm is BaseForm && (((BaseForm)focusedForm)._inMenu > 0 || ((BaseForm)focusedForm).IsConfigForm);
				needPause |= ConfigManager.Config.PreferenceInfo.PauseWhenInMenusAndConfig && !(focusedForm is BaseInputForm) && !focusedForm.GetType().FullName.Contains("Debugger");
				needPause |= ConfigManager.Config.PreferenceInfo.PauseWhenInDebuggingTools && focusedForm.GetType().FullName.Contains("Debugger");
			}

			if(needPause) {
				if(!EmuApi.IsPaused(EmuApi.ConsoleId.Master)) {
					_needResume = true;
					EmuApi.Pause(EmuApi.ConsoleId.Master);
				}
			} else if(_needResume) {
				EmuApi.Resume(EmuApi.ConsoleId.Master);
				_needResume = false;
			}

			EmuApi.SetFlag(EmulationFlags.InBackground, focusedForm == null);*/
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			bool processed = false;
			OnProcessCmdKey?.Invoke(keyData, ref processed);
			return processed || base.ProcessCmdKey(ref msg, keyData);
		}

		public void Show(object sender, IWin32Window owner = null)
		{
			if(sender is ToolStripMenuItem) {
				ToolStripItem menuItem = (ToolStripMenuItem)sender;
				if(menuItem.Image == null) {
					menuItem = menuItem.OwnerItem;
				}
				this.Icon = menuItem.Image;
			}

			CenterOnParent(owner);
			base.Show();
		}

		private void CenterOnParent(IWin32Window owner)
		{
			Form parent = (Form)owner;
			Point point = parent.PointToScreen(new Point(parent.Width / 2, parent.Height / 2));

			this.StartPosition = FormStartPosition.Manual;
			this.Top = point.Y - this.Height / 2;
			this.Left = point.X - this.Width / 2;
		}

		public DialogResult ShowDialog(object sender, IWin32Window owner = null)
		{
			if(sender is ToolStripMenuItem) {
				ToolStripItem menuItem = (ToolStripMenuItem)sender;
				if(menuItem.Image == null) {
					menuItem = menuItem.OwnerItem;
				}
				this.Icon = menuItem.Image;
			}
			return base.ShowDialog(owner);
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if(!DesignMode) {
				if(!_iconSet) {
					base.Icon = Properties.Resources.MesenIcon;
				}
			}

			int tabIndex = 0;
			InitializeTabIndexes(this, ref tabIndex);
			ResourceHelper.ApplyResources(this);
		}

		private void InitializeTabIndexes(TableLayoutPanel tlp, ref int tabIndex)
		{
			tlp.TabIndex = tabIndex;
			tabIndex++;

			for(int i = 0; i < tlp.RowCount; i++) {
				for(int j = 0; j < tlp.ColumnCount; j++) {
					Control ctrl = tlp.GetControlFromPosition(j, i);
					if(ctrl != null) {
						if(ctrl is TableLayoutPanel) {
							InitializeTabIndexes(((TableLayoutPanel)ctrl), ref tabIndex);
						} else {
							InitializeTabIndexes(ctrl, ref tabIndex);
						}
					}
				}
			}
		}

		private void InitializeTabIndexes(Control container, ref int tabIndex)
		{
			container.TabIndex = tabIndex;
			tabIndex++;

			foreach(Control ctrl in container.Controls) {
				if(ctrl is TableLayoutPanel) {
					InitializeTabIndexes(((TableLayoutPanel)ctrl), ref tabIndex);
				} else {
					InitializeTabIndexes(ctrl, ref tabIndex);
				}
			}
		}

		public new Image Icon
		{
			set
			{
				if(value != null) {
					Bitmap b = new Bitmap(value);
					Icon i = System.Drawing.Icon.FromHandle(b.GetHicon());
					base.Icon = i;
					i.Dispose();

					_iconSet = true;
				}
			}				
		}
		
		public new SizeF AutoScaleDimensions
		{
			set 
			{ 
				if(!Program.IsMono) { 
					base.AutoScaleDimensions = value; 
				}
			}
		} 
				
		public new AutoScaleMode AutoScaleMode
		{
			set {
				if(Program.IsMono) { 
					base.AutoScaleMode = AutoScaleMode.None;
				} else {
					base.AutoScaleMode = value;
				}
			}
		}		

		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.toolTip = new System.Windows.Forms.ToolTip(this.components);
			this.SuspendLayout();
			// 
			// toolTip
			// 
			this.toolTip.AutomaticDelay = 0;
			this.toolTip.AutoPopDelay = 32700;
			this.toolTip.InitialDelay = 10;
			this.toolTip.ReshowDelay = 10;
			// 
			// BaseForm
			// 
			this.Name = "BaseForm";
			this.ResumeLayout(false);

		}
	}
}
