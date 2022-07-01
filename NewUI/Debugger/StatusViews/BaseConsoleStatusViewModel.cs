using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.StatusViews
{
	public abstract class BaseConsoleStatusViewModel : ViewModelBase
	{
		[Reactive] public bool EditAllowed { get; set; }
		[Reactive] public UInt64 ElapsedCycles { get; set; }
		[Reactive] public UInt64 CycleCount { get; set; }

		public void UpdateCycleCount(UInt64 newCycleCount)
		{
			if(newCycleCount > CycleCount) {
				ElapsedCycles = newCycleCount - CycleCount;
			} else {
				ElapsedCycles = 0;
			}
			CycleCount = newCycleCount;
		}

		public abstract void UpdateUiState();
		public abstract void UpdateConsoleState();
	}
}
