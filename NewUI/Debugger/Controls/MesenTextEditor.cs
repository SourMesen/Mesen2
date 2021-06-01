using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Styling;
using AvaloniaEdit;
using System;

namespace Mesen.Debugger.Controls
{
	public class MesenTextEditor : TextEditor, IStyleable
	{
		Type IStyleable.StyleKey => typeof(TextEditor);

		public static readonly StyledProperty<string> TextBindingProperty = AvaloniaProperty.Register<MesenTextEditor, string>(nameof(TextBinding), "");

		public string TextBinding
		{
			get { return GetValue(TextBindingProperty); }
			set { SetValue(TextBindingProperty, value); }
		}

		public ScrollViewer? ScrollViewer { get; private set; }

		public double VerticalScrollBarValue
		{
			get => ScrollViewer!.Offset.Y;
			set => ScrollViewer!.Offset = ScrollViewer.Offset.WithY(value);
		}

		public event EventHandler<ScrollChangedEventArgs>? ScrollChanged
		{
			add => ScrollViewer!.ScrollChanged += value;
			remove => ScrollViewer!.ScrollChanged -= value;
		}

		static MesenTextEditor()
		{
			TextBindingProperty.Changed.AddClassHandler<MesenTextEditor>((x, e) => {
				x.Text = (string)e.NewValue!;
			});
		}

		public MesenTextEditor()
		{
		}

		protected override void OnApplyTemplate(TemplateAppliedEventArgs e)
		{
			base.OnApplyTemplate(e);
			ScrollViewer = e.NameScope.Find<ScrollViewer>("PART_ScrollViewer");
		}

		public void ScrollLineToTop(int line)
		{
			VerticalScrollBarValue = line * TextArea.TextView.DefaultLineHeight;
		}
	}
}
