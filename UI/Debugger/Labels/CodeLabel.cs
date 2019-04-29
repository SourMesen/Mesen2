using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Labels
{
	public class CodeLabel
	{
		public UInt32 Address;
		public SnesMemoryType MemoryType;
		public string Label;
		public string Comment;
		public CodeLabelFlags Flags;
		public UInt32 Length = 1;

		public override string ToString()
		{
			StringBuilder sb = new StringBuilder();
			switch(MemoryType) {
				case SnesMemoryType.PrgRom: sb.Append("P:"); break;
				case SnesMemoryType.WorkRam: sb.Append("W:"); break;
				case SnesMemoryType.SaveRam: sb.Append("S:"); break;
				case SnesMemoryType.Register: sb.Append("R:"); break;
			}

			sb.Append(Address.ToString("X4"));
			if(Length > 1) {
				sb.Append("-" + (Address+Length-1).ToString("X4"));
			}
			sb.Append(":");
			sb.Append(Label);
			if(!string.IsNullOrWhiteSpace(Comment)) {
				sb.Append(":");
				sb.Append(Comment.Replace(Environment.NewLine, "\\n").Replace("\n", "\\n").Replace("\r", "\\n"));
			}
			return sb.ToString();
		}

		public AddressInfo GetAbsoluteAddress()
		{
			return new AddressInfo() { Address = (int)this.Address, Type = this.MemoryType };
		}

		public AddressInfo GetRelativeAddress()
		{
			return DebugApi.GetRelativeAddress(GetAbsoluteAddress());
		}

		public byte GetValue()
		{
			return DebugApi.GetMemoryValue(this.MemoryType, this.Address);
		}

		public CodeLabel Clone()
		{
			return (CodeLabel)this.MemberwiseClone();
		}
	}
}
