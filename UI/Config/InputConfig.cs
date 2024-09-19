using Mesen.Interop;
using Mesen.ViewModels;
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
	public class InputConfig : BaseConfig<InputConfig>
	{
		[Reactive] [MinMax(0, 4)] public UInt32 ControllerDeadzoneSize { get; set; } = 2;
		[Reactive] [MinMax(0, 9)] public UInt32 MouseSensitivity { get; set; } = 5;
		[Reactive] public bool HidePointerForLightGuns { get; set; } = false;
		[Reactive][MinMax(0, 10)] public UInt32 ForceFeedbackIntensity { get; set; } = 5;

		[Reactive] public InputDisplayPosition DisplayInputPosition { get; set; } = InputDisplayPosition.BottomRight;
		[Reactive] public bool DisplayInputPort1 { get; set; } = false;
		[Reactive] public bool DisplayInputPort2 { get; set; } = false;
		[Reactive] public bool DisplayInputPort3 { get; set; } = false;
		[Reactive] public bool DisplayInputPort4 { get; set; } = false;
		[Reactive] public bool DisplayInputPort5 { get; set; } = false;
		[Reactive] public bool DisplayInputPort6 { get; set; } = false;
		[Reactive] public bool DisplayInputPort7 { get; set; } = false;
		[Reactive] public bool DisplayInputPort8 { get; set; } = false;
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
				DisplayInputPort6 = this.DisplayInputPort6,
				DisplayInputPort7 = this.DisplayInputPort7,
				DisplayInputPort8 = this.DisplayInputPort8,
				DisplayInputHorizontally = this.DisplayInputHorizontally,
				ForceFeedbackIntensity = Math.Clamp(0.1 * this.ForceFeedbackIntensity, 0.0, 1.0)
			}); ;
		}
	}

	public class KeyMapping : ReactiveObject
	{
		[Reactive] public UInt16 A { get; set; }
		[Reactive] public UInt16 B { get; set; }
		[Reactive] public UInt16 X { get; set; }
		[Reactive] public UInt16 Y { get; set; }
		[Reactive] public UInt16 L { get; set; }
		[Reactive] public UInt16 R { get; set; }
		[Reactive] public UInt16 Up { get; set; }
		[Reactive] public UInt16 Down { get; set; }
		[Reactive] public UInt16 Left { get; set; }
		[Reactive] public UInt16 Right { get; set; }
		[Reactive] public UInt16 Start { get; set; }
		[Reactive] public UInt16 Select { get; set; }
		[Reactive] public UInt16 U { get; set; }
		[Reactive] public UInt16 D { get; set; }

		[Reactive] public UInt16 TurboA { get; set; }
		[Reactive] public UInt16 TurboB { get; set; }
		[Reactive] public UInt16 TurboX { get; set; }
		[Reactive] public UInt16 TurboY { get; set; }
		[Reactive] public UInt16 TurboL { get; set; }
		[Reactive] public UInt16 TurboR { get; set; }
		[Reactive] public UInt16 TurboSelect { get; set; }
		[Reactive] public UInt16 TurboStart { get; set; }

		[Reactive] public UInt16 GenericKey1 { get; set; }

		public virtual InteropKeyMapping ToInterop(ControllerType type, int mappingIndex)
		{
			InteropKeyMapping mappings = new InteropKeyMapping() {
				A = this.A,
				B = this.B,
				X = this.X,
				Y = this.Y,
				L = this.L,
				R = this.R,
				U = this.U,
				D = this.D,
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
				TurboStart = this.TurboStart,
				GenericKey1 = this.GenericKey1
			};

			UInt16[]? customKeys = GetCustomButtons(type);
			if(customKeys == null && mappingIndex == 0) {
				customKeys = GetDefaultCustomKeys(type, null);
			}

			if(customKeys != null) {
				mappings.CustomKeys = new UInt16[100];
				for(int i = 0; i < customKeys.Length; i++) {
					mappings.CustomKeys[i] = customKeys[i];
				}
			}

			return mappings;
		}

		protected virtual UInt16[]? GetCustomButtons(ControllerType type)
		{
			return null;
		}

		public virtual UInt16[]? GetDefaultCustomKeys(ControllerType type, KeyPresetType? preset)
		{
			return null;
		}

		public virtual List<CustomKeyMapping> ToCustomKeys(ControllerType type, int mappingIndex)
		{
			return new();
		}

		public virtual void SetDefaultKeys(ControllerType type, KeyPresetType? preset = null)
		{
			if(type.HasPresets()) {
				switch(preset) {
					case KeyPresetType.WasdKeys: KeyPresets.ApplyWasdLayout(this, type); break;
					case KeyPresetType.ArrowKeys: KeyPresets.ApplyArrowLayout(this, type); break;
					case KeyPresetType.XboxP1: KeyPresets.ApplyXboxLayout(this, 0, type, false); break;
					case KeyPresetType.XboxP1Alt: KeyPresets.ApplyXboxLayout(this, 0, type, true); break;
					case KeyPresetType.XboxP2: KeyPresets.ApplyXboxLayout(this, 1, type, false); break;
					case KeyPresetType.XboxP2Alt: KeyPresets.ApplyXboxLayout(this, 1, type, true); break;
					case KeyPresetType.Ps4P1: KeyPresets.ApplyPs4Layout(this, 0, type, false); break;
					case KeyPresetType.Ps4P1Alt: KeyPresets.ApplyPs4Layout(this, 0, type, true); break;
					case KeyPresetType.Ps4P2: KeyPresets.ApplyPs4Layout(this, 1, type, false); break;
					case KeyPresetType.Ps4P2Alt: KeyPresets.ApplyPs4Layout(this, 1, type, true); break;
				}
			}
		}

		public virtual void ClearKeys(ControllerType type)
		{
			A = 0;
			B = 0;
			X = 0;
			Y = 0;
			L = 0;
			R = 0;
			U = 0;
			D = 0;
			Up = 0;
			Down = 0;
			Left = 0;
			Right = 0;
			Select = 0;
			Start = 0;
			TurboA = 0;
			TurboB = 0;
			TurboX = 0;
			TurboY = 0;
			TurboL = 0;
			TurboR = 0;
			TurboSelect = 0;
			TurboStart = 0;
			GenericKey1 = 0;
		}
	}

	public enum KeyPresetType
	{
		XboxP1,
		XboxP1Alt,
		XboxP2,
		XboxP2Alt,
		Ps4P1,
		Ps4P1Alt,
		Ps4P2,
		Ps4P2Alt,
		WasdKeys,
		ArrowKeys
	}

	public class ControllerConfig : BaseConfig<ControllerConfig>
	{
		protected KeyMapping _mapping1 = new();
		protected KeyMapping _mapping2 = new();
		protected KeyMapping _mapping3 = new();
		protected KeyMapping _mapping4 = new();

		public KeyMapping Mapping1 { get => _mapping1; set => _mapping1 = value; }
		public KeyMapping Mapping2 { get => _mapping2; set => _mapping2 = value; }
		public KeyMapping Mapping3 { get => _mapping3; set => _mapping3 = value; }
		public KeyMapping Mapping4 { get => _mapping4; set => _mapping4 = value; }
		[Reactive] public UInt32 TurboSpeed { get; set; } = 0;
		[Reactive] public ControllerType Type { get; set; } = ControllerType.None;

		public void InitDefaults(DefaultKeyMappingType defaultMappings, ControllerType type)
		{
			InitDefaults<KeyMapping>(defaultMappings, type);
		}

		public void InitDefaults<T>(DefaultKeyMappingType defaultMappings, ControllerType type) where T : KeyMapping, new()
		{
			List<T> mappings = new List<T>();
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
				T mapping = new T();
				mapping.SetDefaultKeys(type, KeyPresetType.XboxP1);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
				T mapping = new T();
				mapping.SetDefaultKeys(type, KeyPresetType.Ps4P1);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
				T mapping = new T();
				mapping.SetDefaultKeys(type, KeyPresetType.WasdKeys);
				mappings.Add(mapping);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
				T mapping = new T();
				mapping.SetDefaultKeys(type, KeyPresetType.ArrowKeys);
				mappings.Add(mapping);
			}

			Type = type;
			TurboSpeed = 2;

			if(mappings.Count > 0) {
				Mapping1 = mappings[0];
				if(mappings.Count > 1) {
					Mapping2 = mappings[1];
					if(mappings.Count > 2) {
						Mapping3 = mappings[2];
						if(mappings.Count > 3) {
							Mapping4 = mappings[3];
						}
					}
				}
			}
		}

		public InteropControllerConfig ToInterop()
		{
			return new InteropControllerConfig() {
				Type = this.Type,
				Keys = new InteropKeyMappingSet() {
					Mapping1 = this.Mapping1.ToInterop(Type, 0),
					Mapping2 = this.Mapping2.ToInterop(Type, 1),
					Mapping3 = this.Mapping3.ToInterop(Type, 2),
					Mapping4 = this.Mapping4.ToInterop(Type, 3),
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
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort6;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort7;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputPort8;
		[MarshalAs(UnmanagedType.I1)] public bool DisplayInputHorizontally;

		public double ForceFeedbackIntensity;
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropKeyMapping
	{
		public UInt16 A;
		public UInt16 B;
		public UInt16 X;
		public UInt16 Y;
		public UInt16 L;
		public UInt16 R;
		public UInt16 Up;
		public UInt16 Down;
		public UInt16 Left;
		public UInt16 Right;
		public UInt16 Start;
		public UInt16 Select;
		public UInt16 U;
		public UInt16 D;

		public UInt16 TurboA;
		public UInt16 TurboB;
		public UInt16 TurboX;
		public UInt16 TurboY;
		public UInt16 TurboL;
		public UInt16 TurboR;
		public UInt16 TurboSelect;
		public UInt16 TurboStart;
		
		public UInt16 GenericKey1;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 100)]
		public UInt16[] CustomKeys;
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
		FamicomControllerP2,
		NesZapper,
		NesArkanoidController,
		PowerPadSideA,
		PowerPadSideB,
		SuborMouse,
		VbController,

		//NES/Famicon expansion devices
		FourScore,
		FamicomZapper,
		TwoPlayerAdapter,
		FourPlayerAdapter,
		FamicomArkanoidController,
		OekaKidsTablet,
		FamilyTrainerMatSideA,
		FamilyTrainerMatSideB,
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

		//NES cart input devices
		BandaiMicrophone,
		DatachBarcodeReader,

		//Game Boy
		GameboyController,
		GameboyAccelerometer,

		//PC Engine
		PceController,
		PceTurboTap,
		PceAvenuePad6,

		//SMS
		SmsController,
		SmsLightPhaser,
		ColecoVisionController,

		//GBA
		GbaController,

		//WonderSwan
		WsController,
		WsControllerVertical,
	}

	public static class ControllerTypeExtensions
	{
		public static bool HasPresets(this ControllerType type)
		{
			switch(type) {
				case ControllerType.SnesController:
				case ControllerType.NesController:
				case ControllerType.FamicomController:
				case ControllerType.FamicomControllerP2:
				case ControllerType.GameboyController:
				case ControllerType.GbaController:
				case ControllerType.PceController:
				case ControllerType.PceAvenuePad6:
				case ControllerType.HoriTrack:
				case ControllerType.BandaiHyperShot:
				case ControllerType.SmsController:
				case ControllerType.ColecoVisionController:
				case ControllerType.WsController:
				case ControllerType.WsControllerVertical:
					return true;
			}

			return false;
		}

		public static bool HasTurbo(this ControllerType type)
		{
			switch(type) {
				case ControllerType.SnesController:
				case ControllerType.NesController:
				case ControllerType.FamicomController:
				case ControllerType.FamicomControllerP2:
				case ControllerType.GameboyController:
				case ControllerType.GbaController:
				case ControllerType.PceController:
				case ControllerType.PceAvenuePad6:
				case ControllerType.Pachinko:
				case ControllerType.HoriTrack:
				case ControllerType.BandaiHyperShot:
				case ControllerType.SmsController:
					return true;
			}

			return false;
		}

		public static bool CanConfigure(this ControllerType type)
		{
			switch(type) {
				case ControllerType.SnesController:
				case ControllerType.NesController:
				case ControllerType.FamicomController:
				case ControllerType.FamicomControllerP2:
				case ControllerType.PowerPadSideA:
				case ControllerType.PowerPadSideB:
				case ControllerType.FamilyTrainerMatSideA:
				case ControllerType.FamilyTrainerMatSideB:
				case ControllerType.SuborKeyboard:
				case ControllerType.FamilyBasicKeyboard:
				case ControllerType.Pachinko:
				case ControllerType.PartyTap:
				case ControllerType.VbController:
				case ControllerType.JissenMahjong:
				case ControllerType.ExcitingBoxing:
				case ControllerType.GameboyController:
				case ControllerType.GbaController:
				case ControllerType.PceController:
				case ControllerType.PceAvenuePad6:
				case ControllerType.HoriTrack:
				case ControllerType.KonamiHyperShot:
				case ControllerType.BandaiHyperShot:
				case ControllerType.SuborMouse:
				case ControllerType.SnesMouse:
				case ControllerType.FamicomZapper:
				case ControllerType.NesZapper:
				case ControllerType.OekaKidsTablet:
				case ControllerType.FamicomArkanoidController:
				case ControllerType.NesArkanoidController:
				case ControllerType.SuperScope:
				case ControllerType.BandaiMicrophone:
				case ControllerType.SmsController:
				case ControllerType.SmsLightPhaser:
				case ControllerType.ColecoVisionController:
				case ControllerType.WsController:
				case ControllerType.WsControllerVertical:
					return true;
			}

			return false;
		}

		public static bool IsTwoButtonController(this ControllerType type)
		{
			switch(type) {
				case ControllerType.NesController:
				case ControllerType.FamicomController:
				case ControllerType.FamicomControllerP2:
				case ControllerType.GameboyController:
				case ControllerType.PceController:
				case ControllerType.HoriTrack:
				case ControllerType.BandaiHyperShot:
				case ControllerType.SmsController:
				case ControllerType.WsController:
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
