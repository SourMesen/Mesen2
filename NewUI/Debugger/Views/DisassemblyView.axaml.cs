using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;

namespace Mesen.Debugger.Views
{
	public class DisassemblyView : UserControl
	{
		private DisassemblyViewModel Model => (DisassemblyViewModel)DataContext!;

		static DisassemblyView()
		{
			BoundsProperty.Changed.AddClassHandler<DisassemblyView>((x, e) => {
				DisassemblyViewer viewer = x.FindControl<DisassemblyViewer>("disViewer");
				x.Model.VisibleRowCount = viewer.GetVisibleRowCount();
			});
		}

		public DisassemblyView()
		{
			InitializeComponent();
			Focusable = true;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void Diassembly_RowClicked(DisassemblyViewer sender, RowClickedEventArgs e)
		{
			if(e.MarginClicked && e.Properties.IsLeftButtonPressed) {
				CpuType cpuType = e.CodeLineData.CpuType;
				AddressInfo relAddress = new AddressInfo() {
					Address = e.CodeLineData.Address,
					Type = cpuType.ToMemoryType()
				};

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
				BreakpointManager.ToggleBreakpoint(absAddress.Address < 0 ? relAddress : absAddress, cpuType);
			}
		}

		public void Disassembly_PointerWheelChanged(object? sender, PointerWheelEventArgs e)
		{
			Model.Scroll((int)(-e.Delta.Y * 3));
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.PageDown: Model.Scroll(Model.VisibleRowCount - 2); e.Handled = true; break;
				case Key.PageUp: Model.Scroll(-(Model.VisibleRowCount - 2)); e.Handled = true; break;
				case Key.Home: Model.ScrollToTop(); e.Handled = true; break;
				case Key.End: Model.ScrollToBottom(); e.Handled = true; break;
			}
		}
	}
}
