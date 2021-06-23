using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System.Reactive.Linq;

namespace Mesen.Debugger.ViewModels
{
	public class NesPpuViewModel : ViewModelBase
	{
		[ObservableAsProperty] public string? Cycle { get; }
		[ObservableAsProperty] public string? Scanline { get; }
		
		[Reactive] public NesPpuState State { get; set; }

		public NesPpuViewModel()
		{
			this.State = new NesPpuState();
			this.WhenAnyValue(x => x.State).Select(st => st.Cycle.ToString()).ToPropertyEx(this, x => x.Cycle);
			this.WhenAnyValue(x => x.State).Select(st => st.Scanline.ToString()).ToPropertyEx(this, x => x.Scanline);
		}
	}
}
