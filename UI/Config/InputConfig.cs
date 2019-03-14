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
				InitializeDefaults();
			}
			ConfigApi.SetInputConfig(this);
		}

		public void InitializeDefaults()
		{
			KeyMapping m1 = new KeyMapping();
			m1.Up = InputApi.GetKeyCode("Pad1 Up");
			m1.Down = InputApi.GetKeyCode("Pad1 Down");
			m1.Left = InputApi.GetKeyCode("Pad1 Left");
			m1.Right = InputApi.GetKeyCode("Pad1 Right");
			m1.A = InputApi.GetKeyCode("Pad1 B");
			m1.B = InputApi.GetKeyCode("Pad1 A");
			m1.X = InputApi.GetKeyCode("Pad1 Y");
			m1.Y = InputApi.GetKeyCode("Pad1 X");
			m1.L = InputApi.GetKeyCode("Pad1 L1");
			m1.R = InputApi.GetKeyCode("Pad1 R1");
			m1.Select = InputApi.GetKeyCode("Pad1 Back");
			m1.Start = InputApi.GetKeyCode("Pad1 Start");

			KeyMapping m2 = new KeyMapping();
			m2.Up = InputApi.GetKeyCode("Up Arrow");
			m2.Down = InputApi.GetKeyCode("Down Arrow");
			m2.Left = InputApi.GetKeyCode("Left Arrow");
			m2.Right = InputApi.GetKeyCode("Right Arrow");
			m2.A = InputApi.GetKeyCode("Z");
			m2.B = InputApi.GetKeyCode("X");
			m2.X = InputApi.GetKeyCode("S");
			m2.Y = InputApi.GetKeyCode("A");
			m2.L = InputApi.GetKeyCode("Q");
			m2.R = InputApi.GetKeyCode("W");
			m2.Select = InputApi.GetKeyCode("E");
			m2.Start = InputApi.GetKeyCode("D");

			Controllers[0].Type = ControllerType.SnesController;
			Controllers[0].Keys.TurboSpeed = 2;
			Controllers[0].Keys.Mapping1 = m1;
			Controllers[0].Keys.Mapping2 = m2;
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
