using System;
using Avalonia.Controls;
using Mesen.Interop;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Disassembly;
using Avalonia.Media;
using Mesen.Debugger.Labels;
using Mesen.Localization;
using Mesen.Config;

namespace Mesen.Debugger.Utilities
{
	public static class CodeTooltipHelper
	{
		public static LocationInfo? GetLocation(CpuType cpuType, CodeSegmentInfo codeSegment)
		{
			int address = -1;
			if(codeSegment.Type == CodeSegmentType.Address || codeSegment.Type == CodeSegmentType.EffectiveAddress) {
				string addressText = codeSegment.Text.Trim(' ', '[', ']', '$');
				int.TryParse(addressText, System.Globalization.NumberStyles.HexNumber, null, out address);
				if(address >= 0) {
					AddressInfo relAddress = new AddressInfo() { Address = address, Type = cpuType.ToMemoryType() };
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					return new LocationInfo { 
						RelAddress = relAddress,
						AbsAddress = absAddress.Address >= 0 ? absAddress : null,
					};
				}
			} else if(codeSegment.Type == CodeSegmentType.Label || codeSegment.Type == CodeSegmentType.LabelDefinition) {
				string labelText = codeSegment.Text.Trim(' ', ',', ':', ']', '[');
				CodeLabel? label = LabelManager.GetLabel(labelText);
				if(label != null) {
					return new LocationInfo {
						Label = label,
						RelAddress = label.GetRelativeAddress(cpuType),
						AbsAddress = label.GetAbsoluteAddress()
					};
				}
			} else if(codeSegment.Type == CodeSegmentType.MarginAddress) {
				string addressText = codeSegment.Text.Trim(' ', '[', ']', '$');
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

		public static DynamicTooltip? GetTooltip(CpuType cpuType, CodeSegmentInfo codeSegment)
		{
			if(codeSegment.Type == CodeSegmentType.OpCode) {
				return OpCodeHelper.GetTooltip(codeSegment);
			} else {
				LocationInfo? codeLoc = GetLocation(cpuType, codeSegment);
				if(codeLoc != null) {
					if(codeSegment.Type == CodeSegmentType.Address || codeSegment.Type == CodeSegmentType.EffectiveAddress || codeSegment.Type == CodeSegmentType.Label || codeSegment.Type == CodeSegmentType.LabelDefinition) {
						if(codeLoc.RelAddress?.Address >= 0) {
							return GetCodeAddressTooltip(cpuType, codeLoc.RelAddress.Value.Address, codeLoc.Label);
						}
					} else if(codeSegment.Type == CodeSegmentType.MarginAddress) {
						if(codeLoc.RelAddress?.Address >= 0) {
							return GetMarginAddressTooltip(cpuType, codeSegment, codeLoc.RelAddress.Value.Address);
						}
					}
				}
			}

			return null;
		}

		private static DynamicTooltip GetMarginAddressTooltip(CpuType cpuType, CodeSegmentInfo codeSegment, int address)
		{
			FontFamily monoFont = ConfigManager.Config.Debug.Font.FontFamilyObject;
			MemoryType memType = cpuType.ToMemoryType();
			bool isCode = DebugApi.GetCdlData((uint)address, 1, memType)[0].HasFlag(CdlFlags.Code);

			TooltipEntries items = new();
			items.AddEntry("Address", GetAddressField(cpuType, address, memType), monoFont);
			if(isCode) {
				items.AddEntry("Byte code", codeSegment.Data.ByteCodeStr, monoFont);
			}

			return new DynamicTooltip() { Items = items };
		}

		private static DynamicTooltip GetCodeAddressTooltip(CpuType cpuType, int address, CodeLabel? label)
		{
			FontFamily monoFont = ConfigManager.Config.Debug.Font.FontFamilyObject;
			MemoryType memType = cpuType.ToMemoryType();
			int byteValue = DebugApi.GetMemoryValue(memType, (uint)address);
			int wordValue = (DebugApi.GetMemoryValue(memType, (uint)address + 1) << 8) | byteValue;

			StackPanel mainPanel = new StackPanel() { Spacing = -4 };
			mainPanel.Children.Add(GetHexDecPanel(byteValue, "X2", monoFont));
			mainPanel.Children.Add(GetHexDecPanel(wordValue, "X4", monoFont));

			TooltipEntries items = new();

			if(label != null) {
				items.AddEntry("Label", label.Label, monoFont);
			}

			items.AddEntry("Address", GetAddressField(cpuType, address, memType), monoFont);
			items.AddEntry("Value", mainPanel);

			if(label?.Comment.Length > 0) {
				items.AddEntry("Comment", label.Comment, monoFont);
			}

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
