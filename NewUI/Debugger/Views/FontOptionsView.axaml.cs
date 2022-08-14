using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;

namespace Mesen.Debugger.Views
{
	public class FontOptionsView : UserControl
	{
		public static readonly StyledProperty<string> InternalFontFamilyProperty = AvaloniaProperty.Register<FontOptionsView, string>(nameof(InternalFontFamily), "", defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

		public string InternalFontFamily
		{
			get { return GetValue(InternalFontFamilyProperty); }
			set { SetValue(InternalFontFamilyProperty, value); }
		}

		static FontOptionsView()
		{
			InternalFontFamilyProperty.Changed.AddClassHandler<FontOptionsView>((s, e) => s.ValidateFont());
		}

		public FontOptionsView()
		{
			InitializeComponent();

			ComboBox cboFontFamily = this.GetControl<ComboBox>("cboFontFamily");
			List<string> fontFamilies = FontManager.Current.GetInstalledFontFamilyNames().ToList();
			fontFamilies.Sort();
			cboFontFamily.Items = fontFamilies;

			ComboBox cboFontSize = this.GetControl<ComboBox>("cboFontSize");
			cboFontSize.Items = new float[] { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 18 };
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			if(DataContext is FontConfig cfg) {
				InternalFontFamily = cfg.FontFamily;
			}
		}

		private async void ValidateFont()
		{
			if(DataContext is FontConfig cfg) {
				if(cfg.FontFamily == InternalFontFamily) {
					//don't validate if this is the currently selected font
					return;
				}

				Typeface typeface = new Typeface(new FontFamily(InternalFontFamily));
				if(!typeface.GlyphTypeface.IsFixedPitch) {
					if(await MesenMsgBox.Show(VisualRoot, "NonMonospaceFontWarning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, InternalFontFamily) != Mesen.Windows.DialogResult.Yes) {
						InternalFontFamily = cfg.FontFamily;
						return;
					}
				}

				FormattedText text = new FormattedText("W", CultureInfo.CurrentCulture, FlowDirection.LeftToRight, typeface, 10, null);
				if(text.Width < 2 || text.Height < 2 || !double.IsFinite(text.Width) || !double.IsFinite(text.Height)) {
					await MesenMsgBox.Show(VisualRoot, "InvalidFont", MessageBoxButtons.OK, MessageBoxIcon.Error, InternalFontFamily);
					InternalFontFamily = cfg.FontFamily;
					return;
				}

				cfg.FontFamily = InternalFontFamily;
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
