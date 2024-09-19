using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger
{
	public class CodeLineData
	{
		public CpuType CpuType { get; private set; }

		public string Text = "";

		public Int32 Address = -1;
		public AddressInfo AbsoluteAddress;

		public LineFlags Flags;

		public byte OpSize = 0;
		public byte[] ByteCode = Array.Empty<byte>();

		private string _byteCodeString = "";
		public string ByteCodeStr 
		{
			get
			{
				if(OpSize > 0 && _byteCodeString == "") {
					string output = "";
					for(int i = 0; i < OpSize && i < ByteCode.Length; i++) {
						output += ByteCode[i].ToString("X2") + " ";
					}
					_byteCodeString = output;
				}

				return _byteCodeString;
			}
		}

		public string Comment = "";

		public bool ShowEffectiveAddress = false;
		public Int64 EffectiveAddress = -1;
		public MemoryType EffectiveAddressType = MemoryType.None;
		public UInt32 Value = 0;
		public byte ValueSize = 0;

		public string GetEffectiveAddressString(string format, out CodeSegmentType segmentType)
		{
			if(ShowEffectiveAddress && EffectiveAddress >= 0) {
				AddressInfo relAddress = new() { Address = (Int32)EffectiveAddress, Type = EffectiveAddressType };
				CodeLabel? label = LabelManager.GetLabel(relAddress);
				if(label != null && !string.IsNullOrWhiteSpace(label.Label)) {
					segmentType = CodeSegmentType.Label;
					if(label.Length > 1) {
						int gap = DebugApi.GetAbsoluteAddress(relAddress).Address - label.GetAbsoluteAddress().Address;
						if(gap > 0) {
							return "[" + label.Label + "+" + gap.ToString() + "]";
						}
					}
					return "[" + label.Label + "]";
				} else {
					segmentType = CodeSegmentType.EffectiveAddress;
					if(EffectiveAddressType.IsUnmapped()) {
						//Default to a single byte for I/O ports (SMS/WS)
						format = ConfigManager.Config.Debug.Debugger.UseLowerCaseDisassembly ? "x2" : "X2";
					}
					return "[$" + EffectiveAddress.ToString(format) + "]";
				}
			} else {
				segmentType = CodeSegmentType.None;
				return "";
			}
		}

		public string GetValueString()
		{
			if(ValueSize == 1) {
				return " = $" + Value.ToString(ConfigManager.Config.Debug.Debugger.UseLowerCaseDisassembly ? "x2" : "X2");
			} else if(ValueSize == 2) {
				return " = $" + Value.ToString(ConfigManager.Config.Debug.Debugger.UseLowerCaseDisassembly ? "x4" : "X4");
			} else if(ValueSize == 4) {
				return " = $" + Value.ToString(ConfigManager.Config.Debug.Debugger.UseLowerCaseDisassembly ? "x8" : "X8");
			} else {
				return "";
			}
		}

		public bool IsAddressHidden
		{
			get
			{
				return (
					Flags.HasFlag(LineFlags.Empty) || //block start/end, etc.
					(Flags.HasFlag(LineFlags.Comment) && Text.Length == 0) || //multi-line comment
					Flags.HasFlag(LineFlags.Label) //label definition
				);
			}
		}

		public bool HasAddress => Address >= 0 && !IsAddressHidden;

		public CodeLineData(CpuType cpuType)
		{
			CpuType = cpuType;
		}

		public CodeLineData(InteropCodeLineData data)
		{
			this.CpuType = data.LineCpuType;

			this.Text = ConvertString(data.Text);
			this.Comment = ConvertString(data.Comment);
			this.OpSize = data.OpSize;
			this.ByteCode = data.ByteCode;

			this.Address = data.Address;
			this.AbsoluteAddress = data.AbsoluteAddress;
			this.ShowEffectiveAddress = data.EffectiveAddress.ShowAddress;
			this.EffectiveAddress = data.EffectiveAddress.Address;
			this.EffectiveAddressType = data.EffectiveAddress.Type != MemoryType.None ? data.EffectiveAddress.Type : CpuType.ToMemoryType();
			this.Flags = (LineFlags)data.Flags;
			this.Value = data.Value;
			this.ValueSize = data.EffectiveAddress.ValueSize;
		}

		private string ConvertString(byte[] stringArray)
		{
			int length = 0;
			for(int i = 0; i < 1000; i++) {
				if(stringArray[i] == 0) {
					length = i;
					break;
				}
			}
			return Encoding.UTF8.GetString(stringArray, 0, length);
		}

		public override string ToString()
		{
			return "$" + this.Address.ToString("X6") + "  " + this.ByteCodeStr?.PadRight(12) + "  " + this.Text;
		}

		public string GetByteCode(int byteCodeSize)
		{
			if(ByteCodeStr.Length > byteCodeSize * 3) {
				return ByteCodeStr.Substring(0, (byteCodeSize - 1) * 3) + "..";
			}
			return ByteCodeStr;
		}

		public string GetAddressText(AddressDisplayType addressDisplayType, string addrFormat)
		{
			string addressText = HasAddress ? Address.ToString(addrFormat) : "";
			string absAddress = AbsoluteAddress.Address >= 0 && !IsAddressHidden ? AbsoluteAddress.Address.ToString(addrFormat) : "";
			string compactAbsAddress = AbsoluteAddress.Address >= 0 && !IsAddressHidden ? (AbsoluteAddress.Address >> 12).ToString("X") : "";
			return addressDisplayType switch {
				AddressDisplayType.CpuAddress => addressText,
				AddressDisplayType.AbsAddress => absAddress,
				AddressDisplayType.Both => (addressText + (string.IsNullOrEmpty(absAddress) ? "" : " [" + absAddress + "]")).Trim(),
				AddressDisplayType.BothCompact => (addressText + (string.IsNullOrEmpty(compactAbsAddress) ? "" : " [" + compactAbsAddress + "]")).Trim(),
				_ => throw new NotImplementedException()
			};
		}
	}

	public struct EffectiveAddressInfo
	{
		public Int64 Address;
		public MemoryType Type;
		public byte ValueSize;
		[MarshalAs(UnmanagedType.I1)] public bool ShowAddress;
	}

	public struct InteropCodeLineData
	{
		public Int32 Address;
		public AddressInfo AbsoluteAddress;
		public byte OpSize;
		public UInt16 Flags;

		public EffectiveAddressInfo EffectiveAddress;
		public UInt32 Value;
		public CpuType LineCpuType;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
		public byte[] ByteCode;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
		public byte[] Text;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
		public byte[] Comment;
	}

	public enum LineFlags : UInt16
	{
		None = 0,
		PrgRom = 0x01,
		WorkRam = 0x02,
		SaveRam = 0x04,
		VerifiedData = 0x08,
		VerifiedCode = 0x10,
		BlockStart = 0x20,
		BlockEnd = 0x40,
		SubStart = 0x80,
		Label = 0x100,
		Comment = 0x200,
		ShowAsData = 0x400,
		UnexecutedCode = 0x800,
		UnmappedMemory = 0x1000,
		Empty = 0x2000
	}
}
