using Avalonia.Controls;
using Mesen.Interop;
using Mesen.Localization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public static class ExpressionTooltipHelper
	{
		public static StackPanel GetHelpTooltip(CpuType cpuType, bool forWatch)
		{
			StackPanel panel = new();

			void addRow(string text) { panel.Children.Add(new TextBlock() { Text = text }); }
			void addBoldRow(string text) { panel.Children.Add(new TextBlock() { Text = text, FontWeight = Avalonia.Media.FontWeight.Bold }); }

			addBoldRow("Notes");
			addRow("  -Use C++ syntax - most expressions/operators are accepted.");
			addRow("  -Use the $ or 0x prefixes to denote hexadecimal values.");
			addRow("  -Labels can be used in expressions");
			addRow(" ");
			addBoldRow("Available values (" + ResourceHelper.GetEnumText(cpuType) + ")");

			string[] tokens = DebugApi.GetTokenList(cpuType);

			Grid tokenGrid = new Grid() {
				ColumnDefinitions = new("Auto, Auto, Auto, Auto"),
				RowDefinitions = new(string.Join(",", Enumerable.Repeat("Auto", (tokens.Length / 4) + 1))),
				Margin = new Avalonia.Thickness(5, 0, 0, 0)
			};

			int col = 0;
			int row = 0;
			foreach(string token in tokens) {
				TextBlock txt = new() { Text = token, Padding = new Avalonia.Thickness(0, 0, 5, 0) };
				tokenGrid.Children.Add(txt);
				Grid.SetColumn(txt, col);
				Grid.SetRow(txt, row);
				col++;
				if(col == 4) {
					col = 0;
					row++;
				}
			}

			panel.Children.Add(tokenGrid);

			if(!forWatch) {
				addRow(" ");
				addBoldRow("Other values");
				addRow("  OpPc: Program counter of the first byte of the current instruction");
				addRow("  Address: CPU memory address being read/written");
				addRow("  MemAddress: RAM or ROM address (byte offset) being read/written (-1 if not mapped to RAM/ROM)");
				addRow("  Value: Value being read/written");
				addRow("  IsRead: True if the CPU is reading data");
				addRow("  IsWrite: True if the CPU is writing data");
				addRow("  IsDma: True if the operation was triggered by DMA");
				addRow("  IsDummy: True if this is a \"dummy\" read or write");
			}

			addRow(" ");
			addBoldRow("Accessing memory");
			addRow("  [<address>] - 8-bit memory value at <address>");
			addRow("  {<address>} - 16-bit memory value at <address>");
			addRow("  #<address> - 32-bit memory value at <address>");
			addRow("  :<address> - Returns the ROM/RAM address for the specified CPU address");

			addRow(" ");
			addBoldRow("Examples");
			addRow("  a == 10 || x == $23");
			addRow("  scanline == 10 && (cycle >= 55 && cycle <= 100)");
			addRow("  x == [$150] || y == [10]");
			addRow("  [[$15] + y] -- Read value at $15, add Y to it and return the value stored at the resulting address.");
			addRow("  {$FFEA} -- NMI handler's address.");
			addRow("  #r0 -- Returns the 32-bit value stored at the address stored in r0.");
			addRow("  #(r0+20) -- Returns the 32-bit value stored at the address stored in r0, offset by 20.");

			return panel;
		}
	}

	public interface IToolHelpTooltip
	{
		object HelpTooltip { get; }
	}
}
