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
		public static bool IsActionAllowed(MemoryType type)
		{
			return type switch {
				MemoryType.NesPpuMemory => true,
				MemoryType.NesChrRam => true,
				MemoryType.NesChrRom => true,
				MemoryType.NesPrgRom => true,
				_ => false,
			};
		}

		public static string ToString(AddressInfo tileAddr, int len)
		{
			StringBuilder sb = new StringBuilder();

			if(tileAddr.Type == MemoryType.NesPpuMemory) {
				tileAddr = DebugApi.GetAbsoluteAddress(tileAddr);
			}

			switch(tileAddr.Type) {
				case MemoryType.NesPrgRom:
				case MemoryType.NesChrRom:
				case MemoryType.NesChrRam:
					for(int i = 0; i < len; i++) {
						if(sb.Length > 0) {
							sb.Append(" ");
						}
						sb.Append(DebugApi.GetMemoryValue(tileAddr.Type, (uint)(tileAddr.Address + i)).ToString("X2"));
					}
					break;

				default:
					return "";
			}

			return sb.ToString();
		}

		public static void CopyTileMem(int address, MemoryType memoryType)
		{
			AddressInfo addr = new AddressInfo() { Address = address, Type = memoryType };
			string tileMem = MemCopyHelper.ToString(addr, 16);

			if(tileMem.Length > 0) {
				ApplicationHelper.GetMainWindow()?.Clipboard?.SetTextAsync(tileMem);
			}
		}

		public static void PasteTileMem(int address, MemoryType memoryType)
		{
			var clipboard = ApplicationHelper.GetMainWindow()?.Clipboard;
			if(clipboard != null) {
				string? text = Task.Run(() => clipboard?.GetTextAsync()).GetAwaiter().GetResult();
				if(text != null) {
					text = text.Replace("\n", "").Replace("\r", "").Replace(" ", "");
					if(Regex.IsMatch(text, "^[A-F0-9]{32}$", RegexOptions.IgnoreCase)) {
						byte[] pastedData = HexUtilities.HexToArray(text);
						for(int i = 0; i < 16; i++) {
							DebugApi.SetMemoryValue(memoryType, (uint)(address + i), (byte)pastedData[i]);
						}
					}
				}
			}
		}
	}
}
