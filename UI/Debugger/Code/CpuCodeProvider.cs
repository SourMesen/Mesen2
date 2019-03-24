using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class CpuCodeDataProvider : ICodeDataProvider
	{
		private int _lineCount;

		public CpuCodeDataProvider()
		{
			_lineCount = (int)DebugApi.GetDisassemblyLineCount();
		}

		public CodeLineData GetCodeLineData(int lineIndex)
		{
			return DebugApi.GetDisassemblyLineData((UInt32)lineIndex);
		}

		public int GetLineAddress(int lineIndex)
		{
			return DebugApi.GetDisassemblyLineData((UInt32)lineIndex).Address;
		}

		public int GetLineCount()
		{
			return _lineCount;
		}

		public int GetLineIndex(uint cpuAddress)
		{
			return (int)DebugApi.GetDisassemblyLineIndex(cpuAddress);
		}

		public bool UseOptimizedSearch { get { return true; } }

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			return DebugApi.SearchDisassembly(searchString, startPosition, endPosition, searchBackwards);
		}
	}
}
