using Avalonia;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using Tmds.DBus;

namespace Mesen.Debugger.Utilities
{
	public static class HdPackCopyHelper
	{
		public static bool IsActionAllowed(MemoryType type)
		{
			return type switch {
				MemoryType.NesPpuMemory => true,
				MemoryType.NesChrRam => true,
				MemoryType.NesChrRom => true,
				_ => false,
			};
		}

		public static string ToHdPackFormat(AddressInfo tileAddr, UInt32[] rawPalette, int paletteIndex, bool forSprite)
		{
			StringBuilder sb = new StringBuilder();

			if(tileAddr.Type == MemoryType.NesPpuMemory) {
				tileAddr = DebugApi.GetAbsoluteAddress(tileAddr);
			}

			switch(tileAddr.Type) {
				case MemoryType.NesChrRom: sb.Append((tileAddr.Address / 16).ToString("X2")); break;
				case MemoryType.NesChrRam:
					for(int i = 0; i < 16; i++) {
						sb.Append(DebugApi.GetMemoryValue(MemoryType.NesChrRam, (uint)(tileAddr.Address + i)).ToString("X2"));
					}
					break;

				default:
					return "";
			}

			if(forSprite) {
				sb.Append(",FF");
				for(int i = 1; i < 4; i++) {
					sb.Append(rawPalette[(paletteIndex + 4) * 4 + i].ToString("X2"));
				}
			} else {
				sb.Append(",");
				sb.Append(rawPalette[0].ToString("X2")); //Always use color 0 for palette index 0
				for(int i = 1; i < 4; i++) {
					sb.Append(rawPalette[paletteIndex * 4 + i].ToString("X2"));
				}
			}

			return sb.ToString();
		}

		public static void CopyToHdPackFormat(int address, MemoryType memoryType, UInt32[] palette, int paletteIndex, bool forSprite, bool isLargeSprite = false)
		{
			AddressInfo addr = new AddressInfo() { Address = address, Type = memoryType };
			string hdPackTile = HdPackCopyHelper.ToHdPackFormat(addr, palette, paletteIndex, forSprite);
			
			if(isLargeSprite && hdPackTile.Length > 0) {
				//Also copy the bottom tile's information to the clipboard
				addr.Address += 16;
				hdPackTile += Environment.NewLine + HdPackCopyHelper.ToHdPackFormat(addr, palette, paletteIndex, forSprite);
			}

			if(hdPackTile.Length > 0) {
				ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(hdPackTile);
			}
		}
	}
}
