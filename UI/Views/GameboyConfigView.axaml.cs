#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.ViewModels;
using Mesen.Windows;
using System;
using Avalonia.Media;
using System.Threading.Tasks;
using Avalonia.Interactivity;
using Avalonia.VisualTree;

namespace Mesen.Views
{
	public class GameboyConfigView : UserControl
	{
		private GameboyConfigViewModel _model;

		public GameboyConfigView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is GameboyConfigViewModel model) {
				_model = model;
			}
		}

		private async Task<Color> SelectColor(Color color)
		{
			ColorPickerViewModel model = new ColorPickerViewModel() { Color = color };
			ColorPickerWindow wnd = new ColorPickerWindow() { DataContext = model };

			bool success = await wnd.ShowCenteredDialog<bool>(this.GetVisualRoot() as Visual);
			if(success) {
				return model.Color;
			}
			return color;
		}

		private async void BgColor_OnClick(object sender, PaletteSelector.ColorClickEventArgs e)
		{
			Color color = await SelectColor(e.Color);
			UInt32[] colors = (UInt32[])_model.Config.BgColors.Clone();
			colors[e.ColorIndex] = color.ToUInt32();
			_model.Config.BgColors = colors;
		}

		private async void Obj0Color_OnClick(object sender, PaletteSelector.ColorClickEventArgs e)
		{
			Color color = await SelectColor(e.Color);
			UInt32[] colors = (UInt32[])_model.Config.Obj0Colors.Clone();
			colors[e.ColorIndex] = color.ToUInt32();
			_model.Config.Obj0Colors = colors;
		}

		private async void Obj1Color_OnClick(object sender, PaletteSelector.ColorClickEventArgs e)
		{
			Color color = await SelectColor(e.Color);
			UInt32[] colors = (UInt32[])_model.Config.Obj1Colors.Clone();
			colors[e.ColorIndex] = color.ToUInt32();
			_model.Config.Obj1Colors = colors;
		}

		private void btnSelectPreset_OnClick(object sender, RoutedEventArgs e)
		{
			((Button)sender).ContextMenu?.Open();
		}

		private void mnuGrayscalePreset_Click(object sender, RoutedEventArgs e)
		{
			SetPalette(Color.FromArgb(255, 232, 232, 232), Color.FromArgb(255, 160, 160, 160), Color.FromArgb(255, 88, 88, 88), Color.FromArgb(255, 16, 16, 16));
		}

		private void mnuGrayscaleHighContrastPreset_Click(object sender, RoutedEventArgs e)
		{
			SetPalette(Color.FromArgb(255, 255, 255, 255), Color.FromArgb(255, 176, 176, 176), Color.FromArgb(255, 104, 104, 104), Color.FromArgb(255, 0, 0, 0));
		}

		private void mnuGreenPreset_Click(object sender, RoutedEventArgs e)
		{
			SetPalette(Color.FromArgb(255, 224, 248, 208), Color.FromArgb(255, 136, 192, 112), Color.FromArgb(255, 52, 104, 86), Color.FromArgb(255, 8, 24, 32));
		}

		private void mnuBrownPreset_Click(object sender, RoutedEventArgs e)
		{
			SetPalette(Color.FromArgb(255, 248, 224, 136), Color.FromArgb(255, 216, 176, 88), Color.FromArgb(255, 152, 120, 56), Color.FromArgb(255, 72, 56, 24));
		}

		private void SetPalette(Color color0, Color color1, Color color2, Color color3)
		{
			_model.Config.BgColors = new UInt32[] { color0.ToUInt32(), color1.ToUInt32(), color2.ToUInt32(), color3.ToUInt32() };
			_model.Config.Obj0Colors = new UInt32[] { color0.ToUInt32(), color1.ToUInt32(), color2.ToUInt32(), color3.ToUInt32() };
			_model.Config.Obj1Colors = new UInt32[] { color0.ToUInt32(), color1.ToUInt32(), color2.ToUInt32(), color3.ToUInt32() };
		}
	}
}
