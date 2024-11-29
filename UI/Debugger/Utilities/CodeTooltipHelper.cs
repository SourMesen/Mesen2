using System;
using Avalonia.Controls;
using Mesen.Interop;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Avalonia.Media;
using Mesen.Debugger.Labels;
using Mesen.Localization;
using Mesen.Config;
using Mesen.Debugger.Integration;

namespace Mesen.Debugger.Utilities
{
	public static class CodeTooltipHelper
	{
		public static LocationInfo? GetLocation(CpuType cpuType, CodeSegmentInfo seg)
		{
			int address = -1;
			if(seg.Type == CodeSegmentType.EffectiveAddress) {
				if(seg.Data.EffectiveAddress >= 0) {
					MemoryType memType = seg.Data.EffectiveAddressType;
					if(memType.IsRelativeMemory()) {
						AddressInfo relAddress = new AddressInfo() { Address = (Int32)seg.Data.EffectiveAddress, Type = memType };
						AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
						return new LocationInfo {
							RelAddress = relAddress,
							AbsAddress = absAddress.Address >= 0 ? absAddress : null,
						};
					} else {
						//Used by e.g ports on the SMS
						return new LocationInfo {
							AbsAddress = new AddressInfo() { Address = (Int32)seg.Data.EffectiveAddress, Type = seg.Data.EffectiveAddressType }
						};
					}
				}
			} else if(seg.Type == CodeSegmentType.Address) {
				string addressText = seg.Text.Trim(' ', '[', ']', '$');
				int.TryParse(addressText, System.Globalization.NumberStyles.HexNumber, null, out address);
				if(address >= 0) {
					MemoryType memType = seg.Data.EffectiveAddress >= 0 ? seg.Data.EffectiveAddressType : cpuType.ToMemoryType();
					if(memType.IsRelativeMemory()) {
						AddressInfo relAddress = new AddressInfo() { Address = address, Type = memType };
						AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
						return new LocationInfo {
							RelAddress = relAddress,
							AbsAddress = absAddress.Address >= 0 ? absAddress : null,
						};
					} else {
						return new LocationInfo {
							AbsAddress = new AddressInfo() { Address = address, Type = memType }
						};
					}
				}
			} else if(seg.Type == CodeSegmentType.Label || seg.Type == CodeSegmentType.LabelDefinition || seg.Type == CodeSegmentType.Token) {
				string labelText = seg.Text.Trim(' ', ',', ':', ']', '[');
				int addressOffset = GetAddressOffset(seg);
				int plusIndex = labelText.IndexOf("+");
				if(plusIndex > 0) {
					labelText = labelText.Substring(0, plusIndex);
				}

				int lineAbsAddress = seg.Data.AbsoluteAddress.Address;
				SourceSymbol? symbol = lineAbsAddress >= 0 ? DebugWorkspaceManager.SymbolProvider?.GetSymbol(labelText, lineAbsAddress, lineAbsAddress + seg.Data.OpSize) : null;
				if(symbol != null) {
					AddressInfo absAddr = DebugWorkspaceManager.SymbolProvider?.GetSymbolAddressInfo(symbol.Value) ?? new AddressInfo() { Address = -1 };
					AddressInfo? relAddr = null;
					CodeLabel? label = null;
					if(absAddr.Address >= 0) {
						absAddr.Address += addressOffset;
						relAddr = DebugApi.GetRelativeAddress(absAddr, cpuType);
						label = LabelManager.GetLabel((uint)absAddr.Address, absAddr.Type);
					}
					return new LocationInfo {
						Symbol = symbol,
						Label = label,
						LabelAddressOffset = addressOffset > 0 ? addressOffset : null,
						RelAddress = relAddr?.Address >= 0 ? relAddr : null,
						AbsAddress = absAddr.Address >= 0 ? absAddr : null
					};
				} else {
					CodeLabel? label = LabelManager.GetLabel(labelText);
					if(label != null) {
						AddressInfo absAddr = label.GetAbsoluteAddress();
						absAddr.Address += addressOffset;
						//absAddr can be cpu memory for register labels (e.g on nes/pce/gb)
						AddressInfo relAddr = absAddr.Type.IsRelativeMemory() ? absAddr : DebugApi.GetRelativeAddress(absAddr, cpuType);
						return new LocationInfo {
							Label = label,
							LabelAddressOffset = addressOffset > 0 ? addressOffset : null,
							RelAddress = relAddr.Address >= 0 ? relAddr : null,
							AbsAddress = absAddr
						};
					}
				}
			} else if(seg.Type == CodeSegmentType.MarginAddress) {
				string addressText = seg.Text.Trim(' ', '[', ']', '$');
				if(int.TryParse(addressText, System.Globalization.NumberStyles.HexNumber, null, out address) && address >= 0) {
					AddressInfo relAddress = new AddressInfo() { Address = address, Type = cpuType.ToMemoryType() };
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					return new LocationInfo {
						RelAddress = relAddress,
						AbsAddress = absAddress.Address >= 0 ? absAddress : null,
					};
				}
			}

			return null;
		}

		private static int GetAddressOffset(CodeSegmentInfo seg)
		{
			int addressOffset = 0;
			string labelText = seg.Text;
			if(seg.OriginalTextIndex > 0 && seg.Data.Text.Length > seg.OriginalTextIndex) {
				string originalText = seg.Data.Text.Substring(seg.OriginalTextIndex);
				if(originalText.StartsWith(labelText + "+")) {
					int start = labelText.Length + 1;
					int i = start;
					while(i < originalText.Length && char.IsDigit(originalText[i])) {
						i++;
					}

					if(i > start) {
						Int32.TryParse(originalText.Substring(start, i - start), out addressOffset);
					}
					return addressOffset;
				}
			} else if(seg.Text.Trim().StartsWith("[") && seg.Text.Contains("+")) {
				//Effective address that contains a multi-byte label
				string lbl = seg.Text.Trim(' ', ',', ':', ']', '[');
				int start = lbl.IndexOf('+') + 1;
				int i = start;
				while(i < lbl.Length && char.IsDigit(lbl[i])) {
					i++;
				}

				if(i > start) {
					Int32.TryParse(lbl.Substring(start, i - start), out addressOffset);
				}

				return addressOffset;
			}

			return 0;
		}

		public static DynamicTooltip? GetTooltip(CpuType cpuType, CodeSegmentInfo seg)
		{
			if(seg.Type == CodeSegmentType.OpCode) {
				return OpCodeHelper.GetTooltip(seg);
			} else if(seg.Type == CodeSegmentType.MarginAddress) {
				return GetMarginAddressTooltip(cpuType, seg);
			} else if(seg.Type == CodeSegmentType.InstructionProgress) {
				return GetInstructionProgressTooltip(cpuType, seg);
			} else {
				if(seg.Type == CodeSegmentType.Address || seg.Type == CodeSegmentType.EffectiveAddress || seg.Type == CodeSegmentType.Label || seg.Type == CodeSegmentType.LabelDefinition) {
					LocationInfo? codeLoc = GetLocation(cpuType, seg);
					if(codeLoc != null && (codeLoc.RelAddress?.Address >= 0 || codeLoc.AbsAddress?.Address >= 0 || codeLoc.Label != null || codeLoc.Symbol != null)) {
						return GetCodeAddressTooltip(cpuType, codeLoc, !codeLoc.RelAddress.HasValue || codeLoc.RelAddress?.Address < 0);
					}
				}
			}

			return null;
		}

		private static DynamicTooltip GetMarginAddressTooltip(CpuType cpuType, CodeSegmentInfo seg)
		{
			MemoryType memType = cpuType.ToMemoryType();

			TooltipEntries items = new();
			int address = seg.Data.HasAddress ? seg.Data.Address : -1;

			string addressField = "";
			if(seg.Data.HasAddress) {
				addressField += "$" + address.ToString("X" + cpuType.GetAddressSize()) + " (" + memType.GetShortName() + ")";
			}

			AddressInfo absAddr = seg.Data.AbsoluteAddress;
			if(absAddr.Address < 0) {
				absAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = memType });
			}

			if(absAddr.Address >= 0) {
				if(!string.IsNullOrEmpty(addressField)) {
					addressField += Environment.NewLine;
				}
				addressField += "$" + absAddr.Address.ToString("X" + cpuType.GetAddressSize()) + " (" + absAddr.Type.GetShortName() + ")";
			}

			items.AddEntry("Address", addressField, true);

			bool isCode = false;
			if(address >= 0) {
				isCode = DebugApi.GetCdlData((uint)address, 1, memType)[0].HasFlag(CdlFlags.Code);
			} else if(seg.Data.AbsoluteAddress.Address >= 0) {
				isCode = DebugApi.GetCdlData((uint)seg.Data.AbsoluteAddress.Address, 1, seg.Data.AbsoluteAddress.Type)[0].HasFlag(CdlFlags.Code);
			}

			if(isCode) {
				items.AddEntry("Byte code", seg.Data.ByteCodeStr, true);
			}

			return new DynamicTooltip() { Items = items };
		}

		public static DynamicTooltip GetCodeAddressTooltip(CpuType cpuType, LocationInfo codeLoc, bool useAbsAddress = false)
		{
			int relAddress = codeLoc.RelAddress?.Address ?? -1;
			AddressInfo? absAddress = codeLoc.AbsAddress ?? (codeLoc.RelAddress.HasValue ? DebugApi.GetAbsoluteAddress(codeLoc.RelAddress.Value) : null);
			FontFamily monoFont = new FontFamily(ConfigManager.Config.Debug.Fonts.OtherMonoFont.FontFamily);
			double fontSize = ConfigManager.Config.Debug.Fonts.OtherMonoFont.FontSize;
			MemoryType cpuMemType = cpuType.ToMemoryType();
			TooltipEntries items = new();

			if(codeLoc.Symbol != null) {
				items.AddEntry("Symbol", codeLoc.Symbol.Value.Name + (codeLoc.LabelAddressOffset != null ? ("+" + codeLoc.LabelAddressOffset) : ""));
			} else if(codeLoc.Label != null) {
				items.AddEntry("Label", codeLoc.Label.Label + (codeLoc.LabelAddressOffset != null ? ("+" + codeLoc.LabelAddressOffset) : ""));
			}

			bool showPreview = false;
			if(relAddress >= 0 || absAddress?.Address >= 0) {
				uint valueAddress = (uint)(absAddress?.Address >= 0 ? absAddress.Value.Address : relAddress);
				MemoryType valueMemType = absAddress?.Address >= 0 ? absAddress.Value.Type : cpuMemType;
				
				AddressCounters counters = DebugApi.GetMemoryAccessCounts(valueAddress, 1, valueMemType)[0];
				if(counters.ExecStamp > 0) {
					showPreview = true;
				} else if(valueMemType.SupportsCdl()) {
					showPreview = DebugApi.GetCdlData(valueAddress, 1, valueMemType)[0].HasFlag(CdlFlags.Code);
				}

				int byteValue = DebugApi.GetMemoryValue(valueMemType, valueAddress);
				int wordValue;
				int dwordValue;
				if(useAbsAddress && absAddress != null) {
					wordValue = (DebugApi.GetMemoryValue(absAddress.Value.Type, (uint)absAddress.Value.Address + 1) << 8) | byteValue;
					dwordValue = (
						(DebugApi.GetMemoryValue(absAddress.Value.Type, (uint)absAddress.Value.Address + 3) << 24) |
						(DebugApi.GetMemoryValue(absAddress.Value.Type, (uint)absAddress.Value.Address + 2) << 16) |
						wordValue
					);
				} else {
					wordValue = (DebugApi.GetMemoryValue(cpuMemType, (uint)relAddress + 1) << 8) | byteValue;
					dwordValue = (
						(DebugApi.GetMemoryValue(cpuMemType, (uint)relAddress + 3) << 24) |
						(DebugApi.GetMemoryValue(cpuMemType, (uint)relAddress + 2) << 16) |
						wordValue
					);
				}

				StackPanel mainPanel = new StackPanel() { Spacing = -4, Margin = new Avalonia.Thickness(0, -1, 0, 0) };
				mainPanel.Children.Add(GetHexDecPanel(byteValue, "X2", monoFont, fontSize));
				mainPanel.Children.Add(GetHexDecPanel(wordValue, "X4", monoFont, fontSize));
				if(cpuType.GetConsoleType() == ConsoleType.Gba || cpuType.GetConsoleType() == ConsoleType.Snes) {
					//Only show 32-bit values for GBA/SNES since these are the 2 systems that are most likely to be using 32-bit values in memory
					mainPanel.Children.Add(GetHexDecPanel(dwordValue, "X8", monoFont, fontSize));
				}

				items.AddEntry("Address", GetAddressField(relAddress, absAddress, cpuType, monoFont, fontSize), true);
				items.AddEntry("Value", mainPanel);

				if(counters.ReadCounter > 0 || counters.WriteCounter > 0 || counters.ExecCounter > 0) {
					TimingInfo timing = EmuApi.GetTimingInfo(cpuType);
					double clocksPerFrame = timing.MasterClockRate / timing.Fps;

					string accessData = (
						(counters.ReadCounter > 0 ? ($"R: {FormatCount(counters.ReadCounter)} - {FormatFrameCount(counters.ReadStamp, timing.MasterClock, clocksPerFrame)}" + Environment.NewLine) : "") +
						(counters.WriteCounter > 0 ? ($"W: {FormatCount(counters.WriteCounter)} - {FormatFrameCount(counters.WriteStamp, timing.MasterClock, clocksPerFrame)}" + Environment.NewLine) : "") +
						(counters.ExecCounter > 0 ? ($"X: {FormatCount(counters.ExecCounter)} - {FormatFrameCount(counters.ExecStamp, timing.MasterClock, clocksPerFrame)}") : "")
					).Trim('\n', '\r');

					items.AddEntry("Stats", accessData);
				}
			} else if(codeLoc.Symbol?.Address != null) {
				AddressInfo? symbolAddr = DebugWorkspaceManager.SymbolProvider?.GetSymbolAddressInfo(codeLoc.Symbol.Value);
				if(symbolAddr == null) {
					items.AddEntry("Constant", "$" + codeLoc.Symbol.Value.Address.Value.ToString("X" + cpuType.GetAddressSize()));
				} else {
					int byteValue = DebugApi.GetMemoryValue(symbolAddr.Value.Type, (uint)symbolAddr.Value.Address);
					int wordValue = (DebugApi.GetMemoryValue(symbolAddr.Value.Type, (uint)symbolAddr.Value.Address + 1) << 8) | byteValue;

					StackPanel mainPanel = new StackPanel() { Spacing = -4 };
					mainPanel.Children.Add(GetHexDecPanel(byteValue, "X2", monoFont, ConfigManager.Config.Debug.Fonts.OtherMonoFont.FontSize));
					mainPanel.Children.Add(GetHexDecPanel(wordValue, "X4", monoFont, ConfigManager.Config.Debug.Fonts.OtherMonoFont.FontSize));
					items.AddEntry("Address", "$" + symbolAddr.Value.Address.ToString("X" + cpuType.GetAddressSize()) + " (" + symbolAddr.Value.Type.GetShortName() + ")", true);
					items.AddEntry("Value", mainPanel);
				}
			}

			if(codeLoc.Label?.Comment.Length > 0) {
				items.AddEntry("Comment", codeLoc.Label.Comment, true);
			}

			if(relAddress >= 0 && showPreview) {
				items.AddEntry("", new Border() {
					BorderBrush = Brushes.Gray,
					BorderThickness = new(1),
					Child = new DisassemblyViewer() {
						Width = 300,
						Height = 150,
						Lines = new CodeDataProvider(cpuType).GetCodeLines(relAddress, 40),
						StyleProvider = new BaseStyleProvider(cpuType),
						FontFamily = ConfigManager.Config.Debug.Fonts.OtherMonoFont.FontFamily,
						FontSize = ConfigManager.Config.Debug.Fonts.OtherMonoFont.FontSize
					}
				});
			}

			return new DynamicTooltip() { Items = items };
		}

		private static DynamicTooltip? GetInstructionProgressTooltip(CpuType cpuType, CodeSegmentInfo seg)
		{
			if(seg.Progress == null) {
				return null;
			}

			CpuInstructionProgress progress = seg.Progress.CpuProgress;
			MemoryOperationInfo operation = progress.LastMemOperation;

			TooltipEntries items = new();
			items.AddEntry("Type", ResourceHelper.GetEnumText(operation.Type), true);
			items.AddEntry("Cycle", seg.Progress.Current.ToString(), true);
			if(operation.Type != MemoryOperationType.Idle) {
				items.AddEntry("Address", "$" + operation.Address.ToString("X" + cpuType.GetAddressSize()), true);
				items.AddEntry("Value", "$" + operation.Value.ToString("X2"), true);
			}

			return new DynamicTooltip() { Items = items };
		}

		private static StackPanel GetAddressField(int relAddress, AddressInfo? absAddr, CpuType cpuType, FontFamily font, double fontSize)
		{
			StackPanel panel = new StackPanel() { Spacing = -4, Margin = new Avalonia.Thickness(0, -1, 0, 0) };
			if(relAddress >= 0) {
				panel.Children.Add(new TextBlock() { Text = "$" + relAddress.ToString("X" + cpuType.GetAddressSize()) + " (CPU)", FontFamily = font, FontSize = fontSize });
			}
			if(absAddr?.Address >= 0) {
				panel.Children.Add(new TextBlock() { Text = "$" + absAddr.Value.Address.ToString("X" + cpuType.GetAddressSize()) + " (" + absAddr.Value.Type.GetShortName() + ")", FontFamily = font, FontSize = fontSize });
			}
			return panel;
		}

		private static StackPanel GetHexDecPanel(int value, string format, FontFamily font, double fontSize)
		{
			StackPanel panel = new StackPanel() { Orientation = Avalonia.Layout.Orientation.Horizontal };
			panel.Children.Add(new TextBlock() { Text = "$" + value.ToString(format), FontFamily = font, FontSize = fontSize });
			panel.Children.Add(new TextBlock() { Text = "  (" + value.ToString() + ")", FontFamily = font, FontSize = fontSize, Foreground = Brushes.DimGray, VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center });
			return panel;
		}

		public static string FormatCount(UInt64 value, UInt64 stamp = UInt64.MaxValue)
		{
			if(stamp == 0) {
				return "n/a";
			}

			return FormatValue(value);
		}

		public static string FormatValue(double value, double minimum = 0)
		{
			if(value > minimum) {
				if(value >= 1000000000000) {
					return (value / 1000000000000).ToString("0.00") + " T";
				} else if(value >= 1000000000) {
					return (value / 1000000000).ToString("0.00") + " G";
				} else if(value >= 1000000) {
					return (value / 1000000).ToString("0.00") + " M";
				} else if(value >= 1000) {
					return (value / 1000).ToString("0.00") + " K";
				}
			}
			return value.ToString("0.##");
		}

		private static string FormatFrameCount(UInt64 stamp, UInt64 masterClock, double clocksPerFrame)
		{
			if(stamp == 0 || stamp > masterClock) {
				return "n/a";
			}

			return FormatValue((masterClock - stamp) / clocksPerFrame) + " frames";
		}
	}
}
