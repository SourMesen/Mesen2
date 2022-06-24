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
		public Int32 AbsoluteAddress = -1;

		public LineFlags Flags;

		public int? CustomIndent = null;

		public byte OpSize = 0;
		public byte[] ByteCode = Array.Empty<byte>();
		public string ByteCodeStr = "";
		public string Comment = "";

		public Int32 EffectiveAddress = -1;
		public UInt16 Value = 0;
		public byte ValueSize = 0;

		public string GetEffectiveAddressString(string format, out CodeSegmentType segmentType)
		{
			if(EffectiveAddress >= 0) {
				AddressInfo relAddress = new AddressInfo() { Address = EffectiveAddress, Type = CpuType.ToMemoryType() };
				CodeLabel? label = LabelManager.GetLabel(relAddress);
				if(label != null) {
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
				return " = $" + Value.ToString("X2");
			} else if(ValueSize == 2) {
				return " = $" + Value.ToString("X4");
			} else {
				return "";
			}
		}

		public int Indentation
		{
			get
			{
				if(CustomIndent.HasValue) {
					return CustomIndent.Value;
				} else if(Flags.HasFlag(LineFlags.ShowAsData) || Flags.HasFlag(LineFlags.BlockStart) || Flags.HasFlag(LineFlags.BlockEnd) || Flags.HasFlag(LineFlags.Label) || (Flags.HasFlag(LineFlags.Comment) && Text.Length == 0)) {
					return 0;
				} else {
					return 15;
				}
			}
		}

		public bool HasAddress
		{
			get
			{
				return (
					Address >= 0 && 
					!Flags.HasFlag(LineFlags.Empty) && 
					!(Flags.HasFlag(LineFlags.Comment) && Text.Length == 0) && 
					!Flags.HasFlag(LineFlags.Label)
				);
			}
		}

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
			this.ByteCodeStr = "";
			for(int i = 0; i < this.OpSize; i++) {
				this.ByteCodeStr += data.ByteCode[i].ToString("X2") + " ";
			}
			this.ByteCode = data.ByteCode;

			this.Address = data.Address;
			this.AbsoluteAddress = data.AbsoluteAddress;
			this.EffectiveAddress = data.EffectiveAddress;
			this.Flags = (LineFlags)data.Flags;
			this.Value = data.Value;
			this.ValueSize = data.ValueSize;
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
	}
	
	public struct InteropCodeLineData
	{
		public Int32 Address;
		public Int32 AbsoluteAddress;
		public byte OpSize;
		public UInt16 Flags;

		public Int32 EffectiveAddress;
		public UInt16 Value;
		public byte ValueSize;
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
