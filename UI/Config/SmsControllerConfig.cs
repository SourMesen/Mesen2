using Mesen.Interop;
using Mesen.Localization;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Config
{
	public class SmsControllerConfig : ControllerConfig
	{
		public new SmsKeyMapping Mapping1 { get => (SmsKeyMapping)_mapping1; set => _mapping1 = value; }
		public new SmsKeyMapping Mapping2 { get => (SmsKeyMapping)_mapping2; set => _mapping2 = value; }
		public new SmsKeyMapping Mapping3 { get => (SmsKeyMapping)_mapping3; set => _mapping3 = value; }
		public new SmsKeyMapping Mapping4 { get => (SmsKeyMapping)_mapping4; set => _mapping4 = value; }

		public SmsControllerConfig()
		{
			_mapping1 = new SmsKeyMapping();
			_mapping2 = new SmsKeyMapping();
			_mapping3 = new SmsKeyMapping();
			_mapping4 = new SmsKeyMapping();
		}
	}

	public class SmsKeyMapping : KeyMapping
	{
		public UInt16[]? LightPhaserButtons { get; set; } = null;

		protected override UInt16[]? GetCustomButtons(ControllerType type)
		{
			return type switch {
				ControllerType.SmsLightPhaser => LightPhaserButtons,
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
				ControllerType.SmsLightPhaser => Enum.GetValues<SmsLightPhaserButtons>().Select(val => new CustomKeyMapping(ResourceHelper.GetEnumText(val), buttonMappings, (int)val)).ToList(),
				_ => new()
			};

			keys.Sort((a, b) => a.Name.CompareTo(b.Name));

			return keys;
		}

		public override void ClearKeys(ControllerType type)
		{
			switch(type) {
				case ControllerType.SmsLightPhaser:
					LightPhaserButtons = new UInt16[2];
					break;
			}
		}

		public override UInt16[]? GetDefaultCustomKeys(ControllerType type, KeyPresetType? preset)
		{
			switch(type) {
				case ControllerType.SmsLightPhaser:
					return new UInt16[2] {
						InputApi.GetKeyCode("Mouse Left"),
						InputApi.GetKeyCode("Mouse Right")
					};

				default:
					return null;
			}
		}

		public override void SetDefaultKeys(ControllerType type, KeyPresetType? preset)
		{
			switch(type) {
				case ControllerType.SmsLightPhaser: LightPhaserButtons = GetDefaultCustomKeys(type, preset); break;

				default:
					base.SetDefaultKeys(type, preset);
					break;
			}
		}
	}

	public enum SmsLightPhaserButtons { Fire, AimOffscreen };
}
