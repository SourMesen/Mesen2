using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using Mesen.GUI.Debugger.Labels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Mesen.GUI.Debugger.Integration.DbgImporter;

namespace Mesen.GUI.Debugger.Code
{
	public interface IDisassemblyManager
	{
		ICodeDataProvider Provider { get; }

		CpuType CpuType { get; }
		SnesMemoryType RelativeMemoryType { get; }
		int AddressSize { get; }
		int ByteCodeSize { get; }
		bool AllowSourceView { get; }

		void RefreshCode(DbgImporter symbolProvider, DbgImporter.FileInfo file);

		Dictionary<string, string> GetTooltipData(string word, int lineIndex);
		LocationInfo GetLocationInfo(string lastWord, int lineIndex);
	}

	public class LocationInfo
	{
		public int Address;
		public CodeLabel Label;
		public SymbolInfo Symbol;

		public int? ArrayIndex = null;
	}
}
