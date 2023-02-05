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

			sb.Append(MemoryType.ToString());
			sb.Append(":");

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

			if(!Enum.TryParse(rowData[0], out MemoryType type)) {
				if(!GetLegacyMemoryType(rowData[0], out type)) {
					//Invalid memory type
					return null;
				}
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
			return type.CanAccessMemoryType(MemoryType);
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

		private static bool GetLegacyMemoryType(string name, out MemoryType type)
		{
			//For Mesen v1 & Mesen-S compatibility
			type = MemoryType.SnesMemory;
			switch(name) {
				case "REG": type = MemoryType.SnesRegister; break;
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

				case "G": type = MemoryType.NesMemory; break;
				case "R": type = MemoryType.NesInternalRam; break;
				case "P": type = MemoryType.NesPrgRom; break;
				case "S": type = MemoryType.NesSaveRam; break;
				case "W": type = MemoryType.NesWorkRam; break;

				default: return false;
			}

			return true;
		}
	}
}
