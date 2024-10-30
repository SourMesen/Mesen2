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
using Avalonia.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Localization;
using Mesen.Debugger.Windows;

namespace Mesen.Debugger.ViewModels
{
	public class BreakpointEditViewModel : DisposableViewModel
	{
		[Reactive] public Breakpoint Breakpoint { get; set; }

		public Control? HelpTooltip { get; } = null;
		public string WindowTitle { get; } = "";
		[Reactive] public bool IsConditionValid { get; private set; }
		[Reactive] public bool OkEnabled { get; private set; }
		[Reactive] public string MaxAddress { get; private set; } = "";
		[Reactive] public bool CanExec { get; private set; } = false;
		[Reactive] public bool HasDummyOperations { get; private set; } = false;

		public Enum[] AvailableMemoryTypes { get; private set; } = Array.Empty<Enum>();

		[Obsolete("For designer only")]
		public BreakpointEditViewModel() : this(null!) { }

		public BreakpointEditViewModel(Breakpoint bp)
		{
			Breakpoint = bp;

			if(Design.IsDesignMode) {
				return;
			}

			WindowTitle = ResourceHelper.GetViewLabel(nameof(BreakpointEditWindow), bp.Forbid ? "wndTitleForbid" : "wndTitle");

			HasDummyOperations = bp.CpuType.HasDummyOperations() && !bp.Forbid;
			HelpTooltip = ExpressionTooltipHelper.GetHelpTooltip(bp.CpuType, false);
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => {
				if(bp.Forbid && !t.SupportsExecBreakpoints()) {
					return false;
				}
				return bp.CpuType.CanAccessMemoryType(t) && t.SupportsBreakpoints() && DebugApi.GetMemorySize(t) > 0;
			}).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Breakpoint.MemoryType)) {
				Breakpoint.MemoryType = (MemoryType)AvailableMemoryTypes[0];
			}

			AddDisposable(this.WhenAnyValue(x => x.Breakpoint.StartAddress)
				.Buffer(2, 1)
				.Select(b => (Previous: b[0], Current: b[1]))
				.Subscribe(t => {
					if(t.Previous == Breakpoint.EndAddress) {
						Breakpoint.EndAddress = t.Current;
					}
				}
			));

			AddDisposable(this.WhenAnyValue(x => x.Breakpoint.MemoryType).Subscribe(memoryType => {
				CanExec = memoryType.SupportsExecBreakpoints();
			}));

			AddDisposable(this.WhenAnyValue(x => x.Breakpoint.MemoryType).Subscribe(memoryType => {
				int maxAddress = DebugApi.GetMemorySize(memoryType) - 1;
				if(maxAddress <= 0) {
					MaxAddress = "(unavailable)";
				} else {
					MaxAddress = "(Max: $" + maxAddress.ToString("X4") + ")";
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.Breakpoint.Condition).Subscribe(condition => {
				if(!string.IsNullOrWhiteSpace(condition)) {
					EvalResultType resultType;
					DebugApi.EvaluateExpression(condition.Replace(Environment.NewLine, " "), Breakpoint.CpuType, out resultType, false);
					if(resultType == EvalResultType.Invalid) {
						IsConditionValid = false;
						return;
					}
				}
				IsConditionValid = true;
			}));

			AddDisposable(this.WhenAnyValue(
				x => x.Breakpoint.BreakOnExec,
				x => x.Breakpoint.BreakOnRead,
				x => x.Breakpoint.BreakOnWrite,
				x => x.Breakpoint.MemoryType,
				x => x.Breakpoint.StartAddress,
				x => x.Breakpoint.EndAddress,
				x => x.IsConditionValid
			).Subscribe(_ => {
				bool enabled = true;
				if(Breakpoint.Type == BreakpointTypeFlags.None || !IsConditionValid) {
					enabled = false;
				} else {
					int maxAddress = DebugApi.GetMemorySize(Breakpoint.MemoryType) - 1;
					if(Breakpoint.StartAddress > maxAddress || Breakpoint.EndAddress > maxAddress || Breakpoint.StartAddress > Breakpoint.EndAddress) {
						enabled = false;
					}
				}
				OkEnabled = enabled;
			}));
		}
	}
}
