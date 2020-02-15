using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Controls
{
	class ctrlMemoryType : ComboBoxWithSeparator
	{
		private bool _disableEvent = false;

		protected override void OnSelectedIndexChanged(EventArgs e)
		{
			if(!_disableEvent) {
				base.OnSelectedIndexChanged(e);
			}
		}

		public void InitMemoryTypeDropdown(SnesMemoryType? newType = null, bool excludeCpuMemory = false)
		{
			this._disableEvent = true;

			SnesMemoryType originalValue = newType.HasValue ? newType.Value : this.GetEnumValue<SnesMemoryType>();

			this.BeginUpdate();
			this.Items.Clear();

			if(!excludeCpuMemory) {
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.CpuMemory));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SpcMemory));
				this.Items.Add("-");
			}
			this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.PrgRom));
			this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.WorkRam));
			if(DebugApi.GetMemorySize(SnesMemoryType.SaveRam) > 0) {
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SaveRam));
			}
			this.Items.Add("-");
			this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.VideoRam));
			this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.CGRam));
			this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SpriteRam));

			if(DebugApi.GetMemorySize(SnesMemoryType.DspProgramRom) > 0) {
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.DspProgramRom));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.DspDataRom));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.DspDataRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.Sa1InternalRam) > 0) {
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Sa1Memory));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Sa1InternalRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.GsuWorkRam) > 0) {
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GsuMemory));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GsuWorkRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.Cx4DataRam) > 0) {
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Cx4DataRam));
			}

			this.SelectedIndex = 0;
			this.SetEnumValue(originalValue);
			this._disableEvent = false;

			this.EndUpdate();
		}
	}
}
