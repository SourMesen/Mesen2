using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Mesen.Windows
{
	public class HdPackBuilderWindow : MesenWindow
	{
		private HdPackBuilderViewModel _model;

		public HdPackBuilderWindow()
		{
			_model = new HdPackBuilderViewModel();
			DataContext = _model;

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.StopRecording();
			_model.Dispose();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}