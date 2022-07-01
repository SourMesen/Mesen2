using Mesen.Interop;
using Mesen.Debugger.ViewModels;

namespace Mesen.Debugger.Disassembly
{
	public class SourceViewStyleProvider : BaseStyleProvider
	{
		private SourceViewViewModel _model;

		public SourceViewStyleProvider(CpuType cpuType, SourceViewViewModel model) : base(cpuType)
		{
			_model = model;
		}

		public override bool IsLineActive(CodeLineData line, int lineIndex)
		{
			return _model.ActiveAddress.HasValue && _model.ActiveAddress.Value == line.Address;
		}

		public override bool IsLineFocused(CodeLineData line, int lineIndex)
		{
			lineIndex += _model.ScrollPosition;
			return _model.SelectedRow == lineIndex;
		}

		public override bool IsLineSelected(CodeLineData line, int lineIndex)
		{
			lineIndex += _model.ScrollPosition;
			return lineIndex >= _model.SelectionStart && lineIndex <= _model.SelectionEnd;
		}
	}
}
