using Dock.Model.ReactiveUI.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointEditViewModel : ViewModelBase
	{
		[Reactive] public Breakpoint Breakpoint { get; set; }
		
		[ObservableAsProperty] public bool StartValid { get; }
		[ObservableAsProperty] public bool EndValid { get; }

		[ObservableAsProperty] public bool IsConditionValid { get; }
		[ObservableAsProperty] public bool OkEnabled { get; }
		[ObservableAsProperty] public string MaxAddress { get; } = "";

		//For designer
		public BreakpointEditViewModel() : this(null!) { }

		public BreakpointEditViewModel(Breakpoint bp)
		{
			Breakpoint = bp;

			this.WhenAnyValue(x => x.Breakpoint.StartAddress)
				.Buffer(2, 1)
				.Select(b => (Previous: b[0], Current: b[1]))
				.Subscribe(t => {
					if(t.Previous == Breakpoint.EndAddress) {
						Breakpoint.EndAddress = t.Current;
					}
				});

			this.WhenAnyValue(x => x.Breakpoint.MemoryType, (memoryType) => {
				int maxAddress = DebugApi.GetMemorySize(memoryType) - 1;
				if(maxAddress <= 0) {
					return "(unavailable)";
				} else {
					return "(Max: $" + maxAddress.ToString("X4") + ")";
				}
			}).ToPropertyEx(this, x => x.MaxAddress);

			this.WhenAnyValue(x => x.Breakpoint.Condition).Select(condition => {
				if(!string.IsNullOrWhiteSpace(condition)) {
					EvalResultType resultType;
					DebugApi.EvaluateExpression(condition.Replace(Environment.NewLine, " "), Breakpoint.CpuType, out resultType, false);
					if(resultType == EvalResultType.Invalid) {
						return false;
					}
				}
				return true;
			}).ToPropertyEx(this, x => x.IsConditionValid);

			this.WhenAnyValue(
				x => x.Breakpoint.BreakOnExec,
				x => x.Breakpoint.BreakOnRead,
				x => x.Breakpoint.BreakOnWrite,
				x => x.Breakpoint.MemoryType,
				x => x.Breakpoint.StartAddress,
				x => x.Breakpoint.EndAddress,
				x => x.Breakpoint.Condition
			).Select(x => {
				if(Breakpoint.Type == BreakpointTypeFlags.None) {
					return false;
				}

				int maxAddress = DebugApi.GetMemorySize(Breakpoint.MemoryType) - 1;
				if(Breakpoint.StartAddress > maxAddress || Breakpoint.EndAddress > maxAddress || Breakpoint.StartAddress > Breakpoint.EndAddress) {
					return false;
				}

				if(!IsConditionValid) {
					return false;
				}

				return true;
			}).ToPropertyEx(this, x => x.OkEnabled);
		}
	}
}
