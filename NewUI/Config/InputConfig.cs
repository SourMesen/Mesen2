using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class InputConfig : BaseConfig<InputConfig>
	{
		[Reactive] public List<ControllerConfig> Controllers { get; set; } = new List<ControllerConfig> { new ControllerConfig(), new ControllerConfig(), new ControllerConfig(), new ControllerConfig(), new ControllerConfig() };

		[Reactive] [MinMax(0, 4)] public UInt32 ControllerDeadzoneSize { get; set; } = 2;
		[Reactive] [MinMax(0, 3)] public UInt32 MouseSensitivity { get; set; } = 1;

		[Reactive] public InputDisplayPosition DisplayInputPosition { get; set; } = InputDisplayPosition.BottomRight;
		[Reactive] public bool DisplayInputPort1 { get; set; } = false;
		[Reactive] public bool DisplayInputPort2 { get; set; } = false;
		[Reactive] public bool DisplayInputPort3 { get; set; } = false;
		[Reactive] public bool DisplayInputPort4 { get; set; } = false;
		[Reactive] public bool DisplayInputPort5 { get; set; } = false;
		[Reactive] public bool DisplayInputHorizontally { get; set; } = true;

		public InputConfig()
		{
		}

		public void ApplyConfig()
		{
			while(Controllers.Count < 5) {
				Controllers.Add(new ControllerConfig());
			}

			ConfigApi.SetInputConfig(new InteropInputConfig() {
				Controllers = new InteropControllerConfig[5] {
					this.Controllers[0].ToInterop(),
					this.Controllers[1].ToInterop(),
					this.Controllers[2].ToInterop(),
					this.Controllers[3].ToInterop(),
					this.Controllers[4].ToInterop()
				},
				ControllerDeadzoneSize = this.ControllerDeadzoneSize,
				MouseSensitivity = this.MouseSensitivity,
				DisplayInputPosition = this.DisplayInputPosition,
				DisplayInputPort1 = this.DisplayInputPort1,
				DisplayInputPort2 = this.DisplayInputPort2,
				DisplayInputPort3 = this.DisplayInputPort3,
				DisplayInputPort4 = this.DisplayInputPort4,
				DisplayInputPort5 = this.DisplayInputPort5,
				DisplayInputHorizontally = this.DisplayInputHorizontally
			});
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

	public class KeyMapping : ReactiveObject
	{
		[Reactive] public UInt32 A { get; set; }
		[Reactive] public UInt32 B { get; set; }
		[Reactive] public UInt32 X { get; set; }
		[Reactive] public UInt32 Y { get; set; }
		[Reactive] public UInt32 L { get; set; }
		[Reactive] public UInt32 R { get; set; }
		[Reactive] public UInt32 Up { get; set; }
		[Reactive] public UInt32 Down { get; set; }
		[Reactive] public UInt32 Left { get; set; }
		[Reactive] public UInt32 Right { get; set; }
		[Reactive] public UInt32 Start { get; set; }
		[Reactive] public UInt32 Select { get; set; }

		[Reactive] public UInt32 TurboA { get; set; }
		[Reactive] public UInt32 TurboB { get; set; }
		[Reactive] public UInt32 TurboX { get; set; }
		[Reactive] public UInt32 TurboY { get; set; }
		[Reactive] public UInt32 TurboL { get; set; }
		[Reactive] public UInt32 TurboR { get; set; }
		[Reactive] public UInt32 TurboSelect { get; set; }
		[Reactive] public UInt32 TurboStart { get; set; }

		public InteropKeyMapping ToInterop()
		{
			return new InteropKeyMapping() {
				A = this.A,
				B = this.B,
				X = this.X,
				Y = this.Y,
				L = this.L,
				R = this.R,
				Up = this.Up,
				Down = this.Down,
				Left = this.Left,
				Right = this.Right,
				Select = this.Select,
				Start = this.Start,
				TurboA = this.TurboA,
				TurboB = this.TurboB,
				TurboX = this.TurboX,
				TurboY = this.TurboY,
				TurboL = this.TurboL,
				TurboR = this.TurboR,
				TurboSelect = this.TurboSelect,
				TurboStart = this.TurboStart
			};
		}
	}

	public class KeyMappingSet : ReactiveObject
	{
		[Reactive] public KeyMapping Mapping1 { get; set; } = new KeyMapping();
		[Reactive] public KeyMapping Mapping2 { get; set; } = new KeyMapping();
		[Reactive] public KeyMapping Mapping3 { get; set; } = new KeyMapping();
		[Reactive] public KeyMapping Mapping4 { get; set; } = new KeyMapping();
		[Reactive] public UInt32 TurboSpeed { get; set; } = 0;
	}

	public class ControllerConfig : ReactiveObject
	{
		[Reactive] public KeyMappingSet Keys { get; set; } = new KeyMappingSet();
		[Reactive] public ControllerType Type { get; set; } = ControllerType.None;

		public InteropControllerConfig ToInterop()
		{
			return new InteropControllerConfig() {
				Type = this.Type,
				Keys = new InteropKeyMappingSet() {
					Mapping1 = this.Keys.Mapping1.ToInterop(),
					Mapping2 = this.Keys.Mapping2.ToInterop(),
					Mapping3 = this.Keys.Mapping3.ToInterop(),
					Mapping4 = this.Keys.Mapping4.ToInterop(),
					TurboSpeed = this.Keys.TurboSpeed
				}
			};
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropInputConfig
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
		public InteropControllerConfig[] Controllers;

		public UInt32 ControllerDeadzoneSize;
		public UInt32 MouseSensitivity;

		public InputDisplayPosition DisplayInputPosition;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort1;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort2;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort3;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort4;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort5;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputHorizontally;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropKeyMapping
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
	}

	public struct InteropKeyMappingSet
	{
		public InteropKeyMapping Mapping1;
		public InteropKeyMapping Mapping2;
		public InteropKeyMapping Mapping3;
		public InteropKeyMapping Mapping4;
		public UInt32 TurboSpeed;
	}

	public struct InteropControllerConfig
	{
		public InteropKeyMappingSet Keys { get; set; }
		public ControllerType Type { get; set; }
	}

	public enum ControllerType
	{
		None = 0,
		SnesController = 1,
		SnesMouse = 2,
		SuperScope = 3,
		Multitap = 4
	}

	public enum InputDisplayPosition
	{
		TopLeft = 0,
		TopRight = 1,
		BottomLeft = 2,
		BottomRight = 3
	}
}
