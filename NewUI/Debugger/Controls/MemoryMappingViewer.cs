using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public record class MemoryMappingBlock
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
		public static readonly StyledProperty<MemoryType> MemTypeProperty = AvaloniaProperty.Register<MemoryMappingViewer, MemoryType>(nameof(MemType));
		private MemoryMappingBlock? _prevTooltipMapping;
		private const int BlockHeight = 32;

		public List<MemoryMappingBlock> Mappings
		{
			get { return GetValue(MappingsProperty); }
			set { SetValue(MappingsProperty, value); }
		}

		public MemoryType MemType
		{
			get { return GetValue(MemTypeProperty); }
			set { SetValue(MemTypeProperty, value); }
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

		protected override void OnPointerMoved(PointerEventArgs e)
		{
			base.OnPointerMoved(e);

			List<MemoryMappingBlock> mappings = new(Mappings);

			int totalSize = mappings.Sum(m => m.Length);
			Size size = Bounds.Size;
			double pixelsPerByte = size.Width / totalSize;
			double x = e.GetCurrentPoint(this).Position.X;
			double pos = 0;
			MemoryMappingBlock? hoveredMapping = null;
			int start = 0;
			foreach(MemoryMappingBlock mapping in mappings) {
				pos += mapping.Length * pixelsPerByte;
				if(pos >= x) {
					hoveredMapping = mapping;
					break;
				}
				start += mapping.Length;
			}

			if(_prevTooltipMapping == hoveredMapping) {
				return;
			}

			_prevTooltipMapping = hoveredMapping;

			if(hoveredMapping != null) {
				TooltipEntries entries = new TooltipEntries();
				DynamicTooltip dynTooltip = new DynamicTooltip();
				entries.AddEntry("Entry", GetBlockText(hoveredMapping));
				int end = start + hoveredMapping.Length - 1;
				entries.AddEntry($"Range ({MemType.GetShortName()})", "$" + start.ToString("X4") + " - $" + end.ToString("X4"));

				AddressInfo absStart = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = start, Type = MemType });
				AddressInfo absEnd = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = end, Type = MemType });
				if(absStart.Address >= 0 && absEnd.Address >= 0 && absStart.Type == absEnd.Type) {
					entries.AddEntry($"Range ({absStart.Type.GetShortName()})", "$" + absStart.Address.ToString("X4") + " - $" + absEnd.Address.ToString("X4"));
				}

				if(hoveredMapping.Note.StartsWith("RW")) {
					entries.AddEntry("Access", "Read/Write");
				} else if(hoveredMapping.Note.StartsWith("R")) {
					entries.AddEntry("Access", "Read-only");
				} else if(hoveredMapping.Note.StartsWith("W")) {
					entries.AddEntry("Access", "Write-only");
				} else if(hoveredMapping.Note.StartsWith("OB")) {
					entries.AddEntry("Access", "Open bus (unmapped)");
				}
				dynTooltip.Items = entries;
				ToolTip.SetTip(this, dynTooltip);

				//Force tooltip to update its position
				ToolTip.SetPlacement(this, PlacementMode.Pointer);
				ToolTip.SetHorizontalOffset(this, 0);
				ToolTip.SetHorizontalOffset(this, 1);
				ToolTip.SetIsOpen(this, true);
			} else {
				ToolTip.SetTip(this, null);
				ToolTip.SetIsOpen(this, false);
			}
		}

		protected override void OnPointerLeave(PointerEventArgs e)
		{
			base.OnPointerLeave(e);
			_prevTooltipMapping = null;
			ToolTip.SetTip(this, null);
			ToolTip.SetIsOpen(this, false);
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
			Pen borderPen = ColorHelper.GetPen(Color.FromRgb(0x60, 0x60, 0x60));
			for(int i = 0; i < mappings.Count; i++) {
				MemoryMappingBlock block = mappings[i];

				double blockWidth = Math.Round(block.Length * pixelsPerByte);
				if(i == mappings.Count - 1) {
					blockWidth = Bounds.Width - x - 1;
				}

				context.DrawRectangle(ColorHelper.GetBrush(block.Color), borderPen, new Rect(x - 0.5, 0.5, blockWidth + 1, BlockHeight));
				text.Text = GetBlockText(block);
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
					context.DrawText(ColorHelper.GetBrush(Colors.Black), new Point(x + (blockWidth + margin - text.Bounds.Width) / 2, (BlockHeight - text.Bounds.Height) / 2), text);
				}

				if(addressText.Bounds.Height < blockWidth - 4) {
					using var rotate = context.PushPostTransform(Matrix.CreateRotation(-Math.PI / 2));
					context.DrawText(ColorHelper.GetBrush(Colors.Black), new Point(-BlockHeight + (BlockHeight - addressText.Bounds.Width) / 2, x), addressText);
				}

				if(!string.IsNullOrEmpty(block.Note)) {
					noteText.Text = block.Note;
					if(noteText.Bounds.Width < blockWidth - 15) {
						context.DrawText(ColorHelper.GetBrush(Colors.Black), new Point(x + blockWidth - noteText.Bounds.Width - 3, BlockHeight - noteText.Bounds.Height), noteText);
					}
				}

				start += block.Length;
				x += blockWidth;
			}
		}

		private static string GetBlockText(MemoryMappingBlock block)
		{
			if(string.IsNullOrEmpty(block.Name)) {
				return block.Page >= 0 ? $"${block.Page:X2}" : "";
			} else if(block.Page >= 0) {
				return $"{block.Name} (${block.Page:X2})";
			} else {
				return block.Name;
			}
		}
	}
}