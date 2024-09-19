using Avalonia;
using Avalonia.Controls;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.ViewModels
{
	public class ControllerInputViewModel : ViewModelBase
	{
		[Reactive] public int ViewHeight { get; set; }

		[Reactive] public bool ButtonA { get; set; }
		[Reactive] public bool ButtonB { get; set; }
		[Reactive] public bool ButtonX { get; set; }
		[Reactive] public bool ButtonY { get; set; }
		[Reactive] public bool ButtonL { get; set; }
		[Reactive] public bool ButtonR { get; set; }
		[Reactive] public bool ButtonU { get; set; }
		[Reactive] public bool ButtonD { get; set; }
		[Reactive] public bool ButtonUp { get; set; }
		[Reactive] public bool ButtonDown { get; set; }
		[Reactive] public bool ButtonLeft { get; set; }
		[Reactive] public bool ButtonRight { get; set; }
		[Reactive] public bool ButtonSelect { get; set; }
		[Reactive] public bool ButtonStart { get; set; }

		public int ControllerIndex { get; }
		public bool IsSnes { get; }
		public bool IsWs { get; }
		public bool HasShoulderButtons { get; }
		public bool HasSelectButton { get; }
		public bool HasStartButton { get; }

		[Obsolete("For designer only")]
		public ControllerInputViewModel() : this(ConsoleType.Ws, 0) { }

		public ControllerInputViewModel(ConsoleType consoleType, int index)
		{
			ControllerIndex = index + 1;
			IsSnes = consoleType == ConsoleType.Snes;
			IsWs = consoleType == ConsoleType.Ws;
			HasShoulderButtons = consoleType == ConsoleType.Snes || consoleType == ConsoleType.Gba;
			HasSelectButton = consoleType != ConsoleType.Sms;
			HasStartButton = consoleType != ConsoleType.Sms || index == 0;
			ViewHeight = consoleType != ConsoleType.Ws ? (HasShoulderButtons ? 34 : 30) : 64;

			if(Design.IsDesignMode) {
				return;
			}

			PropertyChanged += ControllerInputViewModel_PropertyChanged;
		}

		private void ControllerInputViewModel_PropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
		{
			SetInputOverrides();
		}

		public void SetInputOverrides()
		{
			DebugApi.SetInputOverrides((uint)ControllerIndex - 1, new DebugControllerState() {
				A = ButtonA,
				B = ButtonB,
				X = ButtonX,
				Y = ButtonY,
				L = ButtonL,
				R = ButtonR,
				U = ButtonU,
				D = ButtonD,
				Up = ButtonUp,
				Down = ButtonDown,
				Left = ButtonLeft,
				Right = ButtonRight,
				Select = ButtonSelect,
				Start = ButtonStart
			});
		}

		public void OnButtonClick(object param)
		{
			if(param is string buttonName) {
				switch(buttonName) {
					case "A": ButtonA = !ButtonA; break;
					case "B": ButtonB = !ButtonB; break;
					case "X": ButtonX = !ButtonX; break;
					case "Y": ButtonY = !ButtonY; break;
					case "L": ButtonL = !ButtonL; break;
					case "R": ButtonR = !ButtonR; break;
					case "U": ButtonU = !ButtonU; break;
					case "D": ButtonD = !ButtonD; break;
					case "Up": ButtonUp = !ButtonUp; break;
					case "Down": ButtonDown = !ButtonDown; break;
					case "Left": ButtonLeft = !ButtonLeft; break;
					case "Right": ButtonRight = !ButtonRight; break;
					case "Select": ButtonSelect = !ButtonSelect; break;
					case "Start": ButtonStart = !ButtonStart; break;
				}
			}
		}
	}
}
