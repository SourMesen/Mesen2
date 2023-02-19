using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;

namespace Mesen.ViewModels
{
	public class InputConfigViewModel : DisposableViewModel
	{
		[Reactive] public InputConfig Config { get; set; }
		[Reactive] public InputConfig OriginalConfig { get; set; }

		public InputConfigViewModel()
		{
			Config = ConfigManager.Config.Input;
			OriginalConfig = Config.Clone();

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}
	}
}
