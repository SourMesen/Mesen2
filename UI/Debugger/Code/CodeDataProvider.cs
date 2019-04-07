using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class CodeDataProvider : ICodeDataProvider
	{
		private int _lineCount;
		protected CpuType _type;

		public CodeDataProvider(CpuType type)
		{
			_type = type;
			_lineCount = (int)DebugApi.GetDisassemblyLineCount(_type);
		}

		public CodeLineData GetCodeLineData(int lineIndex)
		{
			return DebugApi.GetDisassemblyLineData(_type, (UInt32)lineIndex);
		}

		public int GetLineAddress(int lineIndex)
		{
			return DebugApi.GetDisassemblyLineData(_type, (UInt32)lineIndex).Address;
		}

		public int GetLineCount()
		{
			return _lineCount;
		}

		public int GetLineIndex(uint cpuAddress)
		{
			return (int)DebugApi.GetDisassemblyLineIndex(_type, cpuAddress);
		}

		public bool UseOptimizedSearch { get { return true; } }

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			return DebugApi.SearchDisassembly(_type, searchString, startPosition, endPosition, searchBackwards);
		}
	}
}
