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
			if(seg.Type == CodeSegmentType.Address || seg.Type == CodeSegmentType.EffectiveAddress) {
				string addressText = seg.Text.Trim(' ', '[', ']', '$');
				int.TryParse(addressText, System.Globalization.NumberStyles.HexNumber, null, out address);
				if(address >= 0) {
					AddressInfo relAddress = new AddressInfo() { Address = address, Type = cpuType.ToMemoryType() };
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					return new LocationInfo { 
						RelAddress = relAddress,
						AbsAddress = absAddress.Address >= 0 ? absAddress : null,
					};
				}
			} else if(seg.Type == CodeSegmentType.Label || seg.Type == CodeSegmentType.LabelDefinition) {
				string labelText = seg.Text.Trim(' ', ',', ':', ']', '[');

				int absAddress = seg.Data.AbsoluteAddress.Address;
				SourceSymbol? symbol = absAddress >= 0 ? DebugWorkspaceManager.SymbolProvider?.GetSymbol(labelText, absAddress, absAddress + seg.Data.OpSize) : null;
				if(symbol != null) {
					AddressInfo? absAddr = DebugWorkspaceManager.SymbolProvider?.GetSymbolAddressInfo(symbol.Value);
					AddressInfo? relAddr = null;
					if(absAddr?.Address >= 0) {
						relAddr = DebugApi.GetRelativeAddress(absAddr.Value, cpuType);
					}
					return new LocationInfo {
						Symbol = symbol,
						RelAddress = relAddr?.Address >= 0 ? relAddr : null,
						AbsAddress = absAddr?.Address >= 0 ? absAddr : null
					};
				} else {
					CodeLabel? label = LabelManager.GetLabel(labelText);
					if(label != null) {
						AddressInfo relAddress = label.GetRelativeAddress(cpuType);
						return new LocationInfo {
							Label = label,
							RelAddress = relAddress.Address >= 0 ? relAddress : null,
							AbsAddress = label.GetAbsoluteAddress()
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

		public static DynamicTooltip? GetTooltip(CpuType cpuType, CodeSegmentInfo seg)
		{
			if(seg.Type == CodeSegmentType.OpCode) {
				return OpCodeHelper.GetTooltip(seg);
			} else if(seg.Type == CodeSegmentType.MarginAddress) {
				return GetMarginAddressTooltip(cpuType, seg);
			} else {
				if(seg.Type == CodeSegmentType.Address || seg.Type == CodeSegmentType.EffectiveAddress || seg.Type == CodeSegmentType.Label || seg.Type == CodeSegmentType.LabelDefinition) {
					LocationInfo? codeLoc = GetLocation(cpuType, seg);
					if(codeLoc != null && (codeLoc.RelAddress?.Address >= 0 || codeLoc.Symbol != null)) {
						return GetCodeAddressTooltip(cpuType, codeLoc.RelAddress?.Address ?? -1, codeLoc.Label, codeLoc.Symbol);
					}
				}
			}

			return null;
		}

		private static DynamicTooltip GetMarginAddressTooltip(CpuType cpuType, CodeSegmentInfo seg)
		{
			FontFamily monoFont = new FontFamily(ConfigManager.Config.Debug.Font.FontFamily);
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

			items.AddEntry("Address", addressField, monoFont);

			bool isCode = false;
			if(address >= 0) {
				isCode = DebugApi.GetCdlData((uint)address, 1, memType)[0].HasFlag(CdlFlags.Code);
			} else if(seg.Data.AbsoluteAddress.Address >= 0) {
				isCode = DebugApi.GetCdlData((uint)seg.Data.AbsoluteAddress.Address, 1, seg.Data.AbsoluteAddress.Type)[0].HasFlag(CdlFlags.Code);
			}

			if(isCode) {
				items.AddEntry("Byte code", seg.Data.ByteCodeStr, monoFont);
			}

			return new DynamicTooltip() { Items = items };
		}

		private static DynamicTooltip GetCodeAddressTooltip(CpuType cpuType, int address, CodeLabel? label, SourceSymbol? symbol)
		{
			FontFamily monoFont = new FontFamily(ConfigManager.Config.Debug.Font.FontFamily);
			MemoryType memType = cpuType.ToMemoryType();
			TooltipEntries items = new();

			if(symbol != null) {
				items.AddEntry("Symbol", symbol.Value.Name, monoFont);
			} else if(label != null) {
				items.AddEntry("Label", label.Label, monoFont);
			}

			if(address >= 0) {
				int byteValue = DebugApi.GetMemoryValue(memType, (uint)address);
				int wordValue = (DebugApi.GetMemoryValue(memType, (uint)address + 1) << 8) | byteValue;

				StackPanel mainPanel = new StackPanel() { Spacing = -4 };
				mainPanel.Children.Add(GetHexDecPanel(byteValue, "X2", monoFont));
				mainPanel.Children.Add(GetHexDecPanel(wordValue, "X4", monoFont));

				items.AddEntry("Address", GetAddressField(cpuType, address, memType), monoFont);
				items.AddEntry("Value", mainPanel);
			} else if(symbol?.Address != null) {
				AddressInfo? symbolAddr = DebugWorkspaceManager.SymbolProvider?.GetSymbolAddressInfo(symbol.Value);
				if(symbolAddr == null) {
					items.AddEntry("Constant", "$" + symbol.Value.Address.Value.ToString("X" + cpuType.GetAddressSize()));
				} else {
					int byteValue = DebugApi.GetMemoryValue(symbolAddr.Value.Type, (uint)symbolAddr.Value.Address);
					int wordValue = (DebugApi.GetMemoryValue(symbolAddr.Value.Type, (uint)symbolAddr.Value.Address + 1) << 8) | byteValue;

					StackPanel mainPanel = new StackPanel() { Spacing = -4 };
					mainPanel.Children.Add(GetHexDecPanel(byteValue, "X2", monoFont));
					mainPanel.Children.Add(GetHexDecPanel(wordValue, "X4", monoFont));
					items.AddEntry("Address", "$" + symbolAddr.Value.Address.ToString("X" + cpuType.GetAddressSize()) + " (" + symbolAddr.Value.Type.GetShortName() + ")", monoFont);
					items.AddEntry("Value", mainPanel);
				}
			}

			if(label?.Comment.Length > 0) {
				items.AddEntry("Comment", label.Comment, monoFont);
			}

			if(address >= 0) {
				bool showPreview = DebugApi.GetCdlData((uint)address, 1, memType)[0].HasFlag(CdlFlags.Code);
				if(showPreview) {
					items.AddEntry("", new Border() {
						BorderBrush = Brushes.Gray,
						BorderThickness = new(1),
						Child = new DisassemblyViewer() {
							Width = 300,
							Height = 150,
							Lines = new CodeDataProvider(cpuType).GetCodeLines(address, 40),
							StyleProvider = new BaseStyleProvider(cpuType),
							FontFamily = ConfigManager.Config.Debug.Font.FontFamily,
							FontSize = ConfigManager.Config.Debug.Font.FontSize - 1
						}
					});
				}
			}

			return new DynamicTooltip() { Items = items };
		}

		private static string GetAddressField(CpuType cpuType, int address, MemoryType memType)
		{
			string addressField = "$" + address.ToString("X" + cpuType.GetAddressSize()) + " (CPU)";
			AddressInfo absAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = memType });
			if(absAddr.Address >= 0) {
				addressField += Environment.NewLine + "$" + absAddr.Address.ToString("X" + cpuType.GetAddressSize()) + " (" + absAddr.Type.GetShortName() + ")";
			}

			return addressField;
		}

		private static StackPanel GetHexDecPanel(int value, string format, FontFamily font)
		{
			StackPanel panel = new StackPanel() { Orientation = Avalonia.Layout.Orientation.Horizontal };
			panel.Children.Add(new TextBlock() { Text = "$" + value.ToString(format), FontFamily = font, FontSize = 12 });
			panel.Children.Add(new TextBlock() { Text = "  (" + value.ToString() + ")", FontFamily = font, FontSize = 12, Foreground = Brushes.DimGray, VerticalAlignment = Avalonia.Layout.VerticalAlignment.Center });
			return panel;
		}
	}
}
