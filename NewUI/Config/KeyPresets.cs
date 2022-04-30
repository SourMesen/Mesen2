using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Runtime.InteropServices;

namespace Mesen.Config
{
	public static class KeyPresets
	{
		public static void ApplyWasdLayout(KeyMapping m, ControllerType type)
		{
			m.A = InputApi.GetKeyCode("K");
			m.B = InputApi.GetKeyCode("J");
			if(type == ControllerType.SnesController) {
				m.X = InputApi.GetKeyCode(";");
				m.Y = InputApi.GetKeyCode("M");
				m.L = InputApi.GetKeyCode("U");
				m.R = InputApi.GetKeyCode("I");
				m.Select = InputApi.GetKeyCode("O");
				m.Start = InputApi.GetKeyCode("L");
			} else if(type == ControllerType.PceAvenuePad6) {
				m.X = InputApi.GetKeyCode("H");
				m.Y = InputApi.GetKeyCode("Y");
				m.L = InputApi.GetKeyCode("U");
				m.R = InputApi.GetKeyCode("I");
				m.Select = InputApi.GetKeyCode("N");
				m.Start = InputApi.GetKeyCode("M");
			} else {
				m.TurboA = InputApi.GetKeyCode(";");
				m.TurboB = InputApi.GetKeyCode("M");
				m.Select = InputApi.GetKeyCode("U");
				m.Start = InputApi.GetKeyCode("I");
			}

			m.Up = InputApi.GetKeyCode("W");
			m.Down = InputApi.GetKeyCode("S");
			m.Left = InputApi.GetKeyCode("A");
			m.Right = InputApi.GetKeyCode("D");
		}

		public static void ApplyArrowLayout(KeyMapping m, ControllerType type)
		{
			m.A = InputApi.GetKeyCode("S");
			m.B = InputApi.GetKeyCode("A");
			if(type == ControllerType.SnesController) {
				m.X = InputApi.GetKeyCode("X");
				m.Y = InputApi.GetKeyCode("Z");
				m.L = InputApi.GetKeyCode("Q");
				m.R = InputApi.GetKeyCode("W");
				m.Select = InputApi.GetKeyCode("E");
				m.Start = InputApi.GetKeyCode("D");
			} else if(type == ControllerType.PceAvenuePad6) {
				m.Y = InputApi.GetKeyCode("A");
				m.L = InputApi.GetKeyCode("S");
				m.R = InputApi.GetKeyCode("D");
				m.A = InputApi.GetKeyCode("Z");
				m.B = InputApi.GetKeyCode("X");
				m.X = InputApi.GetKeyCode("C");
				m.Select = InputApi.GetKeyCode("Q");
				m.Start = InputApi.GetKeyCode("W");
			} else {
				m.TurboA = InputApi.GetKeyCode("X");
				m.TurboB = InputApi.GetKeyCode("Z");
				m.Select = InputApi.GetKeyCode("Q");
				m.Start = InputApi.GetKeyCode("W");
			}
			m.Up = InputApi.GetKeyCode("Up Arrow");
			m.Down = InputApi.GetKeyCode("Down Arrow");
			m.Left = InputApi.GetKeyCode("Left Arrow");
			m.Right = InputApi.GetKeyCode("Right Arrow");
		}

		public static void ApplyXboxLayout(KeyMapping m, int player, ControllerType type)
		{
			string prefix = "Pad" + (player + 1).ToString() + " ";
			m.A = InputApi.GetKeyCode(prefix + "B");
			m.B = InputApi.GetKeyCode(prefix + "A");
			if(type == ControllerType.SnesController || type == ControllerType.PceAvenuePad6) {
				m.X = InputApi.GetKeyCode(prefix + "Y");
				m.Y = InputApi.GetKeyCode(prefix + "X");
				m.L = InputApi.GetKeyCode(prefix + "L1");
				m.R = InputApi.GetKeyCode(prefix + "R1");
			} else {
				m.TurboA = InputApi.GetKeyCode(prefix + "Y");
				m.TurboB = InputApi.GetKeyCode(prefix + "X");
			}
			m.Select = InputApi.GetKeyCode(prefix + "Back");
			m.Start = InputApi.GetKeyCode(prefix + "Start");
			m.Up = InputApi.GetKeyCode(prefix + "Up");
			m.Down = InputApi.GetKeyCode(prefix + "Down");
			m.Left = InputApi.GetKeyCode(prefix + "Left");
			m.Right = InputApi.GetKeyCode(prefix + "Right");
		}

		public static void ApplyPs4Layout(KeyMapping m, int player, ControllerType type)
		{
			string prefix = "Joy" + (player + 1).ToString() + " ";
			m.A = InputApi.GetKeyCode(prefix + "But3");
			m.B = InputApi.GetKeyCode(prefix + "But2");
			if(type == ControllerType.SnesController || type == ControllerType.PceAvenuePad6) {
				m.X = InputApi.GetKeyCode(prefix + "But4");
				m.Y = InputApi.GetKeyCode(prefix + "But1");
				m.L = InputApi.GetKeyCode(prefix + "But5");
				m.R = InputApi.GetKeyCode(prefix + "But6");
			} else {
				m.TurboA = InputApi.GetKeyCode(prefix + "But4");
				m.TurboB = InputApi.GetKeyCode(prefix + "But1");
			}
			m.Select = InputApi.GetKeyCode(prefix + "But9");
			m.Start = InputApi.GetKeyCode(prefix + "But10");
			m.Up = InputApi.GetKeyCode(prefix + "DPad Up");
			m.Down = InputApi.GetKeyCode(prefix + "DPad Down");
			m.Left = InputApi.GetKeyCode(prefix + "DPad Left");
			m.Right = InputApi.GetKeyCode(prefix + "DPad Right");
		}
	}
}
