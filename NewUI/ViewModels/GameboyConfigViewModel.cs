using Avalonia;
using Avalonia.Controls;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;

namespace Mesen.ViewModels
{
	public class GameboyConfigViewModel : DisposableViewModel
	{
		[Reactive] public GameboyConfig Config { get; set; }
		public ReactiveCommand<Button, Unit> SetupPlayer { get; }

		public GameboyConfigViewModel()
		{
			Config = ConfigManager.Config.Gameboy.Clone();
			
			IObservable<bool> button1Enabled = this.WhenAnyValue(x => x.Config.Controller.Type, x => x.CanConfigure());
			SetupPlayer = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 0), button1Enabled);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}

		private async void OpenSetup(Button btn, int port)
		{
			PixelPoint startPosition = btn.PointToScreen(new Point(-7, btn.Height));
			ControllerConfigWindow wnd = new ControllerConfigWindow();
			wnd.WindowStartupLocation = WindowStartupLocation.Manual;
			wnd.Position = startPosition;

			ControllerConfig cfg = JsonHelper.Clone(this.Config.Controller);
			wnd.DataContext = new ControllerConfigViewModel(cfg);

			if(await wnd.ShowDialog<bool>(btn.Parent?.VisualRoot as Window)) {
				Config.Controller = cfg;
			}
		}
	}
}
