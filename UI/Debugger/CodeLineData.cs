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
		public string Text;

		public Int32 Address;
		public Int32 AbsoluteAddress;

		public LineFlags Flags;

		public byte OpSize;
		public string ByteCode;
		public string Comment;

		public Int32 EffectiveAddress;
		public UInt16 Value;
		public byte ValueSize;

		public string GetEffectiveAddressString(string format)
		{
			if(EffectiveAddress >= 0) {
				return "[" + EffectiveAddress.ToString(format) + "]";
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
				if(Flags.HasFlag(LineFlags.BlockStart) || Flags.HasFlag(LineFlags.BlockEnd)) {
					return 0;
				} else {
					return 10;
				}
			}
		}

		public CodeLineData()
		{
		}

		public CodeLineData(InteropCodeLineData data)
		{
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
		public byte Flags;

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

	public enum LineFlags : byte
	{
		PrgRom = 1,
		WorkRam = 2,
		SaveRam = 4,
		VerifiedData = 8,
		VerifiedCode = 16,
		BlockStart = 32,
		BlockEnd = 64,
	}
}
