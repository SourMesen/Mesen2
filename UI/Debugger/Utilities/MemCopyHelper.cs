using Avalonia;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Tmds.DBus;

namespace Mesen.Debugger.Utilities
{
	public static class MemCopyHelper
	{
		public static string ToString(AddressInfo tileAddr, int len)
		{
			StringBuilder sb = new StringBuilder();

			if(tileAddr.Type == MemoryType.NesPpuMemory) {
				tileAddr = DebugApi.GetAbsoluteAddress(tileAddr);
			}

			for(int i = 0; i < len; i++) {
				if(sb.Length > 0) {
					sb.Append(" ");
				}
				sb.Append(DebugApi.GetMemoryValue(tileAddr.Type, (uint)(tileAddr.Address + i)).ToString("X2"));
			}

			return sb.ToString();
		}

		public static int GetBytesPerTile(TileFormat format)
		{
			int bitsPerPixel = format.GetBitsPerPixel();
			PixelSize tileSize = format.GetTileSize();
			int bytesPerTile = tileSize.Width * tileSize.Height * bitsPerPixel / 8;

			return bytesPerTile;
		}

		public static void CopyTileMem(int address, MemoryType memoryType, TileFormat format)
		{
			int bytesPerTile = GetBytesPerTile(format);

			AddressInfo addr = new AddressInfo() { Address = address, Type = memoryType };
			string tileMem = MemCopyHelper.ToString(addr, bytesPerTile);

			if(tileMem.Length > 0) {
				ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(tileMem);
			}
		}

		public static void PasteTileMem(int address, MemoryType memoryType, TileFormat format)
		{
			var clipboard = ApplicationHelper.GetMainWindow()?.Clipboard;
			if(clipboard != null) {
				string? text = Task.Run(() => clipboard?.GetTextAsync()).GetAwaiter().GetResult();
				if(text != null) {
					text = text.Replace("\n", "").Replace("\r", "").Replace(" ", "");
					int bytesPerTile = GetBytesPerTile(format);
					int charsPerTile = bytesPerTile * 2;
					string pattern = $"^[A-F0-9]{{{charsPerTile}}}$";

					if(Regex.IsMatch(text, pattern, RegexOptions.IgnoreCase)) {
						byte[] pastedData = HexUtilities.HexToArray(text);
						for(int i = 0; i < bytesPerTile; i++) {
							DebugApi.SetMemoryValue(memoryType, (uint)(address + i), (byte)pastedData[i]);
						}
					}
				}
			}
		}
	}
}
