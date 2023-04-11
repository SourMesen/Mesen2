using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Utilities;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using System.Threading.Tasks;

namespace Mesen.Windows
{
	public class CheatEditWindow : MesenWindow
	{
		private CheatEditWindowViewModel _model;

		[Obsolete("For designer only")]
		public CheatEditWindow() : this(new CheatCode()) { }

		public CheatEditWindow(CheatCode cheat)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = new CheatEditWindowViewModel(cheat);
			DataContext = _model;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			this.GetControl<TextBox>("txtCodes").Focus();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Dispose();
		}

		public static async Task<bool> EditCheat(CheatCode cheat, Control parent)
		{
			CheatCode copy = cheat.Clone();
			CheatEditWindow wnd = new CheatEditWindow(copy);

			bool result = await wnd.ShowCenteredDialog<bool>(parent);
			if(result) {
				cheat.CopyFrom(copy);
			}

			return result;
		}
	}
}
