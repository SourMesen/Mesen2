using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config.Shortcuts
{
	public class KeyCombination
	{
		public UInt16 Key1 { get; set; }
		public UInt16 Key2 { get; set; }
		public UInt16 Key3 { get; set; }

		public bool IsEmpty { get { return Key1 == 0 && Key2 == 0 && Key3 == 0; } }

		public override string ToString()
		{
			if(IsEmpty) {
				return "";
			} else {
				return GetKeyNames();
			}
		}

		public KeyCombination()
		{
		}

		public KeyCombination(List<UInt16>? keyCodes = null)
		{
			if(keyCodes != null) {
				if(keyCodes.Any(code => code > 0xFFFF)) {
					//If both keyboard & gamepad buttons/keys exist, only use the gamepad buttons
					//This fixes an issue with Steam where Steam can remap gamepad buttons to send keyboard keys
					//See: Settings -> Controller Settings -> General Controller Settings -> Checking the Xbox/PS4/Generic/etc controller checkboxes will cause this
					keyCodes = keyCodes.Where(code => code > 0xFFFF).ToList();
				}

				Key1 = keyCodes.Count > 0 ? keyCodes[0] : (UInt16)0;
				Key2 = keyCodes.Count > 1 ? keyCodes[1] : (UInt16)0;
				Key3 = keyCodes.Count > 2 ? keyCodes[2] : (UInt16)0;
			} else {
				Key1 = 0;
				Key2 = 0;
				Key3 = 0;
			}
		}

		private string GetKeyNames()
		{
			List<UInt16> scanCodes = new List<UInt16>() { Key1, Key2, Key3 };
			List<string> keyNames = scanCodes.Select((UInt16 scanCode) => InputApi.GetKeyName(scanCode)).Where((keyName) => !string.IsNullOrWhiteSpace(keyName)).ToList();

			if(keyNames.Count > 1) {
				//Merge left/right ctrl/alt/shift for key combinations
				keyNames = keyNames.Select(key => {
					switch(key) {
						case "Left Ctrl":
						case "Right Ctrl":
							return "Ctrl";

						case "Left Alt":
						case "Right Alt":
							return "Alt";

						case "Left Shift":
						case "Right Shift":
							return "Shift";

						default:
							return key;
					}
				}).ToList();
			}

			keyNames.Sort((string a, string b) => {
				if(a == b) {
					return 0;
				}

				if(a.Contains("Ctrl")) {
					return -1;
				} else if(b.Contains("Ctrl")) {
					return 1;
				}

				if(a.Contains("Alt")) {
					return -1;
				} else if(b.Contains("Alt")) {
					return 1;
				}

				if(a.Contains("Shift")) {
					return -1;
				} else if(b.Contains("Shift")) {
					return 1;
				}

				return a.CompareTo(b);
			});

			return string.Join("+", keyNames);
		}

		public InteropKeyCombination ToInterop()
		{
			return new InteropKeyCombination() {
				Key1 = Key1,
				Key2 = Key2,
				Key3 = Key3
			};
		}
	}
}
