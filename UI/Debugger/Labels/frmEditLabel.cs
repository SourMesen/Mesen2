using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmEditLabel : BaseConfigForm
	{
		private CodeLabel _originalLabel;

		public frmEditLabel(CodeLabel label)
		{
			InitializeComponent();

			_originalLabel = label;
			Entity = label.Clone();

			AddBinding(nameof(CodeLabel.MemoryType), cboType);
			AddBinding(nameof(CodeLabel.Address), txtAddress);
			AddBinding(nameof(CodeLabel.Label), txtLabel);
			AddBinding(nameof(CodeLabel.Comment), txtComment);
			AddBinding(nameof(CodeLabel.Length), nudLength);

			CpuType cpuType = label.MemoryType.ToCpuType();
			cboType.Items.Clear();
			if(cpuType == CpuType.Cpu) {
				if(DebugApi.GetMemorySize(SnesMemoryType.PrgRom) > 0) {
					cboType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.PrgRom));
				}
				if(DebugApi.GetMemorySize(SnesMemoryType.WorkRam) > 0) {
					cboType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.WorkRam));
				}
				if(DebugApi.GetMemorySize(SnesMemoryType.SaveRam) > 0) {
					cboType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SaveRam));
				}
				cboType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Register));
			} else if(cpuType == CpuType.Spc) {
				cboType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SpcRam));
				cboType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SpcRom));
			}
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);
			UpdateByteLabel();
			txtLabel.Focus();
		}

		protected override bool ValidateInput()
		{
			UpdateObject();

			UInt32 address = ((CodeLabel)Entity).Address;
			UInt32 length = ((CodeLabel)Entity).Length;
			SnesMemoryType type = ((CodeLabel)Entity).MemoryType;
			CodeLabel sameLabel = LabelManager.GetLabel(txtLabel.Text);
			int maxAddress = DebugApi.GetMemorySize(type) - 1;

			if(maxAddress <= 0) {
				lblRange.Text = "(unavailable)";
			} else {
				lblRange.Text = "(Max: $" + maxAddress.ToString("X4") + ")";
			}

			for(UInt32 i = 0; i < length; i++) {
				CodeLabel sameAddress = LabelManager.GetLabel(address + i, type);
				if(sameAddress != null) {
					if(_originalLabel == null) {
						//A label already exists and we're not editing an existing label, so we can't add it
						return false;
					} else {
						if(sameAddress.Label != _originalLabel.Label && !sameAddress.Label.StartsWith(_originalLabel.Label + "+")) {
							//A label already exists, we're trying to edit an existing label, but the existing label
							//and the label we're editing aren't the same label.  Can't override an existing label with a different one.
							return false;
						}
					}
				}
			}

			return
				length >= 1 && length <= 65536 &&
				address + (length - 1) <= maxAddress &&
				(sameLabel == null || sameLabel == _originalLabel) 
				&& (txtLabel.Text.Length > 0 || txtComment.Text.Length > 0)
				&& !txtComment.Text.Contains('\x1')
				&& (txtLabel.Text.Length == 0 || LabelManager.LabelRegex.IsMatch(txtLabel.Text));
		}

		protected override void OnApply()
		{
			base.OnApply();

			LabelManager.DeleteLabel(_originalLabel, false);
			LabelManager.SetLabel((CodeLabel)this.Entity, true);
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(keyData == (Keys.Control | Keys.Enter)) {
				this.DialogResult = DialogResult.OK;
				this.Close();
			}
			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void nudLength_ValueChanged(object sender, EventArgs e)
		{
			UpdateByteLabel();
		}

		private void UpdateByteLabel()
		{
			if(nudLength.Value > 1) {
				lblBytes.Text = "bytes";
			} else {
				lblBytes.Text = "byte";
			}
		}
	}
}
