using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Interop;
using Mesen.Config;
using ReactiveUI.Fody.Helpers;
using System;
using Mesen.Debugger.Controls;
using Mesen.Utilities;
using Avalonia.Threading;

namespace Mesen.Debugger.Windows
{
	public class ColorIndexPickerWindow : Window
	{
		public UInt32[] Palette { get; set; } = Array.Empty<UInt32>();
		public int SelectedPalette { get; set; }
		public int BlockSize { get; set; } = 24;
		public int ColumnCount { get; set; } = 16;

		[Obsolete("For designer only")]
		public ColorIndexPickerWindow() : this(CpuType.Nes, 0) { }

		public ColorIndexPickerWindow(CpuType cpuType, int selectedPalette)
		{
			SelectedPalette = selectedPalette;
			switch(cpuType) {
				case CpuType.Nes:
					Palette = ConfigManager.Config.Nes.UserPalette;
					break;

				case CpuType.Pce:
					Palette = ConfigManager.Config.PcEngine.Palette;
					break;

				case CpuType.Gameboy:
					if(selectedPalette < 4) {
						Palette = ConfigManager.Config.Gameboy.BgColors;
					} else if(selectedPalette < 8) {
						Palette = ConfigManager.Config.Gameboy.Obj0Colors;
					} else {
						Palette = ConfigManager.Config.Gameboy.Obj1Colors;
					}
					ColumnCount = 4;
					SelectedPalette = selectedPalette % 4;
					break;

				default:
					throw new NotImplementedException();
			}

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
			if(Owner != null) {
				WindowExtensions.CenterWindow(this, Owner);
			}
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(null!);
		}

		private void PaletteColor_OnClick(object sender, PaletteSelector.ColorClickEventArgs e)
		{
			Close(e.ColorIndex);
		}
	}
}
