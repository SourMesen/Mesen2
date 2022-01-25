using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Debugger.Views
{
	public class FontOptionsView : UserControl
	{
		public FontOptionsView()
		{
			InitializeComponent();

			ComboBox cboFontFamily = this.FindControl<ComboBox>("cboFontFamily");
			List<string> fontFamilies = FontManager.Current.GetInstalledFontFamilyNames().Where((family) => {
				return new Typeface(new FontFamily(family)).GlyphTypeface.IsFixedPitch;
			}).ToList();
			fontFamilies.Sort();
			cboFontFamily.Items = fontFamilies;

			ComboBox cboFontSize = this.FindControl<ComboBox>("cboFontSize");
			cboFontSize.Items = new float[] { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
