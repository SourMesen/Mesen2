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
			}

			return "";
		}
	}
}
