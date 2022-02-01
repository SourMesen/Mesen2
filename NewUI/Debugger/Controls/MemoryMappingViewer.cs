using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Threading;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public class MemoryMappingBlock
	{
		public string Name = "";
		public int Length;
		public Color Color;
		public string Note = "";
		public int Page = -1;
	}

	public partial class MemoryMappingViewer : Control
	{
		public static readonly StyledProperty<List<MemoryMappingBlock>> MappingsProperty = AvaloniaProperty.Register<MemoryMappingViewer, List<MemoryMappingBlock>>(nameof(Mappings));
		private const int BlockHeight = 32;

		public List<MemoryMappingBlock> Mappings
		{
			get { return GetValue(MappingsProperty); }
			set { SetValue(MappingsProperty, value); }
		}

		static MemoryMappingViewer()
		{
			AffectsRender<MemoryMappingViewer>(MappingsProperty);
			AffectsMeasure<MemoryMappingViewer>(MappingsProperty);
		}

		public MemoryMappingViewer()
		{
		}

		protected override Size MeasureOverride(Size availableSize)
		{
			if(Mappings == null) {
				return Size.Empty;
			}
			return new Size(availableSize.Width, BlockHeight);
		}

		public override void Render(DrawingContext context)
		{
			if(Mappings == null) {
				return;
			}

			List<MemoryMappingBlock> mappings = new(Mappings);

			int totalSize = mappings.Sum(m => m.Length);

			Size size = Bounds.Size;

			double pixelsPerByte = size.Width / totalSize;

			int start = 0;
			double x = 0;
			FormattedText noteText = new FormattedText("", new Typeface("Arial"), 9, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			FormattedText addressText = new FormattedText("", new Typeface("Arial"), 11, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			FormattedText text = new FormattedText("", new Typeface("Arial"), 12, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			Pen borderPen = new Pen(Brushes.Black);
			for(int i = 0; i < mappings.Count; i++) {
				MemoryMappingBlock block = mappings[i];

				double blockWidth = Math.Round(block.Length * pixelsPerByte);
				if(i == mappings.Count - 1) {
					blockWidth = Bounds.Width - x - 1;
				}

				context.DrawRectangle(new SolidColorBrush(block.Color), borderPen, new Rect(x - 0.5, 0.5, blockWidth + 1, BlockHeight));

				if(string.IsNullOrEmpty(block.Name)) {
					text.Text = block.Page >= 0 ? $"${block.Page:X2}" : "";
				} else if(block.Page >= 0) {
					text.Text = $"{block.Name} (${block.Page:X2})";
				} else {
					text.Text = block.Name;
				}
				addressText.Text = start.ToString("X4");
				double margin = addressText.Bounds.Height;

				if(text.Bounds.Width >= blockWidth - margin) {
					//Hide name if there's no space
					text.Text = block.Page >= 0 ? $"${block.Page:X2}" : text.Text;
				}

				if(text.Bounds.Width >= blockWidth - margin) {
					//Hide address text if there's no space
					margin = 0;
					addressText.Text = "";
				}

				if(text.Bounds.Width < blockWidth - margin) {
					context.DrawText(Brushes.Black, new Point(x + (blockWidth + margin - text.Bounds.Width) / 2, (BlockHeight - text.Bounds.Height) / 2), text);
				}

				if(addressText.Bounds.Height < blockWidth - 4) {
					using var rotate = context.PushPostTransform(Matrix.CreateRotation(-Math.PI / 2));
					context.DrawText(Brushes.Black, new Point(-BlockHeight + (BlockHeight - addressText.Bounds.Width) / 2, x), addressText);
				}

				if(!string.IsNullOrEmpty(block.Note)) {
					noteText.Text = block.Note;
					if(noteText.Bounds.Width < blockWidth - 15) {
						context.DrawText(Brushes.Black, new Point(x + blockWidth - noteText.Bounds.Width - 3, BlockHeight - addressText.Bounds.Height + 4), noteText);
					}
				}

				start += block.Length;
				x += blockWidth;
			}
		}
	}
}