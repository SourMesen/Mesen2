using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class EmulationConfig : BaseConfig<EmulationConfig>
	{
		[Reactive] [MinMax(0, 5000)] public UInt32 EmulationSpeed { get; set; } = 100;
		[Reactive] [MinMax(0, 5000)] public UInt32 TurboSpeed { get; set; } = 300;
		[Reactive] [MinMax(0, 5000)] public UInt32 RewindSpeed { get; set; } = 100;

		[Reactive] [MinMax(0, 10)] public UInt32 RunAheadFrames { get; set; } = 0;
		
		public void ApplyConfig()
		{
			ConfigApi.SetEmulationConfig(new InteropEmulationConfig() {
				EmulationSpeed = this.EmulationSpeed,
				TurboSpeed = this.TurboSpeed,
				RewindSpeed = this.RewindSpeed,
				RunAheadFrames = this.RunAheadFrames
			});
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropEmulationConfig
	{
		public UInt32 EmulationSpeed;
		public UInt32 TurboSpeed;
		public UInt32 RewindSpeed;

		public UInt32 RunAheadFrames;
	}

	public enum ConsoleRegion
	{
		Auto = 0,
		Ntsc = 1,
		Pal = 2,
		Dendy = 3,
		NtscJapan = 4
	}

	public enum RamState
	{
		Random = 0,
		AllZeros = 1,
		AllOnes = 2,
	}
}
