using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.ViewModels
{
	public abstract class BaseConsoleStatusViewModel : ViewModelBase
	{
		[Reactive] public bool EditAllowed { get; set; }

		public abstract void UpdateUiState();
		public abstract void UpdateEmulationState();
	}
}
