using Avalonia;
using Avalonia.Controls;
using Mesen.GUI.Config;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Views;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class NesInputConfigViewModel : ViewModelBase
	{
		[Reactive] public NesConfig Config { get; set; }
		
		public ReactiveCommand<Button, Unit> SetupPlayer1 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer2 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer3 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer4 { get; }
		public ReactiveCommand<Button, Unit> SetupPlayer5 { get; }

		public NesInputConfigViewModel(NesConfig config)
		{
			Config = config;

			IObservable<bool> button1Enabled = this.WhenAnyValue(x => x.Config.Controllers[0].Type, x => x.CanConfigure());
			this.SetupPlayer1 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 0), button1Enabled);

			IObservable<bool> button2Enabled = this.WhenAnyValue(x => x.Config.Controllers[1].Type, x => x.CanConfigure());
			this.SetupPlayer2 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 1), button2Enabled);

			IObservable<bool> button3Enabled = this.WhenAnyValue(x => x.Config.Controllers[2].Type, x => x.CanConfigure());
			this.SetupPlayer3 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 2), button3Enabled);

			IObservable<bool> button4Enabled = this.WhenAnyValue(x => x.Config.Controllers[3].Type, x => x.CanConfigure());
			this.SetupPlayer4 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 3), button4Enabled);

			IObservable<bool> button5Enabled = this.WhenAnyValue(x => x.Config.Controllers[4].Type, x => x.CanConfigure());
			this.SetupPlayer5 = ReactiveCommand.Create<Button>(btn => this.OpenSetup(btn, 4), button5Enabled);
		}

		private async void OpenSetup(Button btn, int port)
		{
			PixelPoint startPosition = btn.PointToScreen(new Point(-7, btn.Height));
			ControllerConfigWindow wnd = new ControllerConfigWindow();
			wnd.WindowStartupLocation = WindowStartupLocation.Manual;
			wnd.Position = startPosition;

			ControllerConfig cfg = JsonHelper.Clone(this.Config.Controllers[port]);
			wnd.DataContext = new ControllerConfigViewModel(cfg);

			if(await wnd.ShowDialog<bool>(btn.Parent.VisualRoot as Window)) {
				this.Config.Controllers[port] = cfg;
			}
		}
	}
}
