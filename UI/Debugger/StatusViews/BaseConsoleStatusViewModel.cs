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

		private UInt64 _lastBreakCycleCount = 0;
		private bool _isRunning = false;

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
			CycleCount = newCycleCount;

			if(_isRunning) {
				//Don't reset elapsed value if refresh done because "refresh while running" is enabled
				return;
			}

			if(newCycleCount > _lastBreakCycleCount) {
				ElapsedCycles = newCycleCount - _lastBreakCycleCount;
			} else {
				ElapsedCycles = 0;
			}

			_lastBreakCycleCount = newCycleCount;
		}

		public void UpdateUiState(bool isRunning = false)
		{
			_isUpdatingUi = true;
			_needUpdate = false;
			_isRunning = isRunning;
			InternalUpdateUiState();
			_isRunning = false;
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
