using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Controls;
using Mesen.Utilities;
using System;
using System.Linq;

namespace Mesen.Windows
{
	public class InputBarcodeWindow : MesenWindow
	{
		private static string _lastBarcode { get; set; } = "";

		public static readonly StyledProperty<string> BarcodeProperty = AvaloniaProperty.Register<InputBarcodeWindow, string>(nameof(Barcode), "");

		public string Barcode
		{
			get { return GetValue(BarcodeProperty); }
			set { SetValue(BarcodeProperty, value); }
		}

		static InputBarcodeWindow()
		{
			BarcodeProperty.Changed.AddClassHandler<InputBarcodeWindow>((x, e) => {
				//Only allow numbers in textbox
				char[] result = x.Barcode.Where(c => c >= '0' && c <= '9').ToArray();
				if(result.Length != x.Barcode.Length) {
					Dispatcher.UIThread.Post(() => {
						x.Barcode = new string(result);
					});
				}
			});
		}

		public InputBarcodeWindow()
		{
			Barcode = _lastBarcode;

			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			this.GetControl<TextBox>("txtBarcode").FocusAndSelectAll();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_lastBarcode = Barcode;
			Close(Barcode);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(null!);
		}
	}
}
