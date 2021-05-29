using Dock.Model.ReactiveUI.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class LabelEditViewModel : ViewModelBase
	{
		[Reactive] public CodeLabel Label { get; set; }
		[ObservableAsProperty] public bool OkEnabled { get; }
		[ObservableAsProperty] public string MaxAddress { get; } = "";

		//For designer
		public LabelEditViewModel() : this(new CodeLabel()) { }

		public LabelEditViewModel(CodeLabel label, CodeLabel? originalLabel = null)
		{
			Label = label;

			this.WhenAnyValue(x => x.Label.MemoryType, (memoryType) => {
				int maxAddress = DebugApi.GetMemorySize(memoryType) - 1;
				if(maxAddress <= 0) {
					return "(unavailable)";
				} else {
					return "(Max: $" + maxAddress.ToString("X4") + ")";
				}
			}).ToPropertyEx(this, x => x.MaxAddress);

			this.WhenAnyValue(x => x.Label.Label, x => x.Label.Comment, x => x.Label.Length, x => x.Label.MemoryType, x => x.Label.Address, (label, comment, length, memoryType, address) => {
				CodeLabel? sameLabel = LabelManager.GetLabel(label);
				int maxAddress = DebugApi.GetMemorySize(memoryType) - 1;

				for(UInt32 i = 0; i < length; i++) {
					CodeLabel? sameAddress = LabelManager.GetLabel(address + i, memoryType);
					if(sameAddress != null) {
						if(originalLabel == null) {
							//A label already exists and we're not editing an existing label, so we can't add it
							return false;
						} else {
							if(sameAddress.Label != originalLabel.Label && !sameAddress.Label.StartsWith(originalLabel.Label + "+")) {
								//A label already exists, we're trying to edit an existing label, but the existing label
								//and the label we're editing aren't the same label.  Can't override an existing label with a different one.
								return false;
							}
						}
					}
				}

				return
					length >= 1 && length <= 65536 &&
					address + (length - 1) <= maxAddress &&
					(sameLabel == null || sameLabel == originalLabel)
					&& (label.Length > 0 || comment.Length > 0)
					&& !comment.Contains('\x1')
					&& (label.Length == 0 || LabelManager.LabelRegex.IsMatch(label));
			}).ToPropertyEx(this, x => x.OkEnabled);
		}
	}
}
