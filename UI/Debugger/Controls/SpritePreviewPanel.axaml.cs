using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Debugger.ViewModels;
using Mesen.Utilities;
using ReactiveUI;
using System;
using System.Reactive.Linq;

namespace Mesen.Debugger.Controls
{
	public class SpritePreviewPanel : MesenUserControl
	{
		public static readonly StyledProperty<double> InnerHeightProperty = AvaloniaProperty.Register<SpritePreviewPanel, double>(nameof(InnerHeight), 0);
		public static readonly StyledProperty<double> InnerWidthProperty = AvaloniaProperty.Register<SpritePreviewPanel, double>(nameof(InnerWidth), 0);
		public static readonly StyledProperty<bool> FadePreviewProperty = AvaloniaProperty.Register<SpritePreviewPanel, bool>(nameof(FadePreview), false);
		public static readonly StyledProperty<Thickness> BorderSizeProperty = AvaloniaProperty.Register<SpritePreviewPanel, Thickness>(nameof(BorderSize), default);

		public bool FadePreview
		{
			get { return GetValue(FadePreviewProperty); }
			set { SetValue(FadePreviewProperty, value); }
		}

		public double InnerHeight
		{
			get { return GetValue(InnerHeightProperty); }
			set { SetValue(InnerHeightProperty, value); }
		}

		public double InnerWidth
		{
			get { return GetValue(InnerWidthProperty); }
			set { SetValue(InnerWidthProperty, value); }
		}

		public Thickness BorderSize
		{
			get { return GetValue(BorderSizeProperty); }
			set { SetValue(BorderSizeProperty, value); }
		}

		public SpritePreviewModel Model { get; private init; }
		public SpriteViewerConfig Config { get; private init; }

		[Obsolete("For designer only")]
		public SpritePreviewPanel() : this(new(), new()) { }

		public SpritePreviewPanel(SpritePreviewModel model, SpriteViewerConfig config)
		{
			InitializeComponent();

			Model = model;
			Config = config;

			AddDisposable(this.WhenAnyValue(x => x.Model.FadePreview, x => x.Config.DimOffscreenSprites).Subscribe(x => {
				FadePreview = Model.FadePreview == true && Config.DimOffscreenSprites == true;
			}));
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			BorderSize = new Thickness(1 / LayoutHelper.GetLayoutScale(this));
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
