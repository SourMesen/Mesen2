using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class EmulationConfig : BaseConfig<EmulationConfig>
	{
		[MinMax(0, 5000)] public UInt32 EmulationSpeed = 100;
		[MinMax(0, 5000)] public UInt32 TurboSpeed = 300;
		[MinMax(0, 5000)] public UInt32 RewindSpeed = 100;

		[MarshalAs(UnmanagedType.I1)] public bool AllowInvalidInput = false;
		[MarshalAs(UnmanagedType.I1)] public bool EnableMapperRandomPowerOnState = false;

		[MinMax(0, 1000)] public UInt32 PpuExtraScanlinesBeforeNmi = 0;
		[MinMax(0, 1000)] public UInt32 PpuExtraScanlinesAfterNmi = 0;

		public RamPowerOnState RamPowerOnState;
		
		public void ApplyConfig()
		{
			ConfigApi.SetEmulationConfig(this);
		}
	}

	public enum RamPowerOnState
	{
		AllZeros = 0,
		AllOnes = 1,
		Random = 2
	}
}
