using System;

namespace Mesen.GUI.Config
{
	public class KeyPresets
	{
		KeyMapping _wasdLayout;
		KeyMapping _arrowLayout;
		KeyMapping[] _xboxLayouts = new KeyMapping[2];
		KeyMapping[] _ps4Layouts = new KeyMapping[2];
		KeyMapping[] _snes30Layouts = new KeyMapping[2];

		public KeyMapping WasdLayout { get { return _wasdLayout.Clone(); } }
		public KeyMapping ArrowLayout { get { return _arrowLayout.Clone(); } }
		public KeyMapping XboxLayout1 { get { return _xboxLayouts[0].Clone(); } }
		public KeyMapping XboxLayout2 { get { return _xboxLayouts[1].Clone(); } }
		public KeyMapping Ps4Layout1 { get { return _ps4Layouts[0].Clone(); } }
		public KeyMapping Ps4Layout2 { get { return _ps4Layouts[1].Clone(); } }
		public KeyMapping Snes30Layout1 { get { return _snes30Layouts[0].Clone(); } }
		public KeyMapping Snes30Layout2 { get { return _snes30Layouts[1].Clone(); } }

		public KeyPresets()
		{
			_wasdLayout = new KeyMapping() {
				A = InputApi.GetKeyCode("K"),
				B = InputApi.GetKeyCode("J"),
				X = InputApi.GetKeyCode(","),
				Y = InputApi.GetKeyCode("M"),
				Select = InputApi.GetKeyCode("O"),
				Start = InputApi.GetKeyCode("L"),
				L = InputApi.GetKeyCode("U"),
				R = InputApi.GetKeyCode("I"),
				Up = InputApi.GetKeyCode("W"),
				Down = InputApi.GetKeyCode("S"),
				Left = InputApi.GetKeyCode("A"),
				Right = InputApi.GetKeyCode("D")
			};

			_arrowLayout = new KeyMapping() {
				A = InputApi.GetKeyCode("S"),
				B = InputApi.GetKeyCode("A"),
				X = InputApi.GetKeyCode("X"),
				Y = InputApi.GetKeyCode("Z"),
				Select = InputApi.GetKeyCode("E"),
				Start = InputApi.GetKeyCode("D"),
				L = InputApi.GetKeyCode("Q"),
				R = InputApi.GetKeyCode("W"),
				Up = InputApi.GetKeyCode("Up Arrow"),
				Down = InputApi.GetKeyCode("Down Arrow"),
				Left = InputApi.GetKeyCode("Left Arrow"),
				Right = InputApi.GetKeyCode("Right Arrow")
			};
			
			if(Program.IsMono) {
				//TODO test and update for Mono
				for(int i = 0; i < 2; i++) {
					string prefix = "Pad" + (i + 1).ToString() + " ";
					_xboxLayouts[i] = new KeyMapping() {
						A = InputApi.GetKeyCode(prefix + "A"),
						B = InputApi.GetKeyCode(prefix + "X"),
						X = InputApi.GetKeyCode(prefix + "B"),
						Y = InputApi.GetKeyCode(prefix + "Y"),
						Select = InputApi.GetKeyCode(prefix + "Select"),
						Start = InputApi.GetKeyCode(prefix + "Start"),
						Up = InputApi.GetKeyCode(prefix + "Up"),
						Down = InputApi.GetKeyCode(prefix + "Down"),
						Left = InputApi.GetKeyCode(prefix + "Left"),
						Right = InputApi.GetKeyCode(prefix + "Right")
					};

					_ps4Layouts[i] = new KeyMapping() {
						A = InputApi.GetKeyCode(prefix + "B"),
						B = InputApi.GetKeyCode(prefix + "A"),
						X = InputApi.GetKeyCode(prefix + "C"),
						Y = InputApi.GetKeyCode(prefix + "X"),
						Select = InputApi.GetKeyCode(prefix + "L2"),
						Start = InputApi.GetKeyCode(prefix + "R2"),
						Up = InputApi.GetKeyCode(prefix + "Up"),
						Down = InputApi.GetKeyCode(prefix + "Down"),
						Left = InputApi.GetKeyCode(prefix + "Left"),
						Right = InputApi.GetKeyCode(prefix + "Right")
					};

					_snes30Layouts[i] = new KeyMapping() {
						A = InputApi.GetKeyCode(prefix + "Thumb"),
						B = InputApi.GetKeyCode(prefix + "Top2"),
						X = InputApi.GetKeyCode(prefix + "Trigger"),
						Y = InputApi.GetKeyCode(prefix + "Top"),
						Select = InputApi.GetKeyCode(prefix + "Base5"),
						Start = InputApi.GetKeyCode(prefix + "Base6"),
						Up = InputApi.GetKeyCode(prefix + "Y-"),
						Down = InputApi.GetKeyCode(prefix + "Y+"),
						Left = InputApi.GetKeyCode(prefix + "X-"),
						Right = InputApi.GetKeyCode(prefix + "X+")
					};
				}
			} else {
				for(int i = 0; i < 2; i++) {
					string prefix = "Pad" + (i + 1).ToString() + " ";
					_xboxLayouts[i] = new KeyMapping() {
						A = InputApi.GetKeyCode(prefix + "B"),
						B = InputApi.GetKeyCode(prefix + "A"),
						X = InputApi.GetKeyCode(prefix + "Y"),
						Y = InputApi.GetKeyCode(prefix + "X"),
						Select = InputApi.GetKeyCode(prefix + "Back"),
						Start = InputApi.GetKeyCode(prefix + "Start"),
						L = InputApi.GetKeyCode(prefix + "L1"),
						R = InputApi.GetKeyCode(prefix + "R1"),
						Up = InputApi.GetKeyCode(prefix + "Up"),
						Down = InputApi.GetKeyCode(prefix + "Down"),
						Left = InputApi.GetKeyCode(prefix + "Left"),
						Right = InputApi.GetKeyCode(prefix + "Right")
					};

					prefix = "Joy" + (i + 1).ToString() + " ";
					_ps4Layouts[i] = new KeyMapping() {
						A = InputApi.GetKeyCode(prefix + "But3"),
						B = InputApi.GetKeyCode(prefix + "But2"),
						X = InputApi.GetKeyCode(prefix + "But4"),
						Y = InputApi.GetKeyCode(prefix + "But1"),
						Select = InputApi.GetKeyCode(prefix + "But9"),
						Start = InputApi.GetKeyCode(prefix + "But10"),
						L = InputApi.GetKeyCode(prefix + "But5"),
						R = InputApi.GetKeyCode(prefix + "But6"),
						Up = InputApi.GetKeyCode(prefix + "DPad Up"),
						Down = InputApi.GetKeyCode(prefix + "DPad Down"),
						Left = InputApi.GetKeyCode(prefix + "DPad Left"),
						Right = InputApi.GetKeyCode(prefix + "DPad Right")
					};

					_snes30Layouts[i] = new KeyMapping() {
						A = InputApi.GetKeyCode(prefix + "But1"),
						B = InputApi.GetKeyCode(prefix + "But2"),
						X = InputApi.GetKeyCode(prefix + "But4"),
						Y = InputApi.GetKeyCode(prefix + "But5"),
						Select = InputApi.GetKeyCode(prefix + "But11"),
						Start = InputApi.GetKeyCode(prefix + "But12"),
						L = InputApi.GetKeyCode(prefix + "But7"),
						R = InputApi.GetKeyCode(prefix + "But8"),
						Up = InputApi.GetKeyCode(prefix + "Y+"),
						Down = InputApi.GetKeyCode(prefix + "Y-"),
						Left = InputApi.GetKeyCode(prefix + "X-"),
						Right = InputApi.GetKeyCode(prefix + "X+")
					};
				}
			}
		}
	}
}
