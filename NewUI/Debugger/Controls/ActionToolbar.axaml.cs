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
using Mesen.Debugger.ViewModels;
using Avalonia.Threading;
using Mesen.Debugger.Utilities;

namespace Mesen.Debugger.Controls
{
	public class ActionToolbar : UserControl
	{
		public static readonly StyledProperty<List<object>> ItemsProperty = AvaloniaProperty.Register<ActionToolbar, List<object>>(nameof(Items));
		private DispatcherTimer _timer;

		public List<object> Items
		{
			get { return GetValue(ItemsProperty); }
			set { SetValue(ItemsProperty, value); }
		}

		public ActionToolbar()
		{
			InitializeComponent();
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateToolbar());
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_timer.Start();
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			_timer.Stop();
		}

		private void UpdateToolbar()
		{
			foreach(object item in Items) {
				if(item is ContextMenuAction act) {
					act.Update();
				}
			}
		}
	}
}
