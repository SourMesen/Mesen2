using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Globalization;
using System.Text;

namespace Mesen.Debugger.Labels
{
	public class CodeLabel
	{
		public UInt32 Address { get; set; }
		public MemoryType MemoryType { get; set; }
		public string Label { get; set; } = "";
		public string Comment { get; set; } = "";
		public CodeLabelFlags Flags { get; set; }
		public UInt32 Length { get; set; } = 1;

		public CodeLabel()
		{
		}

		public CodeLabel(AddressInfo absAddress)
		{
			Address = (uint)absAddress.Address;
			MemoryType = absAddress.Type;
		}

		public override string ToString()
		{
			StringBuilder sb = new StringBuilder();
			switch(MemoryType) {
				case MemoryType.Register: sb.Append("REG:"); break;
				case MemoryType.SnesPrgRom: sb.Append("PRG:"); break;
				case MemoryType.SnesWorkRam: sb.Append("WORK:"); break;
				case MemoryType.SnesSaveRam: sb.Append("SAVE:"); break;
				case MemoryType.Sa1InternalRam: sb.Append("IRAM:"); break;
				case MemoryType.SpcRam: sb.Append("SPCRAM:"); break;
				case MemoryType.SpcRom: sb.Append("SPCROM:"); break;
				case MemoryType.BsxPsRam: sb.Append("PSRAM:"); break;
				case MemoryType.BsxMemoryPack: sb.Append("MPACK:"); break;
				case MemoryType.DspProgramRom: sb.Append("DSPPRG:"); break;
				case MemoryType.GbPrgRom: sb.Append("GBPRG:"); break;
				case MemoryType.GbWorkRam: sb.Append("GBWRAM:"); break;
				case MemoryType.GbCartRam: sb.Append("GBSRAM:"); break;
				case MemoryType.GbHighRam: sb.Append("GBHRAM:"); break;
				case MemoryType.GbBootRom: sb.Append("GBBOOT:"); break;
				case MemoryType.GameboyMemory: sb.Append("GBREG:"); break;
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

		private static char[] _separator = new char[1] { ':' };
		public static CodeLabel? FromString(string data)
		{
			string[] rowData = data.Split(_separator, 4);
			if(rowData.Length < 3) {
				//Invalid row
				return null;
			}

			MemoryType type;
			switch(rowData[0]) {
				case "REG": type = MemoryType.Register; break;
				case "PRG": type = MemoryType.SnesPrgRom; break;
				case "SAVE": type = MemoryType.SnesSaveRam; break;
				case "WORK": type = MemoryType.SnesWorkRam; break;
				case "IRAM": type = MemoryType.Sa1InternalRam; break;
				case "SPCRAM": type = MemoryType.SpcRam; break;
				case "SPCROM": type = MemoryType.SpcRom; break;
				case "PSRAM": type = MemoryType.BsxPsRam; break;
				case "MPACK": type = MemoryType.BsxMemoryPack; break;
				case "DSPPRG": type = MemoryType.DspProgramRom; break;
				case "GBPRG": type = MemoryType.GbPrgRom; break;
				case "GBWRAM": type = MemoryType.GbWorkRam; break;
				case "GBSRAM": type = MemoryType.GbCartRam; break;
				case "GBHRAM": type = MemoryType.GbHighRam; break;
				case "GBBOOT": type = MemoryType.GbBootRom; break;
				case "GBREG": type = MemoryType.GameboyMemory; break;
				default: return null;
			}

			string addressString = rowData[1];
			uint address = 0;
			uint length = 1;
			if(addressString.Contains("-")) {
				uint addressEnd;
				string[] addressStartEnd = addressString.Split('-');
				if(UInt32.TryParse(addressStartEnd[0], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address) &&
					UInt32.TryParse(addressStartEnd[1], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out addressEnd)) {
					if(addressEnd > address) {
						length = addressEnd - address + 1;
					} else {
						//Invalid label (start < end)
						return null;
					}
				} else {
					//Invalid label (can't parse)
					return null;
				}
			} else {
				if(!UInt32.TryParse(rowData[1], NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address)) {
					//Invalid label (can't parse)
					return null;
				}
				length = 1;
			}

			string labelName = rowData[2];
			if(!string.IsNullOrEmpty(labelName) && !LabelManager.LabelRegex.IsMatch(labelName)) {
				//Reject labels that don't respect the label naming restrictions
				return null;
			}

			CodeLabel codeLabel;
			codeLabel = new CodeLabel {
				Address = address,
				MemoryType = type,
				Label = labelName,
				Length = length,
				Comment = ""
			};

			if(rowData.Length > 3) {
				codeLabel.Comment = rowData[3].Replace("\\n", "\n");
			}

			return codeLabel;
		}

		public bool Matches(CpuType type)
		{
			return type == this.MemoryType.ToCpuType();
		}

		public AddressInfo GetAbsoluteAddress()
		{
			return new AddressInfo() { Address = (int)this.Address, Type = this.MemoryType };
		}

		public AddressInfo GetRelativeAddress(CpuType cpuType)
		{
			if(MemoryType.IsRelativeMemory()) {
				return GetAbsoluteAddress();
			}
			return DebugApi.GetRelativeAddress(GetAbsoluteAddress(), cpuType);
		}

		public byte GetValue()
		{
			return DebugApi.GetMemoryValue(this.MemoryType, this.Address);
		}

		public CodeLabel Clone()
		{
			return JsonHelper.Clone(this);
		}

		public void CopyFrom(CodeLabel copy)
		{
			Address = copy.Address;
			MemoryType = copy.MemoryType;
			Label = copy.Label;
			Comment = copy.Comment;
			Flags = copy.Flags;
			Length = copy.Length;
		}
	}
}
