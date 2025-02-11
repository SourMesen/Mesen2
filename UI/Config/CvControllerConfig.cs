using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Config
{
	public class CvControllerConfig : ControllerConfig
	{
		public new CvKeyMapping Mapping1 { get => (CvKeyMapping)_mapping1; set => _mapping1 = value; }
		public new CvKeyMapping Mapping2 { get => (CvKeyMapping)_mapping2; set => _mapping2 = value; }
		public new CvKeyMapping Mapping3 { get => (CvKeyMapping)_mapping3; set => _mapping3 = value; }
		public new CvKeyMapping Mapping4 { get => (CvKeyMapping)_mapping4; set => _mapping4 = value; }

		public CvControllerConfig()
		{
			_mapping1 = new CvKeyMapping();
			_mapping2 = new CvKeyMapping();
			_mapping3 = new CvKeyMapping();
			_mapping4 = new CvKeyMapping();
		}
	}

	public class CvKeyMapping : KeyMapping
	{
		public UInt16[]? LightPhaserButtons { get; set; } = null;
		public UInt16[]? ColecoVisionControllerButtons { get; set; } = null;

		protected override UInt16[]? GetCustomButtons(ControllerType type)
		{
			return type switch {
				ControllerType.ColecoVisionController => ColecoVisionControllerButtons,
				_ => null
			};
		}

		public override List<CustomKeyMapping> ToCustomKeys(ControllerType type, int mappingIndex)
		{
			UInt16[]? buttonMappings = GetCustomButtons(type);
			if(buttonMappings == null) {
				if(GetDefaultCustomKeys(type, null) != null) {
					if(mappingIndex == 0) {
						SetDefaultKeys(type, null);
					} else {
						ClearKeys(type);
					}
				}

				buttonMappings = GetCustomButtons(type);
				if(buttonMappings == null) {
					return new List<CustomKeyMapping>();
				}
			}

			List<CustomKeyMapping> keys = type switch {
				ControllerType.ColecoVisionController => Enum.GetValues<ColecoVisionControllerButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				_ => new()
			};

			keys.Sort((a, b) => a.Name.CompareTo(b.Name));

			return keys;
		}

		public override void ClearKeys(ControllerType type)
		{
			switch(type) {
				case ControllerType.ColecoVisionController:
					ColecoVisionControllerButtons = new UInt16[18];
					break;
			}
		}

		public override UInt16[]? GetDefaultCustomKeys(ControllerType type, KeyPresetType? preset)
		{
			switch(type) {
				case ControllerType.ColecoVisionController:
					Dictionary<KeyPresetType, string[]> presets = new() {
						{ KeyPresetType.ArrowKeys, new string[] { "Up Arrow", "Down Arrow", "Left Arrow", "Right Arrow",
							"A", "S",
							"Numpad 1", "Numpad 2", "Numpad 3", "Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad 0",
							"Q", "W"
						} },
						{ KeyPresetType.WasdKeys, new string[] { "W", "D", "A", "S",
							"Q", "E",
							"Numpad 1", "Numpad 2", "Numpad 3", "Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad 0",
							"R", "F"
						} },
						{ KeyPresetType.XboxP1, new string[] { "Pad1 Up", "Pad1 Down", "Pad1 Left", "Pad1 Right",
							"Pad1 X", "Pad1 A",
							"Pad1 Back", "Pad1 Start",
							"Pad1 L1", "Pad1 L2", "Pad1 R1", "Pad1 R2",
							"Pad1 RT Up", "Pad1 RT Down", "Pad1 RT Left", "Pad1 RT Right",
							"Pad1 Y", "Pad1 B",
						} },
						{ KeyPresetType.XboxP2, new string[] { "Pad2 Up", "Pad2 Down", "Pad2 Left", "Pad2 Right",
							"Pad2 X", "Pad2 A",
							"Pad2 Back", "Pad2 Start",
							"Pad2 L1", "Pad2 L2", "Pad2 R1", "Pad2 R2",
							"Pad2 RT Up", "Pad2 RT Down", "Pad2 RT Left", "Pad2 RT Right",
							"Pad2 Y", "Pad2 B",
						} },
						{ KeyPresetType.Ps4P1, new string[] { "Joy1 DPad Up", "Joy1 DPad Down", "Joy1 DPad Left", "Joy1 DPad Right",
							"Joy1 But1", "Joy1 But2",
							"Joy1 But9", "Joy1 But10",
							"Joy1 But5", "Joy1 But7", "Joy1 But6", "Joy1 But8",
							"Joy1 Y2+", "Joy1 Y2-", "Joy1 X2+", "Joy1 X2-",
							"Joy1 But4", "Joy1 But3",
						} },
						{ KeyPresetType.Ps4P2, new string[] { "Joy2 DPad Up", "Joy2 DPad Down", "Joy2 DPad Left", "Joy2 DPad Right",
							"Joy2 But1", "Joy2 But2",
							"Joy2 But9", "Joy2 But10",
							"Joy2 But5", "Joy2 But7", "Joy2 But6", "Joy2 But8",
							"Joy2 Y2+", "Joy2 Y2-", "Joy2 X2+", "Joy2 X2-",
							"Joy2 But4", "Joy2 But3",
						} },
					};

					if(!presets.TryGetValue(preset ?? KeyPresetType.ArrowKeys, out string[]? keys)) {
						keys = presets[KeyPresetType.ArrowKeys];
					}

					return keys.Select(x => InputApi.GetKeyCode(x)).ToArray();

				default:
					return null;
			}
		}

		public override void SetDefaultKeys(ControllerType type, KeyPresetType? preset)
		{
			switch(type) {
				case ControllerType.ColecoVisionController: ColecoVisionControllerButtons = GetDefaultCustomKeys(type, preset); break;

				default:
					base.SetDefaultKeys(type, preset);
					break;
			}
		}
	}

	public enum ColecoVisionControllerButtons { Up, Down, Left, Right, L, R, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0, Star, Pound };
}
