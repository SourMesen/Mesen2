using Mesen.Interop;
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
			ConfigApi.SetInputConfig(new InteropInputConfig() {
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

		public virtual InteropKeyMapping ToInterop(ControllerType type)
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

	public class NesKeyMapping : KeyMapping
	{
		public UInt32[] PowerPadButtons = new UInt32[12];
		public UInt32[] FamilyBasicKeyboardButtons = new UInt32[72];
		public UInt32[] PartyTapButtons = new UInt32[6];
		public UInt32[] PachinkoButtons = new UInt32[2];
		public UInt32[] ExcitingBoxingButtons = new UInt32[8];
		public UInt32[] JissenMahjongButtons = new UInt32[21];
		public UInt32[] SuborKeyboardButtons = new UInt32[99];
		public UInt32[] BandaiMicrophoneButtons = new UInt32[3];
		public UInt32[] VirtualBoyButtons = new UInt32[14];
	}

	public class ControllerConfig : ReactiveObject
	{
		[Reactive] public KeyMapping Mapping1 { get; set; } = new KeyMapping();
		[Reactive] public KeyMapping Mapping2 { get; set; } = new KeyMapping();
		[Reactive] public KeyMapping Mapping3 { get; set; } = new KeyMapping();
		[Reactive] public KeyMapping Mapping4 { get; set; } = new KeyMapping();
		[Reactive] public UInt32 TurboSpeed { get; set; } = 0;
		[Reactive] public ControllerType Type { get; set; } = ControllerType.None;

		public InteropControllerConfig ToInterop()
		{
			return new InteropControllerConfig() {
				Type = this.Type,
				Keys = new InteropKeyMappingSet() {
					Mapping1 = this.Mapping1.ToInterop(Type),
					Mapping2 = this.Mapping2.ToInterop(Type),
					Mapping3 = this.Mapping3.ToInterop(Type),
					Mapping4 = this.Mapping4.ToInterop(Type),
					TurboSpeed = this.TurboSpeed
				}
			};
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropInputConfig
	{
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
		
		public UInt32 Microphone;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 100)]
		public UInt32[] CustomKeys;
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
		None,

		//SNES controllers
		SnesController,
		SnesMouse,
		SuperScope,
		Multitap,

		//NES controllers
		NesController,
		FamicomController,
		NesZapper,
		NesArkanoidController,
		PowerPad,
		SuborMouse,
		VsZapper,
		VbController,

		//NES/Famicon expansion devices
		FourScore,
		FamicomZapper,
		FourPlayerAdapter,
		FamicomArkanoidController,
		OekaKidsTablet,
		FamilyTrainerMat,
		KonamiHyperShot,
		FamilyBasicKeyboard,
		PartyTap,
		Pachinko,
		ExcitingBoxing,
		JissenMahjong,
		SuborKeyboard,
		BarcodeBattler,
		HoriTrack,
		BandaiHyperShot,
		AsciiTurboFile,
		BattleBox,

		//Game Boy
		GameboyController,
	}

	public static class ControllerTypeExtensions
	{
		public static bool CanConfigure(this ControllerType type)
		{
			switch(type) {
				case ControllerType.SnesController:
				case ControllerType.NesController:
					return true;
			}

			return false;
		}
	}

	public enum InputDisplayPosition
	{
		TopLeft = 0,
		TopRight = 1,
		BottomLeft = 2,
		BottomRight = 3
	}
}
