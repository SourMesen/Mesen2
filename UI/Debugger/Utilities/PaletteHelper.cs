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
		public static DynamicTooltip GetPreviewPanel(UInt32[] rgbPalette, UInt32[] rawPalette, RawPaletteFormat format, int index, DynamicTooltip? tooltipToUpdate, int colorsPerPalette = 0)
		{
			TooltipEntries entries = tooltipToUpdate?.Items ?? new();
			entries.StartUpdate();

			entries.AddEntry("Color", new TooltipColorEntry(rgbPalette[index]));
			if(colorsPerPalette > 0) {
				entries.AddEntry("Index", "$" + (index % colorsPerPalette).ToString("X2"));
			} else {
				entries.AddEntry("Index", "$" + index.ToString("X2"));
			}

			if(format == RawPaletteFormat.Rgb555) {
				//SNES / GBC
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X4"));
				entries.AddEntry("R", "$" + (rawPalette[index] & 0x1F).ToString("X2"));
				entries.AddEntry("G", "$" + ((rawPalette[index] >> 5) & 0x1F).ToString("X2"));
				entries.AddEntry("B", "$" + ((rawPalette[index] >> 10) & 0x1F).ToString("X2"));
			} else if(format == RawPaletteFormat.Rgb333) {
				//PC Engine
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X3"));
				entries.AddEntry("R", "$" + ((rawPalette[index] >> 3) & 0x07).ToString("X2"));
				entries.AddEntry("G", "$" + (rawPalette[index] >> 6).ToString("X2"));
				entries.AddEntry("B", "$" + (rawPalette[index] & 0x07).ToString("X2"));
			} else if(format == RawPaletteFormat.Rgb222) {
				//SMS
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X2"));
				entries.AddEntry("R", "$" + (rawPalette[index] & 0x03).ToString());
				entries.AddEntry("G", "$" + ((rawPalette[index] >> 2) & 0x03).ToString());
				entries.AddEntry("B", "$" + ((rawPalette[index] >> 4) & 0x03).ToString());
			} else if(format == RawPaletteFormat.Rgb444) {
				//Game Gear
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X3"));
				entries.AddEntry("R", "$" + (rawPalette[index] & 0x0F).ToString());
				entries.AddEntry("G", "$" + ((rawPalette[index] >> 4) & 0x0F).ToString());
				entries.AddEntry("B", "$" + ((rawPalette[index] >> 8) & 0x0F).ToString());
			} else if(format == RawPaletteFormat.Bgr444) {
				//WonderSwan
				entries.AddEntry("Value", "$" + rawPalette[index].ToString("X3"));
				entries.AddEntry("R", "$" + ((rawPalette[index] >> 8) & 0x0F).ToString());
				entries.AddEntry("G", "$" + ((rawPalette[index] >> 4) & 0x0F).ToString());
				entries.AddEntry("B", "$" + (rawPalette[index] & 0x0F).ToString());
			} else {
				//NES/GB
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
