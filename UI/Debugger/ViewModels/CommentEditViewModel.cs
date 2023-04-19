using Mesen.Debugger.Labels;
using Mesen.ViewModels;
using System;
using static Mesen.Debugger.ViewModels.LabelEditViewModel;

namespace Mesen.Debugger.ViewModels
{
	public class CommentEditViewModel : ViewModelBase
	{
		public ReactiveCodeLabel Label { get; set; }

		[Obsolete("For designer only")]
		public CommentEditViewModel() : this(new CodeLabel()) { }

		public CommentEditViewModel(CodeLabel label)
		{
			Label = new ReactiveCodeLabel(label);
		}
	}
}
