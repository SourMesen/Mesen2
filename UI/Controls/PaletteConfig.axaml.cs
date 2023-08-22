using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Linq;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Avalonia.Data;
using System.IO;
using Mesen.ViewModels;
using Mesen.Debugger.Controls;
using Mesen.Windows;
using Mesen.Utilities;
using ReactiveUI;
using System.Reactive;
using Avalonia.VisualTree;

namespace Mesen.Controls
{
	public class PaletteConfig : UserControl
	{
		public static readonly StyledProperty<UInt32[]> PaletteProperty = AvaloniaProperty.Register<PaletteConfig, UInt32[]>(nameof(Palette), Array.Empty<UInt32>(), defaultBindingMode: BindingMode.TwoWay);
		public static readonly StyledProperty<List<PalettePreset>> PalettePresetsProperty = AvaloniaProperty.Register<PaletteConfig, List<PalettePreset>>(nameof(PalettePresets), new List<PalettePreset>());
		public static readonly StyledProperty<int> ColumnCountProperty = AvaloniaProperty.Register<PaletteConfig, int>(nameof(ColumnCount), 16);
		public static readonly StyledProperty<int> BlockSizeProperty = AvaloniaProperty.Register<PaletteConfig, int>(nameof(BlockSize), 16);
		public static readonly StyledProperty<bool> ShowIndexesProperty = AvaloniaProperty.Register<PaletteConfig, bool>(nameof(ShowIndexes), false);
		public static readonly StyledProperty<int> SmallPaletteSizeProperty = AvaloniaProperty.Register<PaletteConfig, int>(nameof(SmallPaletteSize), 64);
		public static readonly StyledProperty<int> LargePaletteSizeProperty = AvaloniaProperty.Register<PaletteConfig, int>(nameof(LargePaletteSize), 512);

		public UInt32[] Palette
		{
			get { return GetValue(PaletteProperty); }
			set { SetValue(PaletteProperty, value); }
		}

		public List<PalettePreset> PalettePresets
		{
			get { return GetValue(PalettePresetsProperty); }
			set { SetValue(PalettePresetsProperty, value); }
		}

		public int ColumnCount
		{
			get { return GetValue(ColumnCountProperty); }
			set { SetValue(ColumnCountProperty, value); }
		}

		public int BlockSize
		{
			get { return GetValue(BlockSizeProperty); }
			set { SetValue(BlockSizeProperty, value); }
		}

		public bool ShowIndexes
		{
			get { return GetValue(ShowIndexesProperty); }
			set { SetValue(ShowIndexesProperty, value); }
		}

		public int SmallPaletteSize
		{
			get { return GetValue(SmallPaletteSizeProperty); }
			set { SetValue(SmallPaletteSizeProperty, value); }
		}

		public int LargePaletteSize
		{
			get { return GetValue(LargePaletteSizeProperty); }
			set { SetValue(LargePaletteSizeProperty, value); }
		}

		public PaletteConfig()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void btnSelectPalette_OnClick(object sender, RoutedEventArgs e)
		{
			((Button)sender).ContextMenu?.Open();
		}

		private async void PaletteColor_OnClick(object sender, PaletteSelector.ColorClickEventArgs e)
		{
			ColorPickerViewModel model = new ColorPickerViewModel() { Color = e.Color };
			ColorPickerWindow wnd = new ColorPickerWindow() { DataContext = model };

			bool success = await wnd.ShowCenteredDialog<bool>(this.GetVisualRoot() as Visual);
			if(success) {
				UInt32[] colors = (UInt32[])Palette.Clone();
				colors[e.ColorIndex] = model.Color.ToUInt32();
				Palette = colors;
			}
		}

		private async void btnLoadPalFile_OnClick(object sender, RoutedEventArgs e)
		{
			string? filename = await FileDialogHelper.OpenFile(null, this.GetVisualRoot(), FileDialogHelper.PaletteExt);
			if(filename != null) {
				LoadPaletteFile(filename);
			}
		}

		private async void btnExportPalette_OnClick(object sender, RoutedEventArgs e)
		{
			string? filename = await FileDialogHelper.SaveFile(null, null, this.GetVisualRoot(), FileDialogHelper.PaletteExt);
			if(filename != null) {
				ExportPalette(filename);
			}
		}

		public void SelectPreset(object parameter)
		{
			Palette = (UInt32[])((UInt32[])parameter).Clone();
		}

		public void LoadPaletteFile(string filename)
		{
			using FileStream? paletteFile = FileHelper.OpenRead(filename);
			if(paletteFile == null) {
				return;
			}

			byte[] paletteFileData = new byte[LargePaletteSize * 3 + 1];
			int byteCount = paletteFile.Read(paletteFileData, 0, LargePaletteSize * 3 + 1);
			if(byteCount == LargePaletteSize * 3 || byteCount == SmallPaletteSize * 3) {
				UInt32[] paletteData = new UInt32[byteCount / 3];
				for(int i = 0; i < byteCount; i += 3) {
					paletteData[i / 3] = ((UInt32)0xFF000000 | (UInt32)paletteFileData[i + 2] | (UInt32)(paletteFileData[i + 1] << 8) | (UInt32)(paletteFileData[i] << 16));
				}
				Palette = paletteData;
			} else {
				MesenMsgBox.Show(VisualRoot, "InvalidPaletteFile", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
			paletteFile.Close();
		}

		public void ExportPalette(string filename)
		{
			List<byte> bytePalette = new List<byte>();
			foreach(UInt32 value in Palette) {
				bytePalette.Add((byte)(value >> 16 & 0xFF));
				bytePalette.Add((byte)(value >> 8 & 0xFF));
				bytePalette.Add((byte)(value & 0xFF));
			}
			FileHelper.WriteAllBytes(filename, bytePalette.ToArray());
		}
	}

	public class PalettePreset
	{
		public string Name { get; set; } = "";
		public UInt32[] Palette { get; set; } = Array.Empty<UInt32>();
	}
}
