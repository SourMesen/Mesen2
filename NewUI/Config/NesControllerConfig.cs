using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Config
{
	public class NesControllerConfig : ControllerConfig
	{
		public new NesKeyMapping Mapping1 { get => (NesKeyMapping)_mapping1; set => _mapping1 = value; }
		public new NesKeyMapping Mapping2 { get => (NesKeyMapping)_mapping2; set => _mapping2 = value; }
		public new NesKeyMapping Mapping3 { get => (NesKeyMapping)_mapping3; set => _mapping3 = value; }
		public new NesKeyMapping Mapping4 { get => (NesKeyMapping)_mapping4; set => _mapping4 = value; }

		public NesControllerConfig()
		{
			_mapping1 = new NesKeyMapping();
			_mapping2 = new NesKeyMapping();
			_mapping3 = new NesKeyMapping();
			_mapping4 = new NesKeyMapping();
		}
	}

	public class NesKeyMapping : KeyMapping
	{
		public UInt32[]? PowerPadButtons = null;
		public UInt32[]? FamilyBasicKeyboardButtons = null;
		public UInt32[]? PartyTapButtons = null;
		public UInt32[]? PachinkoButtons = null;
		public UInt32[]? ExcitingBoxingButtons = null;
		public UInt32[]? JissenMahjongButtons = null;
		public UInt32[]? SuborKeyboardButtons = null;
		public UInt32[]? BandaiMicrophoneButtons = null;
		public UInt32[]? VirtualBoyButtons = null;

		private UInt32[]? GetCustomButtons(ControllerType type)
		{
			return type switch {
				ControllerType.PowerPad => PowerPadButtons,
				ControllerType.FamilyTrainerMat => PowerPadButtons,
				ControllerType.FamilyBasicKeyboard => FamilyBasicKeyboardButtons,
				ControllerType.PartyTap => PartyTapButtons,
				ControllerType.Pachinko => PachinkoButtons,
				ControllerType.ExcitingBoxing => ExcitingBoxingButtons,
				ControllerType.JissenMahjong => JissenMahjongButtons,
				ControllerType.SuborKeyboard => SuborKeyboardButtons,
				ControllerType.VbController => VirtualBoyButtons,
				_ => null
			};
		}

		public override InteropKeyMapping ToInterop(ControllerType type)
		{
			InteropKeyMapping mappings = base.ToInterop(type);
			UInt32[]? customKeys = type switch {
				ControllerType.PowerPad => PowerPadButtons,
				ControllerType.FamilyTrainerMat => PowerPadButtons,
				ControllerType.FamilyBasicKeyboard => FamilyBasicKeyboardButtons,
				ControllerType.PartyTap => PartyTapButtons,
				ControllerType.Pachinko => PachinkoButtons,
				ControllerType.ExcitingBoxing => ExcitingBoxingButtons,
				ControllerType.JissenMahjong => JissenMahjongButtons,
				ControllerType.SuborKeyboard => SuborKeyboardButtons,
				ControllerType.VbController => VirtualBoyButtons,
				_ => null
			};

			if(customKeys != null) {
				mappings.CustomKeys = new UInt32[100];
				for(int i = 0; i < customKeys.Length; i++) {
					mappings.CustomKeys[i] = customKeys[i];
				}
			}

			return mappings;
		}

		public override List<CustomKeyMapping> ToCustomKeys(ControllerType type)
		{
			UInt32[]? buttonMappings = GetCustomButtons(type);
			if(buttonMappings == null) {
				SetDefaultKeys(type, null);
				buttonMappings = GetCustomButtons(type);
				if(buttonMappings == null) {
					return new List<CustomKeyMapping>();
				}
			}

			List<CustomKeyMapping> keys = type switch {
				ControllerType.PowerPad or ControllerType.FamilyTrainerMat => Enum.GetValues<NesPowerPadButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.FamilyBasicKeyboard => Enum.GetValues<NesFamilyBasicKeyboardButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.PartyTap => Enum.GetValues<NesPartyTapButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.Pachinko => Enum.GetValues<NesPachinkoButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.ExcitingBoxing => Enum.GetValues<NesExcitingBoxingButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.JissenMahjong => Enum.GetValues<NesJissenMahjongButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.SuborKeyboard => Enum.GetValues<NesSuborKeyboardButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				ControllerType.VbController => Enum.GetValues<NesVirtualBoyButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				_ => new()
			};

			keys.Sort((a, b) => a.Name.CompareTo(b.Name));

			return keys;
		}

		public override void ClearKeys(ControllerType type)
		{
			switch(type) {
				case ControllerType.FamilyBasicKeyboard:
					FamilyBasicKeyboardButtons = new UInt32[72];
					break;

				case ControllerType.PowerPad:
				case ControllerType.FamilyTrainerMat:
					PowerPadButtons = new UInt32[12];
					break;

				case ControllerType.PartyTap:
					PartyTapButtons = new UInt32[6];
					break;

				case ControllerType.Pachinko:
					PachinkoButtons = new UInt32[2];
					break;

				case ControllerType.ExcitingBoxing:
					ExcitingBoxingButtons = new UInt32[8];
					break;

				case ControllerType.JissenMahjong:
					JissenMahjongButtons = new UInt32[21];
					break;

				case ControllerType.SuborKeyboard:
					SuborKeyboardButtons = new UInt32[99];
					break;

				case ControllerType.VbController:
					VirtualBoyButtons = new UInt32[14];
					break;

				case ControllerType.SnesController:
				case ControllerType.NesController:
					base.ClearKeys(type);
					break;
			}
		}

		public override void SetDefaultKeys(ControllerType type, KeyPresetType? preset)
		{
			switch(type) {
				case ControllerType.FamilyBasicKeyboard:
					FamilyBasicKeyboardButtons = new UInt32[72] {
						InputApi.GetKeyCode("A"), InputApi.GetKeyCode("B"), InputApi.GetKeyCode("C"), InputApi.GetKeyCode("D"),
						InputApi.GetKeyCode("E"), InputApi.GetKeyCode("F"), InputApi.GetKeyCode("G"), InputApi.GetKeyCode("H"),
						InputApi.GetKeyCode("I"), InputApi.GetKeyCode("J"), InputApi.GetKeyCode("K"), InputApi.GetKeyCode("L"),
						InputApi.GetKeyCode("M"), InputApi.GetKeyCode("N"), InputApi.GetKeyCode("O"), InputApi.GetKeyCode("P"),
						InputApi.GetKeyCode("Q"), InputApi.GetKeyCode("R"), InputApi.GetKeyCode("S"), InputApi.GetKeyCode("T"),
						InputApi.GetKeyCode("U"), InputApi.GetKeyCode("V"), InputApi.GetKeyCode("W"), InputApi.GetKeyCode("X"),
						InputApi.GetKeyCode("Y"), InputApi.GetKeyCode("Z"), InputApi.GetKeyCode("0"), InputApi.GetKeyCode("1"),
						InputApi.GetKeyCode("2"), InputApi.GetKeyCode("3"), InputApi.GetKeyCode("4"), InputApi.GetKeyCode("5"),
						InputApi.GetKeyCode("6"), InputApi.GetKeyCode("7"), InputApi.GetKeyCode("8"), InputApi.GetKeyCode("9"),
						InputApi.GetKeyCode("Enter"), InputApi.GetKeyCode("Space"), InputApi.GetKeyCode("Backspace"), InputApi.GetKeyCode("Insert"),
						InputApi.GetKeyCode("Esc"), InputApi.GetKeyCode("Left Ctrl"), InputApi.GetKeyCode("Right Shift"), InputApi.GetKeyCode("Left Shift"),
						InputApi.GetKeyCode("]"), InputApi.GetKeyCode("["),
						InputApi.GetKeyCode("Up Arrow"), InputApi.GetKeyCode("Down Arrow"), InputApi.GetKeyCode("Left Arrow"), InputApi.GetKeyCode("Right Arrow"),
						InputApi.GetKeyCode("."), InputApi.GetKeyCode(","), InputApi.GetKeyCode("'"), InputApi.GetKeyCode(";"),
						InputApi.GetKeyCode("="), InputApi.GetKeyCode("/"), InputApi.GetKeyCode("-"), InputApi.GetKeyCode("`"),
						InputApi.GetKeyCode("F1"), InputApi.GetKeyCode("F2"), InputApi.GetKeyCode("F3"), InputApi.GetKeyCode("F4"),
						InputApi.GetKeyCode("F5"), InputApi.GetKeyCode("F6"), InputApi.GetKeyCode("F7"), InputApi.GetKeyCode("F8"),
						InputApi.GetKeyCode("\\"), InputApi.GetKeyCode("Delete"), InputApi.GetKeyCode("F9"), InputApi.GetKeyCode("Left Alt"),
						InputApi.GetKeyCode("Home"), InputApi.GetKeyCode("End")
					};
					break;

				case ControllerType.PowerPad:
				case ControllerType.FamilyTrainerMat:
					PowerPadButtons = new UInt32[12] {
						InputApi.GetKeyCode("R"),
						InputApi.GetKeyCode("T"),
						InputApi.GetKeyCode("Y"),
						InputApi.GetKeyCode("U"),
						InputApi.GetKeyCode("F"),
						InputApi.GetKeyCode("G"),
						InputApi.GetKeyCode("H"),
						InputApi.GetKeyCode("J"),
						InputApi.GetKeyCode("V"),
						InputApi.GetKeyCode("B"),
						InputApi.GetKeyCode("N"),
						InputApi.GetKeyCode("M"),
					};
					break;

				case ControllerType.PartyTap:
					PartyTapButtons = new UInt32[6] {
						InputApi.GetKeyCode("1"),
						InputApi.GetKeyCode("2"),
						InputApi.GetKeyCode("3"),
						InputApi.GetKeyCode("4"),
						InputApi.GetKeyCode("5"),
						InputApi.GetKeyCode("6"),
					};
					break;

				case ControllerType.Pachinko:
					PachinkoButtons = new UInt32[2] {
						InputApi.GetKeyCode("R"),
						InputApi.GetKeyCode("F")
					};
					break;

				case ControllerType.ExcitingBoxing:
					ExcitingBoxingButtons = new UInt32[8] {
						InputApi.GetKeyCode("Numpad 7"),
						InputApi.GetKeyCode("Numpad 6"),
						InputApi.GetKeyCode("Numpad 4"),
						InputApi.GetKeyCode("Numpad 9"),
						InputApi.GetKeyCode("Numpad 1"),
						InputApi.GetKeyCode("Numpad 5"),
						InputApi.GetKeyCode("Numpad 3"),
						InputApi.GetKeyCode("Numpad 8"),
					};
					break;

				case ControllerType.JissenMahjong:
					JissenMahjongButtons = new UInt32[21] {
						InputApi.GetKeyCode("A"),
						InputApi.GetKeyCode("B"),
						InputApi.GetKeyCode("C"),
						InputApi.GetKeyCode("D"),
						InputApi.GetKeyCode("E"),
						InputApi.GetKeyCode("F"),
						InputApi.GetKeyCode("G"),
						InputApi.GetKeyCode("H"),
						InputApi.GetKeyCode("I"),
						InputApi.GetKeyCode("J"),
						InputApi.GetKeyCode("K"),
						InputApi.GetKeyCode("L"),
						InputApi.GetKeyCode("M"),
						InputApi.GetKeyCode("N"),
						InputApi.GetKeyCode("Space"),
						InputApi.GetKeyCode("Enter"),
						InputApi.GetKeyCode("1"),
						InputApi.GetKeyCode("2"),
						InputApi.GetKeyCode("3"),
						InputApi.GetKeyCode("4"),
						InputApi.GetKeyCode("5"),
					};
					break;

				case ControllerType.SuborKeyboard:
					SuborKeyboardButtons = new UInt32[99] {
						InputApi.GetKeyCode("A"), InputApi.GetKeyCode("B"), InputApi.GetKeyCode("C"), InputApi.GetKeyCode("D"),
						InputApi.GetKeyCode("E"), InputApi.GetKeyCode("F"), InputApi.GetKeyCode("G"), InputApi.GetKeyCode("H"),
						InputApi.GetKeyCode("I"), InputApi.GetKeyCode("J"), InputApi.GetKeyCode("K"), InputApi.GetKeyCode("L"),
						InputApi.GetKeyCode("M"), InputApi.GetKeyCode("N"), InputApi.GetKeyCode("O"), InputApi.GetKeyCode("P"),
						InputApi.GetKeyCode("Q"), InputApi.GetKeyCode("R"), InputApi.GetKeyCode("S"), InputApi.GetKeyCode("T"),
						InputApi.GetKeyCode("U"), InputApi.GetKeyCode("V"), InputApi.GetKeyCode("W"), InputApi.GetKeyCode("X"),
						InputApi.GetKeyCode("Y"), InputApi.GetKeyCode("Z"), InputApi.GetKeyCode("0"), InputApi.GetKeyCode("1"),
						InputApi.GetKeyCode("2"), InputApi.GetKeyCode("3"), InputApi.GetKeyCode("4"), InputApi.GetKeyCode("5"),
						InputApi.GetKeyCode("6"), InputApi.GetKeyCode("7"), InputApi.GetKeyCode("8"), InputApi.GetKeyCode("9"),
						InputApi.GetKeyCode("F1"), InputApi.GetKeyCode("F2"), InputApi.GetKeyCode("F3"), InputApi.GetKeyCode("F4"),
						InputApi.GetKeyCode("F5"), InputApi.GetKeyCode("F6"), InputApi.GetKeyCode("F7"), InputApi.GetKeyCode("F8"),
						InputApi.GetKeyCode("F9"), InputApi.GetKeyCode("F10"), InputApi.GetKeyCode("F11"), InputApi.GetKeyCode("F12"),

						InputApi.GetKeyCode("Numpad 0"), InputApi.GetKeyCode("Numpad 1"), InputApi.GetKeyCode("Numpad 2"), InputApi.GetKeyCode("Numpad 3"),
						InputApi.GetKeyCode("Numpad 4"), InputApi.GetKeyCode("Numpad 5"), InputApi.GetKeyCode("Numpad 6"), InputApi.GetKeyCode("Numpad 7"),
						InputApi.GetKeyCode("Numpad 8"), InputApi.GetKeyCode("Numpad 9"),

						0, InputApi.GetKeyCode("Numpad ."),
						InputApi.GetKeyCode("Numpad +"), InputApi.GetKeyCode("Numpad *"),
						InputApi.GetKeyCode("Numpad /"), InputApi.GetKeyCode("Numpad -"),

						InputApi.GetKeyCode("Num Lock"),

						InputApi.GetKeyCode(","), InputApi.GetKeyCode("."), InputApi.GetKeyCode(";"), InputApi.GetKeyCode("'"),
						InputApi.GetKeyCode("/"), InputApi.GetKeyCode("\\"),
						InputApi.GetKeyCode("="), InputApi.GetKeyCode("-"), InputApi.GetKeyCode("`"),

						InputApi.GetKeyCode("["), InputApi.GetKeyCode("]"),

						InputApi.GetKeyCode("Caps Lock"), InputApi.GetKeyCode("Pause"),

						InputApi.GetKeyCode("Left Ctrl"), InputApi.GetKeyCode("Left Shift"), InputApi.GetKeyCode("Left Alt"),

						InputApi.GetKeyCode("Space"), InputApi.GetKeyCode("Backspace"), InputApi.GetKeyCode("Tab"), InputApi.GetKeyCode("Esc"), InputApi.GetKeyCode("Enter"),

						InputApi.GetKeyCode("End"), InputApi.GetKeyCode("Home"),
						InputApi.GetKeyCode("Insert"), InputApi.GetKeyCode("Delete"),

						InputApi.GetKeyCode("Page Up"), InputApi.GetKeyCode("Page Down"),

						InputApi.GetKeyCode("Up Arrow"), InputApi.GetKeyCode("Down Arrow"), InputApi.GetKeyCode("Left Arrow"), InputApi.GetKeyCode("Right Arrow"),

						0,0,0
					};
					break;

				case ControllerType.VbController:
					VirtualBoyButtons = new UInt32[14] {
						InputApi.GetKeyCode("K"),
						InputApi.GetKeyCode("J"),
						InputApi.GetKeyCode("E"),
						InputApi.GetKeyCode("R"),
						InputApi.GetKeyCode("W"),
						InputApi.GetKeyCode("S"),
						InputApi.GetKeyCode("A"),
						InputApi.GetKeyCode("D"),
						InputApi.GetKeyCode("L"),
						InputApi.GetKeyCode("I"),
						InputApi.GetKeyCode("Q"),
						InputApi.GetKeyCode("O"),
						InputApi.GetKeyCode("Y"),
						InputApi.GetKeyCode("U"),
					};
					break;

				default:
					base.SetDefaultKeys(type, preset);
					break;
			}
		}
	}

	public enum NesPowerPadButtons { B1 = 0, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12 }
	public enum NesExcitingBoxingButtons { LeftHook = 0, MoveRight, MoveLeft, RightHook, LeftJab, HitBody, RightJab, Straight }
	public enum NesVirtualBoyButtons { Down1 = 0, Left1, Select, Start, Up0, Down0, Left0, Right0, Right1, Up1, L, R, B, A };
	public enum NesPartyTapButtons { B1 = 0, B2, B3, B4, B5, B6 };
	public enum NesPachinkoButtons { Press, Release };
	public enum NesJissenMahjongButtons { A = 0, B, C, D, E, F, G, H, I, J, K, L, M, N, Select, Start, Kan, Pon, Chii, Riichi, Ron };

	public enum NesFamilyBasicKeyboardButtons
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		Return, Space, Del, Ins, Esc,
		Ctrl, RightShift, LeftShift,
		RightBracket, LeftBracket,
		Up, Down, Left, Right,
		Dot, Comma, Colon, SemiColon, Underscore, Slash, Minus, Caret,
		F1, F2, F3, F4, F5, F6, F7, F8,
		Yen, Stop, AtSign, Grph, ClrHome, Kana
	};

	public enum NesSuborKeyboardButtons
	{
		A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		Numpad0, Numpad1, Numpad2, Numpad3, Numpad4, Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
		NumpadEnter, NumpadDot, NumpadPlus, NumpadMultiply, NumpadDivide, NumpadMinus, NumLock,
		Comma, Dot, SemiColon, Apostrophe,
		Slash, Backslash,
		Equal, Minus, Grave,
		LeftBracket, RightBracket,
		CapsLock, Pause,
		Ctrl, Shift, Alt,
		Space, Backspace, Tab, Esc, Enter,
		End, Home,
		Ins, Delete,
		PageUp, PageDown,
		Up, Down, Left, Right,
		Unknown1, Unknown2, Unknown3,
	};
}
