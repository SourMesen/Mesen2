using Avalonia.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class LabelEditViewModel : DisposableViewModel
	{
		[Reactive] public ReactiveCodeLabel Label { get; set; }

		[ObservableAsProperty] public bool OkEnabled { get; }
		[ObservableAsProperty] public string MaxAddress { get; } = "";
		[Reactive] public string ErrorMessage { get; private set; } = "";
		
		public bool AllowDelete { get; } = false;
		
		public Enum[] AvailableMemoryTypes { get; private set; } = Array.Empty<Enum>();
		public CpuType CpuType { get; }

		private CodeLabel? _originalLabel;

		[Obsolete("For designer only")]
		public LabelEditViewModel() : this(CpuType.Snes, new CodeLabel()) { }

		public LabelEditViewModel(CpuType cpuType, CodeLabel label, CodeLabel? originalLabel = null)
		{
			_originalLabel = originalLabel;

			Label = new ReactiveCodeLabel(label);
			AllowDelete = originalLabel != null;

			if(Design.IsDesignMode) {
				return;
			}

			CpuType = cpuType;
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => cpuType.CanAccessMemoryType(t) && t.SupportsLabels() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Label.MemoryType)) {
				Label.MemoryType = (MemoryType)AvailableMemoryTypes[0];
			}

			AddDisposable(this.WhenAnyValue(x => x.Label.MemoryType, (memoryType) => {
				int maxAddress = DebugApi.GetMemorySize(memoryType) - 1;
				if(maxAddress <= 0) {
					return "(unavailable)";
				} else {
					return "(Max: $" + maxAddress.ToString("X4") + ")";
				}
			}).ToPropertyEx(this, x => x.MaxAddress));

			AddDisposable(this.WhenAnyValue(x => x.Label.Label, x => x.Label.Comment, x => x.Label.Length, x => x.Label.MemoryType, x => x.Label.Address, (label, comment, length, memoryType, address) => {
				CodeLabel? sameLabel = LabelManager.GetLabel(label);
				int maxAddress = DebugApi.GetMemorySize(memoryType) - 1;

				for(UInt32 i = 0; i < length; i++) {
					CodeLabel? sameAddress = LabelManager.GetLabel(address + i, memoryType);
					if(sameAddress != null) {
						if(originalLabel == null || (sameAddress.Label != originalLabel.Label && !sameAddress.Label.StartsWith(originalLabel.Label + "+"))) {
							//A label already exists, we're trying to edit an existing label, but the existing label
							//and the label we're editing aren't the same label.  Can't override an existing label with a different one.
							ErrorMessage = ResourceHelper.GetMessage("AddressHasOtherLabel", sameAddress.Label.Length > 0 ? sameAddress.Label : sameAddress.Comment);
							return false;
						}
					}
				}

				if(address + (length - 1) > maxAddress) {
					ErrorMessage = ResourceHelper.GetMessage("AddressOutOfRange");
					return false;
				}

				if(label.Length == 0 && comment.Length == 0) {
					ErrorMessage = ResourceHelper.GetMessage("LabelOrCommentRequired");
					return false;
				}

				if(label.Length > 0 && !LabelManager.LabelRegex.IsMatch(label)) {
					ErrorMessage = ResourceHelper.GetMessage("InvalidLabel");
					return false;
				}

				if(sameLabel != null && sameLabel != originalLabel) {
					ErrorMessage = ResourceHelper.GetMessage("LabelNameInUse");
					return false;
				}

				if(length >= 1 && length <= 65536 && !comment.Contains('\x1')) {
					ErrorMessage = "";
					return true;
				}

				return false;
			}).ToPropertyEx(this, x => x.OkEnabled));
		}

		public void DeleteLabel()
		{
			if(_originalLabel != null) {
				LabelManager.DeleteLabel(_originalLabel, true);
			}
		}

		public void Commit()
		{
			Label.Commit();
		}

		public class ReactiveCodeLabel : ReactiveObject
		{
			private CodeLabel _originalLabel;

			public ReactiveCodeLabel(CodeLabel label)
			{
				_originalLabel = label;

				Address = label.Address;
				Label = label.Label;
				Comment = label.Comment;
				MemoryType = label.MemoryType;
				Flags = label.Flags;
				Length = label.Length;
			}

			public void Commit()
			{
				_originalLabel.Address = Address;
				_originalLabel.Label = Label;
				_originalLabel.Comment = Comment;
				_originalLabel.MemoryType = MemoryType;
				_originalLabel.Flags = Flags;
				_originalLabel.Length = Length;
			}

			[Reactive] public UInt32 Address { get; set; }
			[Reactive] public MemoryType MemoryType { get; set; }
			[Reactive] public string Label { get; set; } = "";
			[Reactive] public string Comment { get; set; } = "";
			[Reactive] public CodeLabelFlags Flags { get; set; }
			[Reactive] public UInt32 Length { get; set; } = 1;
		}
	}
}
