using Avalonia;
using Avalonia.Controls;
using Avalonia.VisualTree;
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
		[Reactive] public GameboyConfig OriginalConfig { get; set; }
		[Reactive] public GameboyConfigTab SelectedTab { get; set; } = 0;

		public ReactiveCommand<Button, Unit> SetupPlayer { get; }

		public GameboyConfigViewModel()
		{
			Config = ConfigManager.Config.Gameboy;
			OriginalConfig = Config.Clone();

			IObservable<bool> button1Enabled = this.WhenAnyValue(x => x.Config.Controller.Type, x => x.CanConfigure());
			SetupPlayer = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 0), button1Enabled);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}

		private async void OpenSetup(Button btn, int port)
		{
			PixelPoint startPosition = btn.PointToScreen(new Point(-7, btn.Bounds.Height));
			ControllerConfigWindow wnd = new ControllerConfigWindow();
			ControllerConfig cfg = Config.Controller.Clone();
			wnd.DataContext = new ControllerConfigViewModel(ControllerType.GameboyController, cfg, Config.Controller, 0);

			if(await wnd.ShowDialogAtPosition<bool>(btn.GetVisualRoot() as Visual, startPosition)) {
				Config.Controller = cfg;
			}
		}
	}

	public enum GameboyConfigTab
	{
		General,
		Audio,
		Emulation,
		Input,
		Video
	}
}
