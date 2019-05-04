using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public interface IDisassemblyManager
	{
		ICodeDataProvider Provider { get; }

		int AddressSize { get; }
		int ByteCodeSize { get; }
		bool AllowSourceView { get; }

		void RefreshCode(DbgImporter symbolProvider, DbgImporter.FileInfo file);
		void ToggleBreakpoint(int lineIndex);
		void EnableDisableBreakpoint(int lineIndex);

		Dictionary<string, string> GetTooltipData(string word, int lineIndex);
	}
}
