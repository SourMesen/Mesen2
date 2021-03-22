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
	public class SnesCpuViewModel : ViewModelBase
	{
		[ObservableAsProperty] public string RegA { get; }
		[ObservableAsProperty] public string RegX { get; }
		[ObservableAsProperty] public string RegY { get; }
		[ObservableAsProperty] public string RegSP { get; }
		[ObservableAsProperty] public string RegD { get; }
		[ObservableAsProperty] public string RegPC { get; }
		[ObservableAsProperty] public string RegK { get; }
		[ObservableAsProperty] public string RegDBR { get; }
		[ObservableAsProperty] public string RegPS { get; }
		
		[ObservableAsProperty] public bool FlagN { get; }
		[ObservableAsProperty] public bool FlagV { get; }
		[ObservableAsProperty] public bool FlagM { get; }
		[ObservableAsProperty] public bool FlagX { get; }
		[ObservableAsProperty] public bool FlagD { get; }
		[ObservableAsProperty] public bool FlagI { get; }
		[ObservableAsProperty] public bool FlagZ { get; }
		[ObservableAsProperty] public bool FlagC { get; }
		[ObservableAsProperty] public bool FlagE { get; }
		[ObservableAsProperty] public bool FlagNmi { get; }
		[ObservableAsProperty] public bool FlagIrq { get; }

		[ObservableAsProperty] public string StackPreview { get; }

		private CpuState _state;
		public CpuState State
		{
			get => _state;
			set => this.RaiseAndSetIfChanged(ref _state, value);
		}

		public SnesCpuViewModel()
		{
			this.State = new CpuState();
			this.WhenAnyValue(x => x.State).Select(st => st.A.ToString("X4")).ToPropertyEx(this, x => x.RegA);
			this.WhenAnyValue(x => x.State).Select(st => st.X.ToString("X4")).ToPropertyEx(this, x => x.RegX);
			this.WhenAnyValue(x => x.State).Select(st => st.Y.ToString("X4")).ToPropertyEx(this, x => x.RegY);
			this.WhenAnyValue(x => x.State).Select(st => st.SP.ToString("X4")).ToPropertyEx(this, x => x.RegSP);
			this.WhenAnyValue(x => x.State).Select(st => st.D.ToString("X4")).ToPropertyEx(this, x => x.RegD);
			this.WhenAnyValue(x => x.State).Select(st => ((st.K << 16) | st.PC).ToString("X6")).ToPropertyEx(this, x => x.RegPC);
			this.WhenAnyValue(x => x.State).Select(st => st.K.ToString("X2")).ToPropertyEx(this, x => x.RegK);
			this.WhenAnyValue(x => x.State).Select(st => st.DBR.ToString("X2")).ToPropertyEx(this, x => x.RegDBR);
			this.WhenAnyValue(x => x.State).Select(st => ((int)st.PS).ToString("X2")).ToPropertyEx(this, x => x.RegPS);
			
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Negative)).ToPropertyEx(this, x => x.FlagN);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Overflow)).ToPropertyEx(this, x => x.FlagV);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.MemoryMode8)).ToPropertyEx(this, x => x.FlagM);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.IndexMode8)).ToPropertyEx(this, x => x.FlagX);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Decimal)).ToPropertyEx(this, x => x.FlagD);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.IrqDisable)).ToPropertyEx(this, x => x.FlagI);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Zero)).ToPropertyEx(this, x => x.FlagZ);
			this.WhenAnyValue(x => x.State).Select(st => st.PS.HasFlag(ProcFlags.Carry)).ToPropertyEx(this, x => x.FlagC);
			this.WhenAnyValue(x => x.State).Select(st => st.EmulationMode).ToPropertyEx(this, x => x.FlagE);
			this.WhenAnyValue(x => x.State).Select(st => st.NmiFlag).ToPropertyEx(this, x => x.FlagNmi);
			this.WhenAnyValue(x => x.State).Select(st => st.IrqSource != 0).ToPropertyEx(this, x => x.FlagIrq);

			this.WhenAnyValue(x => x.State).Select(st => {
				if(Design.IsDesignMode) {
					return "";
				}

				StringBuilder sb = new StringBuilder();
				for(UInt32 i = (uint)st.SP + 1; (i & 0xFF) != 0; i++) {
					sb.Append("$");
					sb.Append(DebugApi.GetMemoryValue(SnesMemoryType.CpuMemory, i).ToString("X2"));
					sb.Append(", ");
				}
				string stack = sb.ToString();
				if(stack.Length > 2) {
					stack = stack.Substring(0, stack.Length - 2);
				}
				return stack;
			}).ToPropertyEx(this, x => x.StackPreview);

			//_a = this.WhenAnyValue(x => x.State).Select(st => st.A).ToProperty(this, x => x.A);
			//_x = this.WhenAnyValue(x => x.State).Select(st => st.X).ToProperty(this, x => x.X);
		}
	}
}
