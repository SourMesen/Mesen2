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
		public int? LabelAddressOffset;
		public SourceSymbol? Symbol;
		public SourceCodeLocation? SourceLocation;

		public int? ArrayIndex = null;
	}
}
