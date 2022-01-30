using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Controls;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.Windows
{
	public class GoToWindow : Window
	{
		public static int _lastAddress { get; set; } = 0;

		public int Address { get; set; }
		public int Maximum { get; set; }

		[Obsolete("For designer only")]
		public GoToWindow() : this(0) { }

		public GoToWindow(int maximum)
		{
			Address = _lastAddress < maximum ? _lastAddress : 0;
			Maximum = maximum;

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			this.FindControl<MesenNumericTextBox>("txtAddress").Focus();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_lastAddress = Address;
			Close(Address);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(null);
		}
	}
}
