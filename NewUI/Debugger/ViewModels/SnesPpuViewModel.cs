using Avalonia.Controls;
using Mesen.GUI;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class SnesPpuViewModel : ViewModelBase
	{
		[ObservableAsProperty] public string Cycle { get; }
		[ObservableAsProperty] public string Scanline { get; }
		[ObservableAsProperty] public string HClock { get; }

		private PpuState _state;
		public PpuState State
		{
			get => _state;
			set => this.RaiseAndSetIfChanged(ref _state, value);
		}

		public SnesPpuViewModel()
		{
			this.State = new PpuState();
			this.WhenAnyValue(x => x.State).Select(st => st.Cycle.ToString()).ToPropertyEx(this, x => x.Cycle);
			this.WhenAnyValue(x => x.State).Select(st => st.Scanline.ToString()).ToPropertyEx(this, x => x.Scanline);
			this.WhenAnyValue(x => x.State).Select(st => st.HClock.ToString()).ToPropertyEx(this, x => x.HClock);
		}
	}
}
