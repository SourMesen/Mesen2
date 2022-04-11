using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.ViewModels
{
	public class MemoryMappingViewModel : ViewModelBase
	{
		private CpuType _cpuType;

		[Reactive] public List<MemoryMappingBlock> CpuMappings { get; private set; } = new();
		[Reactive] public List<MemoryMappingBlock>? PpuMappings { get; private set; } = null;

		[Obsolete("For designer only")]
		public MemoryMappingViewModel() : this(CpuType.Nes) { }

		public MemoryMappingViewModel(CpuType cpuType)
		{
			_cpuType = cpuType;

			if(Design.IsDesignMode) {
				return;
			}

			Refresh();
		}

		public void Refresh()
		{
			if(_cpuType == CpuType.Nes) {
				NesCartridgeState state = DebugApi.GetConsoleState<NesState>(ConsoleType.Nes).Cartridge;
				CpuMappings = GetNesCpuMappings(state);
				PpuMappings = GetNesPpuMappings(state);
			} else if(_cpuType == CpuType.Gameboy) {
				CpuMappings = GetGameboyCpuMappings(DebugApi.GetConsoleState<GbState>(ConsoleType.Gameboy));
			} else if(_cpuType == CpuType.Pce) {
				CpuMappings = GetPceCpuMappings(DebugApi.GetConsoleState<PceState>(ConsoleType.PcEngine).MemoryManager);
			}
		}

		private List<MemoryMappingBlock> GetNesCpuMappings(NesCartridgeState state)
		{
			Dictionary<NesPrgMemoryType, Color> mainColors = new() {
				{ NesPrgMemoryType.WorkRam, Color.FromRgb(0xCD, 0xDC, 0xFA) },
				{ NesPrgMemoryType.SaveRam, Color.FromRgb(0xFA, 0xDC, 0xCD) },
				{ NesPrgMemoryType.PrgRom, Color.FromRgb(0xC4, 0xE7, 0xD4) }
			};

			Dictionary<NesPrgMemoryType, Color> altColors = new() {
				{ NesPrgMemoryType.WorkRam, Color.FromRgb(0xBD, 0xCC, 0xEA) },
				{ NesPrgMemoryType.SaveRam, Color.FromRgb(0xEA, 0xCC, 0xBD) },
				{ NesPrgMemoryType.PrgRom, Color.FromRgb(0xA4, 0xD7, 0xB4) }
			};

			Dictionary<NesPrgMemoryType, string> blockNames = new() {
				{ NesPrgMemoryType.WorkRam, "WRAM" },
				{ NesPrgMemoryType.SaveRam, "SRAM" },
				{ NesPrgMemoryType.PrgRom, "" }
			};

			Dictionary<NesMemoryAccessType, string> accessNotes = new() {
				{ NesMemoryAccessType.Unspecified, "OB" },
				{ NesMemoryAccessType.NoAccess, "OB" },
				{ NesMemoryAccessType.Read, "R" },
				{ NesMemoryAccessType.Write, "W" },
				{ NesMemoryAccessType.ReadWrite, "RW" },
			};

			List<MemoryMappingBlock> mappings = new();

			mappings.Add(new MemoryMappingBlock() { Name = "Internal RAM", Length = 0x2000, Color = Color.FromRgb(222, 222, 222) });
			mappings.Add(new MemoryMappingBlock() { Name = "Registers", Length = 0x2020, Color = Color.FromRgb(222, 222, 222) });

			NesPrgMemoryType? memoryType = null;
			NesMemoryAccessType accessType = NesMemoryAccessType.Unspecified;
			int currentSize = -0x20;

			void addBlock(int i)
			{
				int startIndex = i - (currentSize / 0x100);

				if(memoryType.HasValue) {
					Color color = mainColors[memoryType.Value];

					if(mappings[^1].Color == color) {
						//Use alternative color if the previous color is identical
						color = altColors[memoryType.Value];
					}

					int page = memoryType.Value switch {
						NesPrgMemoryType.WorkRam => (int)(state.PrgMemoryOffset[startIndex] / state.WorkRamPageSize),
						NesPrgMemoryType.SaveRam => (int)(state.PrgMemoryOffset[startIndex] / state.SaveRamPageSize),
						_ or NesPrgMemoryType.PrgRom => (int)(state.PrgMemoryOffset[startIndex] / state.PrgPageSize)
					};

					mappings.Add(new MemoryMappingBlock() { Length = currentSize, Name = blockNames[memoryType.Value], Page = page, Note = accessNotes[accessType], Color = color });
				} else {
					mappings.Add(new MemoryMappingBlock() { Length = currentSize, Name = "N/A", Note = accessNotes[accessType], Color = Color.FromRgb(222, 222, 222) });
				}
				currentSize = 0;
			}

			for(int i = 0x40; i < 0x100; i++) {
				if(state.PrgMemoryAccess[i] != NesMemoryAccessType.NoAccess) {
					bool forceNewBlock = (
						(memoryType == NesPrgMemoryType.PrgRom && state.PrgMemoryOffset[i] % state.PrgPageSize == 0) ||
						(memoryType == NesPrgMemoryType.WorkRam && state.PrgMemoryOffset[i] % state.WorkRamPageSize == 0) ||
						(memoryType == NesPrgMemoryType.SaveRam && state.PrgMemoryOffset[i] % state.SaveRamPageSize == 0)
					);

					if(forceNewBlock || memoryType != state.PrgMemoryType[i] || state.PrgMemoryOffset[i] - state.PrgMemoryOffset[i - 1] != 0x100) {
						addBlock(i);
					}

					memoryType = state.PrgMemoryType[i];
					accessType = state.PrgMemoryAccess[i];
				} else {
					if(memoryType != null) {
						addBlock(i);
					}
					memoryType = null;
					accessType = NesMemoryAccessType.Unspecified;
				}
				currentSize += 0x100;
			}
			addBlock(0x100);

			return mappings;
		}

		private List<MemoryMappingBlock> GetNesPpuMappings(NesCartridgeState state)
		{
			List<MemoryMappingBlock> mappings = new();

			Dictionary<NesChrMemoryType, Color> mainColors = new() {
				{ NesChrMemoryType.NametableRam, Color.FromRgb(0xF4, 0xC7, 0xD4) },
				{ NesChrMemoryType.ChrRom, Color.FromRgb(0xC4, 0xE7, 0xD4) },
				{ NesChrMemoryType.ChrRam, Color.FromRgb(0xC4, 0xE0, 0xF4) }
			};

			Dictionary<NesChrMemoryType, Color> altColors = new() {
				{ NesChrMemoryType.NametableRam, Color.FromRgb(0xD4, 0xA7, 0xB4) },
				{ NesChrMemoryType.ChrRom, Color.FromRgb(0xA4, 0xD7, 0xB4) },
				{ NesChrMemoryType.ChrRam, Color.FromRgb(0xB4, 0xD0, 0xE4) }
			};

			Dictionary<NesMemoryAccessType, string> accessNotes = new() {
				{ NesMemoryAccessType.Unspecified, "OB" },
				{ NesMemoryAccessType.NoAccess, "OB" },
				{ NesMemoryAccessType.Read, "R" },
				{ NesMemoryAccessType.Write, "W" },
				{ NesMemoryAccessType.ReadWrite, "RW" },
			};

			NesChrMemoryType? memoryType = null;
			NesMemoryAccessType accessType = NesMemoryAccessType.Unspecified;
			int currentSize = 0;

			void addBlock(int i)
			{
				int startIndex = i - (currentSize / 0x100);

				if(memoryType.HasValue) {
					Color color = mainColors[memoryType.Value];

					if(mappings[^1].Color == color) {
						//Use alternative color if the previous color is identical
						color = altColors[memoryType.Value];
					}

					int page = memoryType.Value switch {
						NesChrMemoryType.NametableRam => (int)(state.ChrMemoryOffset[startIndex] / 0x400),
						NesChrMemoryType.ChrRom => (int)(state.ChrMemoryOffset[startIndex] / state.ChrPageSize),
						_ or NesChrMemoryType.ChrRam => (int)(state.ChrMemoryOffset[startIndex] / state.ChrRamPageSize),
					};

					string name = "";
					if(memoryType == NesChrMemoryType.NametableRam) {
						name = "NT" + page.ToString();
						page = -1;
					}

					mappings.Add(new MemoryMappingBlock() { Length = currentSize, Name = name, Page = page, Note = accessNotes[accessType], Color = color });
				} else {
					mappings.Add(new MemoryMappingBlock() { Length = currentSize, Name = "N/A", Note = accessNotes[accessType], Color = Color.FromRgb(222, 222, 222) });
				}
				currentSize = 0;
			}

			for(int i = 0x00; i < 0x30; i++) {
				if(state.ChrMemoryAccess[i] != NesMemoryAccessType.NoAccess) {
					bool forceNewBlock = (
						(memoryType == NesChrMemoryType.NametableRam && state.ChrMemoryOffset[i] % 0x400 == 0) ||
						(memoryType == NesChrMemoryType.ChrRom && state.ChrMemoryOffset[i] % state.ChrPageSize == 0) ||
						(memoryType == NesChrMemoryType.ChrRam && state.ChrMemoryOffset[i] % state.ChrRamPageSize == 0)
					);

					if(forceNewBlock || memoryType != state.ChrMemoryType[i] || state.ChrMemoryOffset[i] - state.ChrMemoryOffset[i - 1] != 0x100) {
						addBlock(i);
					}

					memoryType = state.ChrMemoryType[i];
					accessType = state.ChrMemoryAccess[i];
				} else {
					if(memoryType != null) {
						addBlock(i);
					}
					memoryType = null;
					accessType = NesMemoryAccessType.Unspecified;
				}
				currentSize += 0x100;
			}
			addBlock(0x30);

			return mappings;
		}

		private List<MemoryMappingBlock> GetGameboyCpuMappings(GbState gbState)
		{
			GbMemoryManagerState state = gbState.MemoryManager;
			List<MemoryMappingBlock> mappings = new();

			Dictionary<GbMemoryType, Color> mainColors = new() {
				{ GbMemoryType.BootRom, Color.FromRgb(222, 222, 222) },
				{ GbMemoryType.WorkRam, Color.FromRgb(0xCD, 0xDC, 0xFA) },
				{ GbMemoryType.CartRam, Color.FromRgb(0xFA, 0xDC, 0xCD) },
				{ GbMemoryType.PrgRom, Color.FromRgb(0xC4, 0xE7, 0xD4) }
			};

			Dictionary<GbMemoryType, Color> altColors = new() {
				{ GbMemoryType.BootRom, Color.FromRgb(222, 222, 222) },
				{ GbMemoryType.WorkRam, Color.FromRgb(0xBD, 0xCC, 0xEA) },
				{ GbMemoryType.CartRam, Color.FromRgb(0xEA, 0xCC, 0xBD) },
				{ GbMemoryType.PrgRom, Color.FromRgb(0xA4, 0xD7, 0xB4) }
			};

			Dictionary<GbMemoryType, string> blockNames = new() {
				{ GbMemoryType.BootRom, "BOOT" },
				{ GbMemoryType.WorkRam, "WRAM" },
				{ GbMemoryType.CartRam, gbState.HasBattery ? "SRAM" : "Cart RAM" },
				{ GbMemoryType.PrgRom, "" }
			};

			Dictionary<GbRegisterAccess, string> accessNotes = new() {
				{ GbRegisterAccess.None, "OB" },
				{ GbRegisterAccess.Read, "R" },
				{ GbRegisterAccess.Write, "W" },
				{ GbRegisterAccess.ReadWrite, "RW" },
			};

			GbMemoryType memoryType = GbMemoryType.None;
			GbRegisterAccess accessType = GbRegisterAccess.None;
			int currentSize = 0;
			
			const int prgBankSize = 0x4000;
			const int cartBankSize = 0x2000;
			int wramBankSize = gbState.Ppu.CgbEnabled ? 0x1000 : 0x2000;

			void addBlock(int i)
			{
				int startIndex = i - (currentSize / 0x100);

				if(memoryType != GbMemoryType.None) {
					Color color = mainColors[memoryType];
					if(mappings[^1].Color == color) {
						//Use alternative color if the previous color is identical
						color = altColors[memoryType];
					}

					int page = memoryType switch {
						GbMemoryType.BootRom => -1,
						GbMemoryType.WorkRam => (int)(state.MemoryOffset[startIndex] / wramBankSize),
						GbMemoryType.CartRam => (int)(state.MemoryOffset[startIndex] / cartBankSize),
						_ or GbMemoryType.PrgRom => (int)(state.MemoryOffset[startIndex] / prgBankSize)
					};

					mappings.Add(new MemoryMappingBlock() { Length = currentSize, Name = blockNames[memoryType], Page = page, Note = accessNotes[accessType], Color = color });
				} else {
					mappings.Add(new MemoryMappingBlock() { Length = currentSize, Name = "N/A", Note = accessNotes[accessType], Color = Color.FromRgb(222, 222, 222) });
				}
				currentSize = 0;
			}

			for(int i = 0; i < 0xFE; i++) {
				if(i == 0x80) {
					addBlock(i);
					mappings.Add(new MemoryMappingBlock() { Name = "VRAM", Length = 0x2000, Color = Color.FromRgb(0xFA, 0xDC, 0xCD) });
					currentSize = 0;
					memoryType = GbMemoryType.None;
					accessType = GbRegisterAccess.None;
					currentSize = 0;
					i += 0x20;
				}

				if(state.MemoryAccessType[i] != GbRegisterAccess.None) {
					bool forceNewBlock = (
						(memoryType == GbMemoryType.PrgRom && state.MemoryOffset[i] % prgBankSize == 0) ||
						(memoryType == GbMemoryType.WorkRam && state.MemoryOffset[i] % wramBankSize == 0) ||
						(memoryType == GbMemoryType.CartRam && state.MemoryOffset[i] % cartBankSize == 0)
					);

					if(forceNewBlock || memoryType != state.MemoryType[i] || state.MemoryOffset[i] - state.MemoryOffset[i - 1] != 0x100) {
						addBlock(i);
					}

					memoryType = state.MemoryType[i];
					accessType = state.MemoryAccessType[i];
				} else {
					if(memoryType != GbMemoryType.None) {
						addBlock(i);
					}
					memoryType = GbMemoryType.None;
					accessType = GbRegisterAccess.None;
				}
				currentSize += 0x100;
			}
			addBlock(0xFE);

			mappings.Add(new MemoryMappingBlock() { Name = "", Length = 0x200, Color = Color.FromRgb(222, 222, 222) });

			return mappings;
		}

		private List<MemoryMappingBlock> GetPceCpuMappings(PceMemoryManager state)
		{
			//TODO improve/complete logic for save ram, etc.
			List<MemoryMappingBlock> mappings = new();

			Dictionary<MemoryType, Color> mainColors = new() {
				{ MemoryType.Register, Color.FromRgb(222, 222, 222) },
				{ MemoryType.PceWorkRam, Color.FromRgb(0xCD, 0xDC, 0xFA) },
				//{ MemoryType.CartRam, Color.FromRgb(0xFA, 0xDC, 0xCD) },
				{ MemoryType.PcePrgRom, Color.FromRgb(0xC4, 0xE7, 0xD4) }
			};

			Dictionary<MemoryType, Color> altColors = new() {
				{ MemoryType.Register, Color.FromRgb(222, 222, 222) },
				{ MemoryType.PceWorkRam, Color.FromRgb(0xBD, 0xCC, 0xEA) },
				//{ MemoryType.CartRam, Color.FromRgb(0xEA, 0xCC, 0xBD) },
				{ MemoryType.PcePrgRom, Color.FromRgb(0xA4, 0xD7, 0xB4) }
			};

			Dictionary<MemoryType, string> blockNames = new() {
				{ MemoryType.Register, "REG" },
				{ MemoryType.PceWorkRam, "WRAM" },
				//{ MemoryType.CartRam, gbState.HasBattery ? "SRAM" : "Cart RAM" },
				{ MemoryType.PcePrgRom, "ROM" }
			};

			Dictionary<MemoryType, string> accessNotes = new() {
				{ MemoryType.Register, "RW" },
				{ MemoryType.PceWorkRam, "W" },
				//{ MemoryType.CartRam, gbState.HasBattery ? "SRAM" : "Cart RAM" },
				{ MemoryType.PcePrgRom, "R" },
			};

			for(int i = 0; i < 8; i++) {
				AddressInfo absAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = i * 0x2000, Type = MemoryType.PceMemory });
				if(absAddr.Address >= 0) {
					MemoryType memType = absAddr.Type;
					string note = "";
					if(memType == MemoryType.PcePrgRom && state.Mpr[i] != absAddr.Address / 0x2000) {
						note = " ($" + (absAddr.Address / 0x2000).ToString("X2") + ")";
					}
					mappings.Add(new MemoryMappingBlock() {
						Length = 0x2000,
						Name = blockNames[memType],
						Page = state.Mpr[i],
						Note = accessNotes[memType] + note,
						Color = (i % 2 == 0) ? mainColors[memType] : altColors[memType]
					});
				} else if(state.Mpr[i] == 0xFF) {
					MemoryType memType = MemoryType.Register;
					mappings.Add(new MemoryMappingBlock() {
						Length = 0x2000,
						Name = blockNames[memType],
						Page = state.Mpr[i],
						Note = accessNotes[memType],
						Color = (i % 2 == 0) ? mainColors[memType] : altColors[memType]
					});
				} else {
					mappings.Add(new MemoryMappingBlock() {
						Length = 0x2000,
						Name = "N/A",
						Note = "OB",
						Color = Color.FromRgb(222, 222, 222)
					});
				}
			}

			return mappings;
		}
	}
}