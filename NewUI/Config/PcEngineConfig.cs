using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class PcEngineConfig : BaseConfig<PcEngineConfig>
	{
		[Reactive] public ControllerConfig Port1 { get; set; } = new();
		
		[Reactive] public ControllerConfig Port1A { get; set; } = new();
		[Reactive] public ControllerConfig Port1B { get; set; } = new();
		[Reactive] public ControllerConfig Port1C { get; set; } = new();
		[Reactive] public ControllerConfig Port1D { get; set; } = new();
		[Reactive] public ControllerConfig Port1E { get; set; } = new();

		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;
		[Reactive] public bool PreventSelectRunReset { get; set; } = true;

		[Reactive] public UInt32 Channel1Vol { get; set; } = 100;
		[Reactive] public UInt32 Channel2Vol { get; set; } = 100;
		[Reactive] public UInt32 Channel3Vol { get; set; } = 100;
		[Reactive] public UInt32 Channel4Vol { get; set; } = 100;
		[Reactive] public UInt32 Channel5Vol { get; set; } = 100;
		[Reactive] public UInt32 Channel6Vol { get; set; } = 100;

		public void ApplyConfig()
		{
			ConfigApi.SetPcEngineConfig(new InteropPcEngineConfig() {
				Port1 = Port1.ToInterop(),
				Port1A = Port1A.ToInterop(),
				Port1B = Port1B.ToInterop(),
				Port1C = Port1C.ToInterop(),
				Port1D = Port1D.ToInterop(),
				Port1E = Port1E.ToInterop(),

				RamPowerOnState = RamPowerOnState,
				PreventSelectRunReset = PreventSelectRunReset,

				Channel1Vol = Channel1Vol,
				Channel2Vol = Channel2Vol,
				Channel3Vol = Channel3Vol,
				Channel4Vol = Channel4Vol,
				Channel5Vol = Channel5Vol,
				Channel6Vol = Channel6Vol,
			});
		}

		internal void InitializeDefaults(DefaultKeyMappingType defaultMappings)
		{
			List<KeyMapping> mappings = new List<KeyMapping>();
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyXboxLayout(mapping, 0, ControllerType.PceController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyPs4Layout(mapping, 0, ControllerType.PceController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyWasdLayout(mapping, ControllerType.PceController);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
				KeyMapping mapping = new();
				KeyPresets.ApplyArrowLayout(mapping, ControllerType.PceController);
				mappings.Add(mapping);
			}

			Port1.Type = ControllerType.PceController;
			Port1.TurboSpeed = 2;
			if(mappings.Count > 0) {
				Port1.Mapping1 = mappings[0];
				if(mappings.Count > 1) {
					Port1.Mapping2 = mappings[1];
					if(mappings.Count > 2) {
						Port1.Mapping3 = mappings[2];
						if(mappings.Count > 3) {
							Port1.Mapping4 = mappings[3];
						}
					}
				}
			}
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropPcEngineConfig
	{
		public InteropControllerConfig Port1;

		public InteropControllerConfig Port1A;
		public InteropControllerConfig Port1B;
		public InteropControllerConfig Port1C;
		public InteropControllerConfig Port1D;
		public InteropControllerConfig Port1E;

		public RamState RamPowerOnState;
		[MarshalAs(UnmanagedType.I1)] public bool PreventSelectRunReset;

		public UInt32 Channel1Vol;
		public UInt32 Channel2Vol;
		public UInt32 Channel3Vol;
		public UInt32 Channel4Vol;
		public UInt32 Channel5Vol;
		public UInt32 Channel6Vol;
	}
}
