using Mesen.Debugger.Controls;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Disassembly
{
	public class CodeDataProvider : ICodeDataProvider
	{
		private int _lineCount;
		protected CpuType _type;

		public CodeDataProvider(CpuType type)
		{
			_type = type;
			_lineCount = DebugApi.GetMemorySize(type.ToMemoryType());
		}

		public CpuType CpuType => _type;
		
		public int GetLineCount() => _lineCount;

		public CodeLineData[] GetCodeLines(int startPosition, int rowCount)
		{
			return DebugApi.GetDisassemblyOutput(_type, (uint)startPosition, (uint)rowCount);
		}

		public bool UseOptimizedSearch { get { return true; } }

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			return DebugApi.SearchDisassembly(_type, searchString, startPosition, endPosition, searchBackwards);
		}
	}
}
