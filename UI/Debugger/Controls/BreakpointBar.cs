using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public class BreakpointBar : Control
	{
		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			BreakpointManager.BreakpointsChanged += BreakpointManager_BreakpointsChanged;
			if(DataContext is DisassemblyViewModel disModel) {
				disModel.SetRefreshScrollBar(() => InvalidateVisual());
			} else if(DataContext is SourceViewViewModel srcModel) {
				srcModel.SetRefreshScrollBar(() => InvalidateVisual());
			}
			base.OnAttachedToVisualTree(e);
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			BreakpointManager.BreakpointsChanged -= BreakpointManager_BreakpointsChanged;
			if(DataContext is DisassemblyViewModel disModel) {
				disModel.SetRefreshScrollBar(null);
			} else if(DataContext is SourceViewViewModel srcModel) {
				srcModel.SetRefreshScrollBar(null);
			}
			base.OnDetachedFromVisualTree(e);
		}

		private void BreakpointManager_BreakpointsChanged(object? sender, EventArgs e)
		{
			InvalidateVisual();
		}

		public override void Render(DrawingContext context)
		{
			base.Render(context);

			double height = Bounds.Height;
			double width = Bounds.Width - 1.0;
			if(DataContext is DisassemblyViewModel disModel) {
				int maxAddress = DebugApi.GetMemorySize(disModel.CpuType.ToMemoryType()) - 1;

				int? activeAddress = disModel.ActiveAddress;
				if(activeAddress >= 0) {
					int position = (int)(((double)activeAddress.Value / maxAddress) * height) - 2;
					SolidColorBrush brush = new(ConfigManager.Config.Debug.Debugger.CodeActiveStatementColor);
					Pen pen = new Pen(Colors.Black.ToUInt32());
					context.FillRectangle(brush, new Rect(0.5, position - 0.5, width, 3));
					context.DrawRectangle(pen, new Rect(0.5, position - 0.5, width, 3));
				}

				foreach(Breakpoint bp in BreakpointManager.GetBreakpoints(disModel.CpuType)) {
					int address = bp.GetRelativeAddress();
					if(address >= 0 && (bp.IsSingleAddress || bp.BreakOnExec)) {
						int position = (int)(((double)address / maxAddress) * height) - 2;
						if(bp.Enabled) {
							SolidColorBrush brush = new(bp.GetColor());
							context.FillRectangle(brush, new Rect(0, position, 4, 4));
						} else {
							Pen pen = new Pen(bp.GetColor().ToUInt32());
							SolidColorBrush brush = new(Colors.White);
							context.FillRectangle(brush, new Rect(0, position, 4, 4));
							context.DrawRectangle(pen, new Rect(0.5, position + 0.5, 3, 3));
						}
					}
				}
			} else if(DataContext is SourceViewViewModel srcModel && srcModel.SelectedFile != null) {
				int? activeLine = srcModel.GetActiveLineIndex();
				if(activeLine >= 0) {
					int position = (int)(((double)activeLine.Value / srcModel.SelectedFile.Data.Length) * height) - 2;
					SolidColorBrush brush = new(ConfigManager.Config.Debug.Debugger.CodeActiveStatementColor);
					Pen pen = new Pen(Colors.Black.ToUInt32());
					context.FillRectangle(brush, new Rect(0.5, position - 0.5, width, 3));
					context.DrawRectangle(pen, new Rect(0.5, position - 0.5, width, 3));
				}

				for(int i = 0, len = srcModel.SelectedFile.Data.Length; i < len; i++) {
					Breakpoint? bp = srcModel.GetBreakpoint(i);
					if(bp != null) {
						int position = (int)(((double)i / len) * height) - 2;
						if(bp.Enabled) {
							SolidColorBrush brush = new SolidColorBrush(bp.GetColor());
							context.FillRectangle(brush, new Rect(0, position, 4, 4));
						} else {
							Pen pen = new Pen(bp.GetColor().ToUInt32());
							context.DrawRectangle(pen, new Rect(0.5, position + 0.5, 3, 3));
						}
					}
				}
			}
		}
	}
}
