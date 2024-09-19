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
	public class WsConfigViewModel : DisposableViewModel
	{
		[Reactive] public WsConfig Config { get; set; }
		[Reactive] public WsConfig OriginalConfig { get; set; }
		[Reactive] public WsConfigTab SelectedTab { get; set; } = 0;

		public ReactiveCommand<Button, Unit> SetupPlayerHorizontal { get; }
		public ReactiveCommand<Button, Unit> SetupPlayerVertical { get; }

		public WsConfigViewModel()
		{
			Config = ConfigManager.Config.Ws;
			OriginalConfig = Config.Clone();

			IObservable<bool> button1Enabled = this.WhenAnyValue(x => x.Config.ControllerHorizontal.Type, x => x.CanConfigure());
			IObservable<bool> button2Enabled = this.WhenAnyValue(x => x.Config.ControllerVertical.Type, x => x.CanConfigure());
			SetupPlayerHorizontal = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 0), button1Enabled);
			SetupPlayerVertical = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 1), button2Enabled);

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => { Config.ApplyConfig(); }));
		}

		private async void OpenSetup(Button btn, int port)
		{
			PixelPoint startPosition = btn.PointToScreen(new Point(-7, btn.Bounds.Height));
			ControllerConfigWindow wnd = new ControllerConfigWindow();
			ControllerConfig orgCfg = port == 0 ? Config.ControllerHorizontal : Config.ControllerVertical;
			ControllerConfig cfg = port == 0 ? Config.ControllerHorizontal.Clone() : Config.ControllerVertical.Clone();
			wnd.DataContext = new ControllerConfigViewModel(port == 0 ? ControllerType.WsController : ControllerType.WsControllerVertical, cfg, orgCfg, port);

			if(await wnd.ShowDialogAtPosition<bool>(btn.GetVisualRoot() as Visual, startPosition)) {
				if(port == 0) {
					Config.ControllerHorizontal = cfg;
				} else {
					Config.ControllerVertical = cfg;
				}
			}
		}
	}

	public enum WsConfigTab
	{
		General,
		Audio,
		Emulation,
		Input,
		Video
	}
}
