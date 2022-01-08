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
		public static DynamicTooltip? GetTooltip(CpuType cpuType, string text, CodeSegmentType segmentType)
		{
			FontFamily monoFont = ConfigManager.Config.Debug.Font.FontFamilyObject;

			int address = -1;
			CodeLabel? label = null;
			if(segmentType == CodeSegmentType.Address || segmentType == CodeSegmentType.EffectiveAddress) {
				string addressText = text.Trim(' ', '[', ']', '$');
				int.TryParse(addressText, System.Globalization.NumberStyles.HexNumber, null, out address);
			} else if(segmentType == CodeSegmentType.Label) {
				string labelText = text.Trim(' ', ',', ':', ']', '[');
				label = LabelManager.GetLabel(labelText);
				if(label != null) {
					address = label.GetRelativeAddress(cpuType).Address;
				}
			}

			if(address >= 0) {
				SnesMemoryType memType = cpuType.ToMemoryType();
				int byteValue = DebugApi.GetMemoryValue(memType, (uint)address);
				int wordValue = (DebugApi.GetMemoryValue(memType, (uint)address + 1) << 8) | byteValue;

				StackPanel mainPanel = new StackPanel() { Spacing = -4 };
				mainPanel.Children.Add(GetHexDecPanel(byteValue, "X2", monoFont));
				mainPanel.Children.Add(GetHexDecPanel(wordValue, "X4", monoFont));

				TooltipEntries items = new();
				string addressField = "$" + address.ToString("X" + cpuType.GetAddressSize()) + " (CPU)";
				AddressInfo absAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = memType });
				if(absAddr.Address >= 0) {
					addressField += Environment.NewLine + "$" + absAddr.Address.ToString("X" + cpuType.GetAddressSize()) + " (" + ResourceHelper.GetEnumText(absAddr.Type) + ")";
				}

				if(label != null) {
					items.AddEntry("Label", label.Label, monoFont);
				}

				items.AddEntry("Address", addressField, monoFont);
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
							StyleProvider = new BaseStyleProvider(),
							FontFamily = ConfigManager.Config.Debug.Font.FontFamily,
							FontSize = ConfigManager.Config.Debug.Font.FontSize - 1
						}
					});
				}

				return new DynamicTooltip() { Items = items };
			}
		
			return null;
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
