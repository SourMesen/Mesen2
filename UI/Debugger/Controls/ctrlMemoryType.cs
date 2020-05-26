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

			if(EmuApi.GetRomInfo().CoprocessorType == CoprocessorType.Gameboy) {
				this.AddGameboyTypes(excludeCpuMemory);
			} else {
				this.AddSnesMemoryTypes(excludeCpuMemory);
				this.AddGameboyTypes(excludeCpuMemory);
			}

			this.SelectedIndex = 0;
			this.SetEnumValue(originalValue);
			this._disableEvent = false;

			this.EndUpdate();
		}

		private void AddGameboyTypes(bool excludeCpuMemory)
		{
			if(DebugApi.GetMemorySize(SnesMemoryType.GbPrgRom) > 0) {
				if(this.Items.Count > 0) {
					this.Items.Add("-");
				}

				if(!excludeCpuMemory) {
					this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GameboyMemory));
				}
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbPrgRom));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbWorkRam));
				if(DebugApi.GetMemorySize(SnesMemoryType.GbCartRam) > 0) {
					this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbCartRam));
				}
				if(DebugApi.GetMemorySize(SnesMemoryType.GbBootRom) > 0) {
					this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbBootRom));
				}
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbVideoRam));
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GbSpriteRam));
			}
		}

		private void AddSnesMemoryTypes(bool excludeCpuMemory)
		{
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
				if(!excludeCpuMemory) {
					this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Sa1Memory));
				}
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Sa1InternalRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.GsuWorkRam) > 0) {
				this.Items.Add("-");
				if(!excludeCpuMemory) {
					this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GsuMemory));
				}
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.GsuWorkRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.Cx4DataRam) > 0) {
				this.Items.Add("-");
				if(!excludeCpuMemory) {
					this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Cx4Memory));
				}
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.Cx4DataRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.BsxPsRam) > 0) {
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.BsxPsRam));
			}

			if(DebugApi.GetMemorySize(SnesMemoryType.BsxMemoryPack) > 0) {
				this.Items.Add("-");
				this.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.BsxMemoryPack));
			}
		}
	}
}
