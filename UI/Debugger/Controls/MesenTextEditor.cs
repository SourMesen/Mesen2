using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Styling;
using AvaloniaEdit;
using AvaloniaEdit.Editing;
using System;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Controls
{
	public class MesenTextEditor : TextEditor
	{
		protected override Type StyleKeyOverride => typeof(TextEditor);

		public static readonly StyledProperty<string> TextBindingProperty = AvaloniaProperty.Register<MesenTextEditor, string>(nameof(TextBinding), "", defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);

		public string TextBinding
		{
			get { return GetValue(TextBindingProperty); }
			set { SetValue(TextBindingProperty, value); }
		}

		public ScrollViewer? ScrollViewer { get; private set; }

		public double VerticalScrollBarValue
		{
			get => ScrollViewer?.Offset.Y ?? 0;
			set
			{
				if(ScrollViewer != null) {
					ScrollViewer.Offset = ScrollViewer.Offset.WithY(value);
				}
			}
		}

		public event EventHandler<ScrollChangedEventArgs>? ScrollChanged
		{
			add => ScrollViewer!.ScrollChanged += value;
			remove => ScrollViewer!.ScrollChanged -= value;
		}

		private bool _readyEventSent = false;
		public event EventHandler<EventArgs>? TextEditorReady;

		static MesenTextEditor()
		{
			TextBindingProperty.Changed.AddClassHandler<MesenTextEditor>((x, e) => {
				if(x.Text != (string?)e.NewValue) {
					x.Text = (string)e.NewValue!;
				}
			});

			BoundsProperty.Changed.AddClassHandler<MesenTextEditor>((x, e) => {
				if(!x._readyEventSent && e.NewValue is Rect bounds && bounds.Width > 0 && bounds.Height > 0) {
					x._readyEventSent = true;
					x.TextEditorReady?.Invoke(x, EventArgs.Empty);
				}
			});
		}

		public MesenTextEditor()
		{
			Options.AllowScrollBelowDocument = false;
		}

		protected override void OnTextChanged(EventArgs e)
		{
			if(TextBinding != Text) {
				TextBinding = Text;
			}
			base.OnTextChanged(e);
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

		public void ScrollLineToMiddle(int lineNumber)
		{
			if(Bounds.Height > 0) {
				lineNumber = Math.Max(0, lineNumber - (int)(Bounds.Height / TextArea.TextView.DefaultLineHeight/ 2));
				VerticalScrollBarValue = lineNumber * TextArea.TextView.DefaultLineHeight;
			}
		}
	}
}
