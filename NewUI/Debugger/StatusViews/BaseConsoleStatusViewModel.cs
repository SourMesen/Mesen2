using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Debugger.StatusViews
{
	public abstract class BaseConsoleStatusViewModel : ViewModelBase
	{
		[Reactive] public bool EditAllowed { get; set; }

		public abstract void UpdateUiState();
		public abstract void UpdateConsoleState();
	}
}
