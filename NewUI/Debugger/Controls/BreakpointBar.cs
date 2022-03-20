using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
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
			if(DataContext is DisassemblyViewModel model) {
				model.SetRefreshScrollBar(() => InvalidateVisual());
			}
			base.OnAttachedToVisualTree(e);
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			BreakpointManager.BreakpointsChanged -= BreakpointManager_BreakpointsChanged;
			if(DataContext is DisassemblyViewModel model) {
				model.SetRefreshScrollBar(null);
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

			if(DataContext is DisassemblyViewModel model) {
				double height = Bounds.Height;
				int maxAddress = DebugApi.GetMemorySize(model.CpuType.ToMemoryType()) - 1;
				foreach(Breakpoint bp in BreakpointManager.GetBreakpoints(model.CpuType)) {
					int address = bp.GetRelativeAddress();
					if(address >= 0) {
						int position = (int)(((double)address / maxAddress) * height) - 2;
						context.FillRectangle(Brushes.DarkRed, new Rect(0, position, 4, 4));
					}
				}
			}
		}
	}
}
