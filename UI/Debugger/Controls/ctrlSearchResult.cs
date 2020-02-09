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
using Mesen.GUI.Forms;
using System.IO;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Debugger.Integration;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlSearchResult : BaseControl
	{
		public new event EventHandler Click;
		public new event EventHandler DoubleClick;

		public ctrlSearchResult()
		{
			InitializeComponent();
			this.TabStop = false;
			this.AddClickHandler(this);
		}

		protected override bool ProcessTabKey(bool forward)
		{
			return true;
		}

		private void ChildClickHandler(object sender, EventArgs e)
		{
			Click?.Invoke(this, e);
		}

		private void CtrlDoubleClickHandler(object sender, EventArgs e)
		{
			DoubleClick?.Invoke(this, e);
		}

		private void AddClickHandler(Control parent)
		{
			foreach(Control ctrl in parent.Controls) {
				ctrl.Click += ChildClickHandler;
				ctrl.DoubleClick += CtrlDoubleClickHandler;
				AddClickHandler(ctrl);
			}
		}
		
		public void Initialize(SearchResultInfo info)
		{
			lblLabelName.Text = info.Caption;
			if(info.Length > 1) {
				if(info.Length == 2) {
					lblLabelName.Text += " (word)";
				} else {
					lblLabelName.Text += $" ({info.Length} bytes)";
				}
			}
			if(info.AbsoluteAddress?.Address >= 0) {
				lblAbsoluteAddress.Text = "$" + info.AbsoluteAddress?.Address.ToString("X4") + ":$" + info.Value.ToString("X2");
				if(info.RelativeAddress?.Address >= 0) {
					lblRelativeAddress.ForeColor = SystemColors.ControlText;
					lblRelativeAddress.Text = "$" + info.RelativeAddress?.Address.ToString("X4") + ":$" + info.Value.ToString("X2");
				} else {
					lblRelativeAddress.ForeColor = SystemColors.GrayText;
					lblRelativeAddress.Text = "<out of scope>";
				}
				SnesMemoryType memType = info.AbsoluteAddress?.Type ?? SnesMemoryType.Register;
				lblMemoryType.Text = ResourceHelper.GetEnumText(memType);
				lblCpu.Text = "CPU";
				if(info.SearchResultType == SearchResultType.Function) {
					picType.Image = Properties.Resources.Function;
				} else if(info.SearchResultType == SearchResultType.JumpTarget) {
					picType.Image = Properties.Resources.JumpTarget;
				} else if(memType == SnesMemoryType.Register) {
					picType.Image = Properties.Resources.RegisterIcon;
				} else {
					picType.Image = Properties.Resources.CheatCode;
				}
			} else if(info.SearchResultType == SearchResultType.File) {
				lblMemoryType.Text = "File";
				lblCpu.Text = "";
				lblAbsoluteAddress.Text = "";
				lblRelativeAddress.Text = "";
				picType.Image = Properties.Resources.LogWindow;
			} else if(info.SearchResultType == SearchResultType.Constant) {
				lblMemoryType.Text = "Constant";
				lblCpu.Text = "";
				lblAbsoluteAddress.Text = "";
				lblRelativeAddress.Text = "Value: $" + info.Value.ToString("X2");
				picType.Image = Properties.Resources.Enum;
			} else {
				lblAbsoluteAddress.Text = "";
				lblRelativeAddress.Text = "";
				lblCpu.Text = "";
				lblMemoryType.Text = "";
				picType.Image = null;
			}

			picWarning.Visible = info.Disabled;
			if(info.Disabled) {
				this.Enabled = false;
			} else {
				this.Enabled = true;
			}

			if(!string.IsNullOrWhiteSpace(info.File?.Name)) {
				if(info.SearchResultType == SearchResultType.File) {
					lblLocation.Text = Path.GetFileName(info.File?.Name);
				} else {
					lblLocation.Text = Path.GetFileName(info.File?.Name) + ":" + (info.SourceLocation?.LineNumber + 1).ToString();
				}
			} else {
				lblLocation.Text = "";
			}
		}
	}

	public enum SearchResultType
	{
		Function,
		JumpTarget,
		Constant,
		Data,
		File
	}

	public class SearchResultInfo
	{
		public string Caption;
		public AddressInfo? AbsoluteAddress;
		public AddressInfo? RelativeAddress;
		public int Value;
		public int Length;
		public SourceFileInfo File;
		public SourceCodeLocation SourceLocation;
		public SearchResultType SearchResultType;
		public CodeLabel CodeLabel;
		public bool Disabled;
	}
}
