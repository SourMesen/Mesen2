using Mesen.GUI.Debugger.Labels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger
{
	public class CodeLineData
	{
		private CpuType _cpuType;

		public string Text;

		public Int32 Address;
		public Int32 AbsoluteAddress;

		public LineFlags Flags;

		public int? CustomIndent = null;

		public byte OpSize;
		public string ByteCode;
		public string Comment;

		public Int32 EffectiveAddress;
		public UInt16 Value;
		public byte ValueSize;

		public string GetEffectiveAddressString(string format)
		{
			if(EffectiveAddress >= 0) {
				AddressInfo relAddress = new AddressInfo() { Address = EffectiveAddress, Type = _cpuType == CpuType.Spc ? SnesMemoryType.SpcMemory : SnesMemoryType.CpuMemory };
				CodeLabel label = LabelManager.GetLabel(relAddress);
				if(label != null) {
					if(label.Length > 1) {
						int gap = DebugApi.GetAbsoluteAddress(relAddress).Address - label.GetAbsoluteAddress().Address;
						if(gap > 0) {
							return "[" + label.Label + "+" + gap.ToString() + "]";
						}
					}
					return "[" + label.Label + "]";
				} else {
					return "[" + EffectiveAddress.ToString(format) + "]";
				}
			} else {
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
				} else if(Flags.HasFlag(LineFlags.BlockStart) || Flags.HasFlag(LineFlags.BlockEnd) || Flags.HasFlag(LineFlags.Label) || (Flags.HasFlag(LineFlags.Comment) && Text.Length == 0)) {
					return 0;
				} else {
					return 15;
				}
			}
		}

		public CodeLineData(CpuType cpuType)
		{
			_cpuType = cpuType;
		}

		public CodeLineData(InteropCodeLineData data, CpuType cpuType)
		{
			_cpuType = cpuType;

			this.Text = ConvertString(data.Text);
			this.Comment = ConvertString(data.Comment);
			this.OpSize = data.OpSize;
			for(int i = 0; i < this.OpSize; i++) {
				this.ByteCode += "$" + data.ByteCode[i].ToString("X2") + " ";
			}

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

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
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
	}
}
