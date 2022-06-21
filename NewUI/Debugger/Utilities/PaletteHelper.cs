using Mesen.Debugger.Controls;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public class PaletteHelper
	{
		public static DynamicTooltip? GetPreviewPanel(UInt32[] rgbPalette, UInt32[] rawPalette, RawPaletteFormat format, int index, DynamicTooltip? tooltipToUpdate)
		{
			TooltipEntries entries = tooltipToUpdate?.Items ?? new();
			entries.StartUpdate();

			entries.AddEntry("Color", new TooltipColorEntry(rgbPalette[index]));
			entries.AddEntry("Index", "$" + index.ToString("X2"));
			if(format == RawPaletteFormat.Rgb555) {
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X4"));
				entries.AddEntry("R", "$" + (rawPalette[index] & 0x1F).ToString("X2"));
				entries.AddEntry("G", "$" + ((rawPalette[index] >> 5) & 0x1F).ToString("X2"));
				entries.AddEntry("B", "$" + (rawPalette[index] >> 10).ToString("X2"));
			} else if(format == RawPaletteFormat.Rgb333) {
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X3"));
				entries.AddEntry("R", "$" + ((rawPalette[index] >> 3) & 0x07).ToString("X2"));
				entries.AddEntry("G", "$" + (rawPalette[index] >> 6).ToString("X2"));
				entries.AddEntry("B", "$" + (rawPalette[index] & 0x07).ToString("X2"));
			} else {
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X2"));
			}
			entries.AddEntry("Color Code (Hex)", "#" + rgbPalette[index].ToString("X8").Substring(2));
			entries.AddEntry("Color Code (RGB)",
				((rgbPalette[index] >> 16) & 0xFF).ToString() + ", " +
				((rgbPalette[index] >> 8) & 0xFF).ToString() + ", " +
				(rgbPalette[index] & 0xFF).ToString()
			);

			entries.EndUpdate();
			if(tooltipToUpdate != null) {
				return tooltipToUpdate;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}
	}
}
