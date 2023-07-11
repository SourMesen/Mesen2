using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.ComponentModel;

namespace Mesen.Debugger.StatusViews
{
	public abstract class BaseConsoleStatusViewModel : ViewModelBase
	{
		[Reactive] public bool EditAllowed { get; set; }
		[Reactive] public UInt64 ElapsedCycles { get; set; }
		[Reactive] public UInt64 CycleCount { get; set; }

		private bool _isUpdatingUi = false;
		private bool _needUpdate = false;

		public BaseConsoleStatusViewModel()
		{
			PropertyChanged += BaseConsoleStatusViewModel_PropertyChanged;
		}

		private void BaseConsoleStatusViewModel_PropertyChanged(object? sender, PropertyChangedEventArgs e)
		{
			if(!_isUpdatingUi) {
				switch(e.PropertyName) {
					case nameof(EditAllowed) or nameof(ElapsedCycles) or nameof(CycleCount):
						//Ignore these properties
						return;
				}

				_needUpdate = true;
			}
		}

		public void UpdateCycleCount(UInt64 newCycleCount)
		{
			if(newCycleCount > CycleCount) {
				ElapsedCycles = newCycleCount - CycleCount;
			} else {
				ElapsedCycles = 0;
			}
			CycleCount = newCycleCount;
		}

		public void UpdateUiState()
		{
			_isUpdatingUi = true;
			_needUpdate = false;
			InternalUpdateUiState();
			_isUpdatingUi = false;
			_needUpdate = false;
		}

		public void UpdateConsoleState()
		{
			if(_needUpdate && !_isUpdatingUi) {
				//Only update emulator state if user manually changed the state in the UI
				//Otherwise this causes issues when a state is loaded (e.g step back)
				InternalUpdateConsoleState();
			}
		}

		protected abstract void InternalUpdateUiState();
		protected abstract void InternalUpdateConsoleState();
	}
}
