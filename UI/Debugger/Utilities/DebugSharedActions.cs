using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	internal class DebugSharedActions
	{
		public static List<ContextMenuAction> GetStepActions(Control wnd, Func<CpuType> getCpuType)
		{
			return new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.Continue,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Continue),
					IsEnabled = () => EmuApi.IsPaused(),
					OnClick = () => DebugSharedActions.ResumeExecution()
				},
				new ContextMenuAction() {
					ActionType = ActionType.Break,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Break),
					IsEnabled = () => !EmuApi.IsPaused(),
					OnClick = () => Step(getCpuType(), StepType.Step)
				},
				new ContextMenuAction() {
					ActionType = ActionType.Custom,
					IsVisible = () => false,
					AllowedWhenHidden = true,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ToggleBreakContinue),
					OnClick = () => {
						if(EmuApi.IsPaused()) {
							DebugSharedActions.ResumeExecution();
						} else {
							Step(getCpuType(), StepType.Step);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.StepInto,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepInto),
					OnClick = () => Step(getCpuType(), StepType.Step, 1)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepOver,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepOver),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).StepOver,
					AllowedWhenHidden = true,
					OnClick = () => Step(getCpuType(), StepType.StepOver, 1)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepOut,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepOut),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).StepOut,
					AllowedWhenHidden = true,
					OnClick = () => Step(getCpuType(), StepType.StepOut, 1)
				},
				new ContextMenuSeparator() { IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).StepBack },
				new ContextMenuAction() {
					ActionType = ActionType.StepBack,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepBack),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).StepBack,
					OnClick = () => Step(getCpuType(), StepType.StepBack, (int)StepBackType.Instruction)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepBackScanline,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepBackScanline),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).StepBack,
					OnClick = () => Step(getCpuType(), StepType.StepBack, (int)StepBackType.Scanline)
				},
				new ContextMenuAction() {
					ActionType = ActionType.StepBackFrame,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.StepBackFrame),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).StepBack,
					OnClick = () => Step(getCpuType(), StepType.StepBack, (int)StepBackType.Frame)
				},
				new ContextMenuSeparator() { IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).CpuCycleStep },
				new ContextMenuAction() {
					ActionType = ActionType.RunCpuCycle,
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).CpuCycleStep,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunCpuCycle),
					OnClick = () => Step(getCpuType(), StepType.CpuCycleStep, 1)
				},
				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.RunPpuCycle,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunPpuCycle),
					OnClick = () => Step(getCpuType(), StepType.PpuStep, 1)
				},
				new ContextMenuAction() {
					ActionType = ActionType.RunPpuScanline,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunPpuScanline),
					OnClick = () => Step(getCpuType(), StepType.PpuScanline, 1)
				},
				new ContextMenuAction() {
					ActionType = ActionType.RunPpuFrame,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunPpuFrame),
					OnClick = () => Step(getCpuType(), StepType.PpuFrame, 1)
				},

				new ContextMenuSeparator(),
				
				new ContextMenuAction() {
					ActionType = ActionType.RunToNmi,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunToNmi),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).RunToNmi,
					OnClick = () => Step(getCpuType(), StepType.RunToNmi)
				},
				new ContextMenuAction() {
					ActionType = ActionType.RunToIrq,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RunToIrq),
					IsVisible = () => DebugApi.GetDebuggerFeatures(getCpuType()).RunToIrq,
					OnClick = () => Step(getCpuType(), StepType.RunToIrq)
				},
				new ContextMenuAction() {
					ActionType = ActionType.BreakOn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakOn),
					OnClick = () => new BreakOnWindow(getCpuType()).ShowCenteredDialog(wnd)
				},

				new ContextMenuSeparator(),

				new ContextMenuAction() {
					ActionType = ActionType.BreakIn,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.BreakIn),
					OnClick = () => new BreakInWindow(getCpuType()).ShowCenteredDialog(wnd)
				}
			};
		}

		public static void Step(CpuType cpuType, StepType type, int instructionCount = 1)
		{
			switch(type) {
				case StepType.PpuStep:
				case StepType.PpuScanline:
				case StepType.PpuFrame:
					DebugApi.Step(cpuType.GetConsoleType().GetMainCpuType(), instructionCount, type);
					break;

				default:
					DebugApi.Step(cpuType, instructionCount, type);
					break;
			}
		}

		private static void ResumeExecution()
		{
			DebugApi.ResumeExecution();
			if(ConfigManager.Config.Debug.Debugger.FocusGameOnResume) {
				ApplicationHelper.GetMainWindow()?.BringToFront();
			}
		}
	}

	enum StepBackType
	{
		Instruction,
		Scanline,
		Frame
	}
}
