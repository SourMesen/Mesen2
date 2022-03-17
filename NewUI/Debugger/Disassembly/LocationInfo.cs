using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Interop;

namespace Mesen.Debugger.Disassembly
{
	public class LocationInfo
	{
		public AddressInfo? RelAddress;
		public AddressInfo? AbsAddress;
		public CodeLabel? Label;
		public SourceSymbol? Symbol;

		public int? ArrayIndex = null;
	}
}
