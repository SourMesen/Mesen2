namespace Mesen.Interop
{
	public static class DebugEventInfoExtensions
	{
		public static string GetRegisterName(this DebugEventInfo evt)
		{
			if(evt.RegisterId < 0) {
				return "";
			}

			switch(evt.Operation.MemType.ToCpuType()) {
				case CpuType.Pce:
					switch(evt.RegisterId) {
						case 0: return "MAWR";
						case 1: return "MARR";
						case 2: return evt.Operation.Type.IsRead() ? "VRR" : "VWR";
						case 5: return "CR";
						case 6: return "RCR";
						case 7: return "BXR";
						case 8: return "BYR";
						case 9: return "MWR";
						case 0xA: return "HSR";
						case 0xB: return "HDR";
						case 0xC: return "VPR";
						case 0xD: return "VDW";
						case 0xE: return "VCR";
						case 0xF: return "DCR";
						case 0x10: return "SOUR";
						case 0x11: return "DESR";
						case 0x12: return "LENR";
						case 0x13: return "DVSSR";
					}
					break;

				case CpuType.Sms:
					byte codeReg = (byte)(evt.Operation.Value >> 6);
					byte regNumber = (byte)(evt.Operation.Value & 0x0F);
					if(codeReg == 2) {
						switch(regNumber) {
							case 0: return "Mode1";
							case 1: return "Mode2";
							case 2: return "NtAddress";
							case 3: return "CtAddress";
							case 4: return "PgtAddress";
							case 5: return "SatAddress";
							case 6: return "SpgtAddress";
							case 7: return "BgColor";
							case 8: return "BgXScroll";
							case 9: return "BgYScroll";
							case 10: return "LineCounter";
						}
					}
					break;
			}

			return "";
		}
	}
}
