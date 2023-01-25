using Avalonia.Controls;
using Mesen.Debugger.Controls;
using Mesen.Interop;

namespace Mesen.Debugger.Disassembly
{
	public class CodeDataProvider : ICodeDataProvider
	{
		private int _lineCount;
		protected CpuType _type;

		public CodeDataProvider(CpuType type)
		{
			_type = type;

			if(Design.IsDesignMode) {
				return;
			}

			_lineCount = DebugApi.GetMemorySize(type.ToMemoryType());
		}

		public CpuType CpuType => _type;
		
		public int GetLineCount() => _lineCount;

		public CodeLineData[] GetCodeLines(int startPosition, int rowCount)
		{
			return DebugApi.GetDisassemblyOutput(_type, (uint)startPosition, (uint)rowCount);
		}

		public int GetRowAddress(int address, int rowOffset)
		{
			return DebugApi.GetDisassemblyRowAddress(_type, (uint)address, rowOffset);
		}
	}
}
