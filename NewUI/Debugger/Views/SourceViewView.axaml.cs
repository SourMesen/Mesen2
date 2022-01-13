using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using AvaloniaEdit.Editing;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using AvaloniaEdit.Rendering;
using AvaloniaEdit.Utils;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System.Globalization;
using System.Linq;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Views
{
	public class SourceViewView : UserControl
	{
		static SourceViewView()
		{
			DataContextProperty.Changed.AddClassHandler<SourceViewView>((s, e) => {
				if(e.OldValue is SourceViewViewModel model) {
					model.SaveScrollPosition();
					model.SetEditor(null);
				}
			});
		}

		public SourceViewView()
		{
			InitializeComponent();
		}

		protected override void OnDataContextChanged(System.EventArgs e)
		{
			if(DataContext is SourceViewViewModel model) {
				MesenTextEditor editor = this.FindControl<MesenTextEditor>("TextEditor");
				model.SetEditor(editor);
			}
			base.OnDataContextChanged(e);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}

	public class ActiveLineBackgroundRenderer : IBackgroundRenderer
	{
		private static ISolidColorBrush ActiveBackground = Brushes.Yellow;
		private SourceViewViewModel _model;
		private int? _activeAddress;

		public ActiveLineBackgroundRenderer(SourceViewViewModel model)
		{
			_model = model;
		}

		public KnownLayer Layer
		{
			get { return KnownLayer.Background; }
		}

		public void Draw(TextView textView, DrawingContext drawingContext)
		{
			if(_model.SelectedFile == null) {
				return;
			}

			foreach(var v in textView.VisualLines) {
				string? originalText = v.GetTextLine(0).GetTextRuns()[0].StringRange.String;
				int indent = originalText == null ? 0 : originalText.Length - originalText.TrimStart().Length;
				var rc = BackgroundGeometryBuilder.GetRectsFromVisualSegment(textView, v, indent, 1000).First();
				int linenum = v.FirstDocumentLine.LineNumber - 1;
				AddressInfo? address = _model.SymbolProvider.GetLineAddress(_model.SelectedFile, linenum);
				if(address != null) {
					AddressInfo relAddress = DebugApi.GetRelativeAddress(address.Value, _model.CpuType);
					if(relAddress.Address == _activeAddress) {
						drawingContext.FillRectangle(ActiveBackground, rc);
					}
				}
			}
		}

		public void SetActiveAddress(int? activeAddress)
		{
			_activeAddress = activeAddress;
		}
	}

	public class LineNumberMargin : AbstractMargin
	{
		protected FontFamily Typeface { get; set; }
		protected double EmSize { get; set; }
		private SourceViewViewModel _model;

		private int MaxLineNumberLength { get; } = 5;

		public LineNumberMargin(SourceViewViewModel model)
		{
			_model = model;
			Typeface = FontFamily.Default;
			EmSize = 12;
		}

		protected override Size MeasureOverride(Size availableSize)
		{
			Typeface = GetValue(TextBlock.FontFamilyProperty);
			EmSize = GetValue(TextBlock.FontSizeProperty);

			var text = TextFormatterFactory.CreateFormattedText(
				 this,
				 new string('9', MaxLineNumberLength),
				 Typeface,
				 EmSize,
				 GetValue(TemplatedControl.ForegroundProperty)
			);
			return new Size(text.Bounds.Width + 5, 0);
		}

		public override void Render(DrawingContext context)
		{
			var textView = TextView;
			var renderSize = Bounds.Size;
			if(textView != null && textView.VisualLinesValid && _model.SelectedFile != null) {
				context.FillRectangle(ColorHelper.GetBrush(Color.FromRgb(235, 235, 235)), new Rect(renderSize));
				context.DrawLine(ColorHelper.GetPen(Colors.LightGray), new Point(renderSize.Width, 0), new Point(renderSize.Width, renderSize.Height));
				SolidColorBrush addressBrush = ColorHelper.GetBrush(Colors.Gray);
				foreach(var line in textView.VisualLines) {
					var lineNumber = line.FirstDocumentLine.LineNumber - 1;
					AddressInfo? address = _model.SymbolProvider.GetLineAddress(_model.SelectedFile, lineNumber);
					if(address != null) {
						AddressInfo relAddress = DebugApi.GetRelativeAddress(address.Value, _model.CpuType);
						if(relAddress.Address >= 0) {
							var text = TextFormatterFactory.CreateFormattedText(
								 this,
								 relAddress.Address.ToString("X4"),
								 Typeface, EmSize, addressBrush
							);
							var y = line.GetTextLineVisualYPosition(line.TextLines[0], VisualYPosition.TextTop);
							context.DrawText(addressBrush, new Point(renderSize.Width - text.Bounds.Width, y - textView.VerticalOffset), text);
						}
					}
				}
			}
		}
	}
}
