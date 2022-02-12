using Mesen.Debugger.Controls;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Disassembly
{
	public interface IDisassemblyManager
	{
		ICodeDataProvider Provider { get; }

		CpuType CpuType { get; }
		MemoryType RelativeMemoryType { get; }
		MemoryType PrgMemoryType { get; }
		int AddressSize { get; }
		int ByteCodeSize { get; }
		bool AllowSourceView { get; }

		void RefreshCode(ISymbolProvider symbolProvider, SourceFileInfo file);

		//Dictionary<string, string>? GetTooltipData(string word, int lineIndex);
		//LocationInfo GetLocationInfo(string lastWord, int lineIndex);
	}

	public class LocationInfo
	{
		public AddressInfo? RelAddress;
		public AddressInfo? AbsAddress;
		public CodeLabel? Label;
		public SourceSymbol? Symbol;

		public int? ArrayIndex = null;
	}
}
