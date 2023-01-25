using Mesen.Interop;
using Mesen.Debugger.ViewModels;

namespace Mesen.Debugger.Disassembly
{
	public class DisassemblyViewStyleProvider : BaseStyleProvider
	{
		private DisassemblyViewModel _model;

		public DisassemblyViewStyleProvider(CpuType cpuType, DisassemblyViewModel model) : base(cpuType)
		{
			_model = model;
		}

		public override bool IsLineActive(CodeLineData line, int lineIndex)
		{
			return line.HasAddress && _model.ActiveAddress.HasValue && _model.ActiveAddress.Value == line.Address;
		}

		public override bool IsLineFocused(CodeLineData line, int lineIndex)
		{
			return line.HasAddress && _model.SelectedRowAddress == line.Address;
		}

		public override bool IsLineSelected(CodeLineData line, int lineIndex)
		{
			return line.HasAddress && line.Address >= _model.SelectionStart && line.Address <= _model.SelectionEnd;
		}
	}
}
