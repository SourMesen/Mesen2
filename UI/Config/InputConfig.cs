using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class InputConfig
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
		public ControllerConfig[] Controllers = new ControllerConfig[5];

		[MinMax(0, 4)] public UInt32 ControllerDeadzoneSize = 2;
		[MinMax(0, 3)] public UInt32 MouseSensitivity = 1;

		public InputConfig()
		{
		}

		public InputConfig Clone()
		{
			InputConfig cfg = (InputConfig)this.MemberwiseClone();
			cfg.Controllers = new ControllerConfig[5];
			for(int i = 0; i < 5; i++) {
				cfg.Controllers[i] = Controllers[i];
			}
			return cfg;
		}

		public void ApplyConfig()
		{
			if(Controllers.Length != 5) {
				Controllers = new ControllerConfig[5];
			}
			ConfigApi.SetInputConfig(this);
		}

		public void InitializeDefaults(DefaultKeyMappingType defaultMappings)
		{
			KeyPresets presets = new KeyPresets();
			List<KeyMapping> mappings = new List<KeyMapping>();
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
				mappings.Add(presets.XboxLayout1);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
				mappings.Add(presets.Ps4Layout1);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
				mappings.Add(presets.WasdLayout);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
				mappings.Add(presets.ArrowLayout);
			}
			
			Controllers[0].Type = ControllerType.SnesController;
			Controllers[0].Keys.TurboSpeed = 2;
			if(mappings.Count > 0) {
				Controllers[0].Keys.Mapping1 = mappings[0];
				if(mappings.Count > 1) {
					Controllers[0].Keys.Mapping2 = mappings[1];
					if(mappings.Count > 2) {
						Controllers[0].Keys.Mapping3 = mappings[2];
						if(mappings.Count > 3) {
							Controllers[0].Keys.Mapping4 = mappings[3];
						}
					}
				}
			}
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct KeyMapping
	{
		public UInt32 A;
		public UInt32 B;
		public UInt32 X;
		public UInt32 Y;
		public UInt32 L;
		public UInt32 R;
		public UInt32 Up;
		public UInt32 Down;
		public UInt32 Left;
		public UInt32 Right;
		public UInt32 Start;
		public UInt32 Select;

		public UInt32 TurboA;
		public UInt32 TurboB;
		public UInt32 TurboX;
		public UInt32 TurboY;
		public UInt32 TurboL;
		public UInt32 TurboR;
		public UInt32 TurboSelect;
		public UInt32 TurboStart;

		public KeyMapping Clone()
		{
			return (KeyMapping)this.MemberwiseClone();
		}
	}

	public struct KeyMappingSet
	{
		public KeyMapping Mapping1;
		public KeyMapping Mapping2;
		public KeyMapping Mapping3;
		public KeyMapping Mapping4;
		public UInt32 TurboSpeed;
	}

	public struct ControllerConfig
	{
		public KeyMappingSet Keys;
		public ControllerType Type;
	}

	public enum ControllerType
	{
		None = 0,
		SnesController = 1,
		SnesMouse = 2,
		SuperScope = 3
	}
}
