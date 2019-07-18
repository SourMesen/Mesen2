using Mesen.GUI.Config;
using Mesen.GUI.Debugger;
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
		private ctrlTooltip ctrlTooltip;

		public BaseForm()
		{
			InitializeComponent();
		}

		public virtual bool IsConfigForm { get { return false; } }
		public bool InMenu { get { return _inMenu > 0; } }

		public static ctrlTooltip GetPopupTooltip(Form form)
		{
			return (form as BaseForm).ctrlTooltip;
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
			this.ctrlTooltip = new Mesen.GUI.Debugger.ctrlTooltip();
			this.SuspendLayout();
			// 
			// toolTip
			// 
			this.toolTip.AutomaticDelay = 0;
			this.toolTip.AutoPopDelay = 32700;
			this.toolTip.InitialDelay = 10;
			this.toolTip.ReshowDelay = 10;
			// 
			// ctrlTooltip
			// 
			this.ctrlTooltip.AutoSize = true;
			this.ctrlTooltip.Location = new System.Drawing.Point(0, 0);
			this.ctrlTooltip.Name = "ctrlTooltip";
			this.ctrlTooltip.Size = new System.Drawing.Size(1, 1);
			this.ctrlTooltip.TabIndex = 0;
			this.ctrlTooltip.Visible = false;
			// 
			// BaseForm
			// 
			this.ClientSize = new System.Drawing.Size(284, 261);
			this.Controls.Add(this.ctrlTooltip);
			this.Name = "BaseForm";
			this.ResumeLayout(false);
			this.PerformLayout();

		}
	}
}
