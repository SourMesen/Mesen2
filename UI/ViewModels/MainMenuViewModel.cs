using Avalonia.Controls;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class MainMenuViewModel : ViewModelBase
	{
		public MainWindowViewModel MainWindow { get; set; }

		[Reactive] public List<object> FileMenuItems { get; set; } = new();
		[Reactive] public List<object> GameMenuItems { get; set; } = new();
		[Reactive] public List<object> OptionsMenuItems { get; set; } = new();
		[Reactive] public List<object> ToolsMenuItems { get; set; } = new();
		[Reactive] public List<object> DebugMenuItems { get; set; } = new();
		[Reactive] public List<object> HelpMenuItems { get; set; } = new();
		
		[Reactive] private List<object> _netPlayControllers { get; set; } = new();

		private RomInfo RomInfo => MainWindow.RomInfo;
		private bool IsGameRunning => RomInfo.Format != RomFormat.Unknown;
		private bool IsFdsGame => RomInfo.Format == RomFormat.Fds;
		private bool IsVsSystemGame => RomInfo.Format == RomFormat.VsSystem || RomInfo.Format == RomFormat.VsDualSystem;
		private bool IsVsDualSystemGame => RomInfo.Format == RomFormat.VsDualSystem;
		private List<RecentItem> RecentItems => ConfigManager.Config.RecentFiles.Items;

		private ConfigWindow? _cfgWindow = null;
		private MainMenuAction _selectControllerAction = new();

		[Obsolete("For designer only")]
		public MainMenuViewModel() : this(new MainWindowViewModel()) { }

		public MainMenuViewModel(MainWindowViewModel windowModel)
		{
			MainWindow = windowModel;
		}

		private void OpenConfig(MainWindow wnd, ConfigWindowTab tab)
		{
			if(_cfgWindow == null) {
				_cfgWindow = new ConfigWindow(tab);
				_cfgWindow.Closed += cfgWindow_Closed;
				_cfgWindow.ShowCentered((Control)wnd);
			} else {
				(_cfgWindow.DataContext as ConfigViewModel)!.SelectTab(tab);
				_cfgWindow.BringToFront();
			}
		}

		private void cfgWindow_Closed(object? sender, EventArgs e)
		{
			_cfgWindow = null;
			if(ConfigManager.Config.Preferences.GameSelectionScreenMode == GameSelectionMode.Disabled && MainWindow.RecentGames.Visible) {
				MainWindow.RecentGames.Visible = false;
			} else if(ConfigManager.Config.Preferences.GameSelectionScreenMode != GameSelectionMode.Disabled && !IsGameRunning) {
				MainWindow.RecentGames.Init(GameScreenMode.RecentGames);
			}
		}

		public void Initialize(MainWindow wnd)
		{
			InitFileMenu(wnd);
			InitGameMenu(wnd);
			InitOptionsMenu(wnd);
			InitToolMenu(wnd);
			InitDebugMenu(wnd);
			InitHelpMenu(wnd);
		}

		private void InitFileMenu(MainWindow wnd)
		{
			FileMenuItems = new List<object>() {
				new MainMenuAction(EmulatorShortcut.OpenFile) { ActionType = ActionType.Open },
				new ContextMenuSeparator(),
				new MainMenuAction() {
					ActionType = ActionType.SaveState,
					SubActions = new List<object> {
						GetSaveStateMenuItem(1, true),
						GetSaveStateMenuItem(2, true),
						GetSaveStateMenuItem(3, true),
						GetSaveStateMenuItem(4, true),
						GetSaveStateMenuItem(5, true),
						GetSaveStateMenuItem(6, true),
						GetSaveStateMenuItem(7, true),
						GetSaveStateMenuItem(8, true),
						GetSaveStateMenuItem(9, true),
						GetSaveStateMenuItem(10, true),
						new ContextMenuSeparator(),
						new MainMenuAction(EmulatorShortcut.SaveStateDialog) { ActionType = ActionType.SaveStateDialog },
						new MainMenuAction(EmulatorShortcut.SaveStateToFile) { ActionType = ActionType.SaveStateToFile },
					}
				},
				new MainMenuAction() {
					ActionType = ActionType.LoadState,
					SubActions = new List<object> {
						GetSaveStateMenuItem(1, false),
						GetSaveStateMenuItem(2, false),
						GetSaveStateMenuItem(3, false),
						GetSaveStateMenuItem(4, false),
						GetSaveStateMenuItem(5, false),
						GetSaveStateMenuItem(6, false),
						GetSaveStateMenuItem(7, false),
						GetSaveStateMenuItem(8, false),
						GetSaveStateMenuItem(9, false),
						GetSaveStateMenuItem(10, false),
						new ContextMenuSeparator(),
						GetSaveStateMenuItem(11, false),
						new ContextMenuSeparator(),
						new MainMenuAction(EmulatorShortcut.LoadStateDialog) { ActionType = ActionType.LoadStateDialog },
						new MainMenuAction(EmulatorShortcut.LoadStateFromFile) { ActionType = ActionType.LoadStateFromFile },
					}
				},

				new MainMenuAction(EmulatorShortcut.LoadLastSession) {
					ActionType = ActionType.LoadLastSession,
					IsEnabled = () => File.Exists(Path.Combine(ConfigManager.RecentGamesFolder, MainWindow.RomInfo.GetRomName() + ".rgd"))
				},

				new ContextMenuSeparator(),
				new MainMenuAction() {
					ActionType = ActionType.RecentFiles,
					IsEnabled = () => ConfigManager.Config.RecentFiles.Items.Count > 0,
					SubActions = new List<object>() {
						GetRecentMenuItem(0),
						GetRecentMenuItem(1),
						GetRecentMenuItem(2),
						GetRecentMenuItem(3),
						GetRecentMenuItem(4),
						GetRecentMenuItem(5),
						GetRecentMenuItem(6),
						GetRecentMenuItem(7),
						GetRecentMenuItem(8),
						GetRecentMenuItem(9)
					}
				},
				new ContextMenuSeparator(),
				new MainMenuAction(EmulatorShortcut.Exit) { ActionType = ActionType.Exit },
			};
		}

		private MainMenuAction GetRecentMenuItem(int index)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				DynamicText = () => index < RecentItems.Count ? RecentItems[index].DisplayText : "",
				CustomShortcutText = () => index < RecentItems.Count ? RecentItems[index].ShortenedFolder : "",
				IsVisible = () => index < RecentItems.Count,
				OnClick = () => {
					if(index < RecentItems.Count) {
						LoadRomHelper.LoadRom(RecentItems[index].RomFile, RecentItems[index].PatchFile);
					}
				}
			};
		}

		private MainMenuAction GetSaveStateMenuItem(int slot, bool forSave)
		{
			EmulatorShortcut shortcut;
			if(forSave) {
				shortcut = (EmulatorShortcut)((int)EmulatorShortcut.SaveStateSlot1 + slot - 1);
			} else {
				shortcut = (EmulatorShortcut)((int)EmulatorShortcut.LoadStateSlot1 + slot - 1);
			}
			
			bool isAutoSaveSlot = slot == 11;

			return new MainMenuAction(shortcut) {
				ActionType = ActionType.Custom,
				DynamicText = () => {
					string statePath = Path.Combine(ConfigManager.SaveStateFolder, EmuApi.GetRomInfo().GetRomName() + "_" + slot + "." + FileDialogHelper.MesenSaveStateExt);
					string slotName = isAutoSaveSlot ? "Auto" : slot.ToString();

					string header;
					if(!File.Exists(statePath)) {
						header = slotName + ". " + ResourceHelper.GetMessage("EmptyState");
					} else {
						DateTime dateTime = new FileInfo(statePath).LastWriteTime;
						header = slotName + ". " + dateTime.ToShortDateString() + " " + dateTime.ToShortTimeString();
					}
					return header;
				},
				OnClick = () => {
					if(forSave) {
						EmuApi.SaveState((uint)slot);
					} else {
						EmuApi.LoadState((uint)slot);
					}
				}
			};
		}

		private void InitGameMenu(MainWindow wnd)
		{
			GameMenuItems = new List<object>() {
				new MainMenuAction(EmulatorShortcut.Pause) { ActionType = ActionType.Pause, IsVisible = () => !EmuApi.IsPaused() && (!ConfigManager.Config.Preferences.PauseWhenInMenusAndConfig || !EmuApi.IsRunning()) },
				new MainMenuAction(EmulatorShortcut.Pause) { ActionType = ActionType.Resume, IsVisible = () => EmuApi.IsPaused() || (ConfigManager.Config.Preferences.PauseWhenInMenusAndConfig && EmuApi.IsRunning()) },
				new ContextMenuSeparator(),
				new MainMenuAction(EmulatorShortcut.Reset) { ActionType = ActionType.Reset },
				new MainMenuAction(EmulatorShortcut.PowerCycle) { ActionType = ActionType.PowerCycle },
				new MainMenuAction(EmulatorShortcut.ReloadRom) { ActionType = ActionType.ReloadRom },
				new ContextMenuSeparator(),
				new MainMenuAction(EmulatorShortcut.PowerOff) { ActionType = ActionType.PowerOff },
				
				new ContextMenuSeparator() { IsVisible = () => IsGameRunning && RomInfo.ConsoleType != ConsoleType.Gameboy && RomInfo.Format != RomFormat.GameGear && RomInfo.ConsoleType != ConsoleType.Gba },
				new MainMenuAction() { 
					ActionType = ActionType.GameConfig,
					IsVisible = () => IsGameRunning && RomInfo.ConsoleType != ConsoleType.Gameboy && RomInfo.Format != RomFormat.GameGear && RomInfo.ConsoleType != ConsoleType.Gba,
					IsEnabled = () => IsGameRunning,
					OnClick = () => {
						new GameConfigWindow().ShowCenteredDialog((Control)wnd);
					}
				},

				new ContextMenuSeparator() { IsVisible = () => IsFdsGame },

				new MainMenuAction() {
					ActionType = ActionType.SelectDisk,
					IsVisible = () => IsFdsGame,
					SubActions = new List<object>() {
						GetFdsInsertDiskItem(0),
						GetFdsInsertDiskItem(1),
						GetFdsInsertDiskItem(2),
						GetFdsInsertDiskItem(3),
						GetFdsInsertDiskItem(4),
						GetFdsInsertDiskItem(5),
						GetFdsInsertDiskItem(6),
						GetFdsInsertDiskItem(7),
					}
				},
				
				new MainMenuAction(EmulatorShortcut.FdsEjectDisk) {
					ActionType = ActionType.EjectDisk,
					IsVisible = () => IsFdsGame,
				},

				new ContextMenuSeparator() { IsVisible = () => IsVsSystemGame },

				new MainMenuAction(EmulatorShortcut.VsInsertCoin1) { ActionType = ActionType.InsertCoin1, IsVisible = () => IsVsSystemGame },
				new MainMenuAction(EmulatorShortcut.VsInsertCoin2) { ActionType = ActionType.InsertCoin2, IsVisible = () => IsVsSystemGame },
				new MainMenuAction(EmulatorShortcut.VsInsertCoin3) { ActionType = ActionType.InsertCoin3, IsVisible = () => IsVsDualSystemGame },
				new MainMenuAction(EmulatorShortcut.VsInsertCoin4) { ActionType = ActionType.InsertCoin4, IsVisible = () => IsVsDualSystemGame },

				new ContextMenuSeparator() { IsVisible = () => EmuApi.IsShortcutAllowed(EmulatorShortcut.InputBarcode) || EmuApi.IsShortcutAllowed(EmulatorShortcut.RecordTape) || EmuApi.IsShortcutAllowed(EmulatorShortcut.StopRecordTape) },
				
				new MainMenuAction(EmulatorShortcut.InputBarcode) {
					ActionType = ActionType.InputBarcode,
					IsVisible = () => EmuApi.IsShortcutAllowed(EmulatorShortcut.InputBarcode)
				},

				new MainMenuAction() {
					ActionType = ActionType.TapeRecorder,
					IsVisible = () => EmuApi.IsShortcutAllowed(EmulatorShortcut.RecordTape) || EmuApi.IsShortcutAllowed(EmulatorShortcut.StopRecordTape),
					SubActions = new() {
						new MainMenuAction(EmulatorShortcut.LoadTape) {
							ActionType = ActionType.Play,
						},
						new ContextMenuSeparator(),
						new MainMenuAction(EmulatorShortcut.RecordTape) {
							ActionType = ActionType.Record,
						},
						new MainMenuAction(EmulatorShortcut.StopRecordTape) {
							ActionType = ActionType.Stop,
						},
					}
				},
			};
		}

		private MainMenuAction GetFdsInsertDiskItem(int diskSide)
		{
			return new MainMenuAction(EmulatorShortcut.FdsInsertDiskNumber) {
				ActionType = ActionType.Custom,
				CustomText = "Disk " + ((diskSide / 2) + 1) + " Side " + ((diskSide % 2 == 0) ? "A" : "B"),
				ShortcutParam = (uint)diskSide,
				IsVisible = () => EmuApi.IsShortcutAllowed(EmulatorShortcut.FdsInsertDiskNumber, (uint)diskSide)
			};
		}

		private void InitOptionsMenu(MainWindow wnd)
		{
			OptionsMenuItems = new List<object>() {
				new MainMenuAction() {
					ActionType = ActionType.Speed,
					SubActions = new List<object> {
						GetSpeedMenuItem(ActionType.NormalSpeed, 100),
						new ContextMenuSeparator(),
						new MainMenuAction(EmulatorShortcut.IncreaseSpeed) {
							ActionType = ActionType.IncreaseSpeed
						},
						new MainMenuAction(EmulatorShortcut.DecreaseSpeed) {
							ActionType = ActionType.DecreaseSpeed
						},
						GetSpeedMenuItem(ActionType.MaximumSpeed, 0, EmulatorShortcut.MaxSpeed),
						new ContextMenuSeparator(),
						GetSpeedMenuItem(ActionType.TripleSpeed, 300),
						GetSpeedMenuItem(ActionType.DoubleSpeed, 200),
						GetSpeedMenuItem(ActionType.HalfSpeed, 50),
						GetSpeedMenuItem(ActionType.QuarterSpeed, 25),
						new ContextMenuSeparator(),
						new MainMenuAction(EmulatorShortcut.ToggleFps) {
							ActionType = ActionType.ShowFps,
							IsSelected = () => ConfigManager.Config.Preferences.ShowFps
						}
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.VideoScale,
					SubActions = new List<object>() {
						GetScaleMenuItem(1, EmulatorShortcut.SetScale1x),
						GetScaleMenuItem(2, EmulatorShortcut.SetScale2x),
						GetScaleMenuItem(3, EmulatorShortcut.SetScale3x),
						GetScaleMenuItem(4, EmulatorShortcut.SetScale4x),
						GetScaleMenuItem(5, EmulatorShortcut.SetScale5x),
						GetScaleMenuItem(6, EmulatorShortcut.SetScale6x),
						GetScaleMenuItem(7, EmulatorShortcut.SetScale7x),
						GetScaleMenuItem(8, EmulatorShortcut.SetScale8x),
						GetScaleMenuItem(9, EmulatorShortcut.SetScale9x),
						GetScaleMenuItem(10, EmulatorShortcut.SetScale10x),
						new ContextMenuSeparator(),
						new MainMenuAction(EmulatorShortcut.ToggleFullscreen) {
							ActionType = ActionType.Fullscreen
						},
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.VideoFilter,
					SubActions = new List<object>() {
						GetVideoFilterMenuItem(VideoFilterType.None),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType.NtscBlargg),
						GetVideoFilterMenuItem(VideoFilterType.NtscBisqwit),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType.LcdGrid),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType.xBRZ2x),
						GetVideoFilterMenuItem(VideoFilterType.xBRZ3x),
						GetVideoFilterMenuItem(VideoFilterType.xBRZ4x),
						GetVideoFilterMenuItem(VideoFilterType.xBRZ5x),
						GetVideoFilterMenuItem(VideoFilterType.xBRZ6x),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType.HQ2x),
						GetVideoFilterMenuItem(VideoFilterType.HQ3x),
						GetVideoFilterMenuItem(VideoFilterType.HQ4x),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType.Scale2x),
						GetVideoFilterMenuItem(VideoFilterType.Scale3x),
						GetVideoFilterMenuItem(VideoFilterType.Scale4x),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType._2xSai),
						GetVideoFilterMenuItem(VideoFilterType.Super2xSai),
						GetVideoFilterMenuItem(VideoFilterType.SuperEagle),
						new ContextMenuSeparator(),
						GetVideoFilterMenuItem(VideoFilterType.Prescale2x),
						GetVideoFilterMenuItem(VideoFilterType.Prescale3x),
						GetVideoFilterMenuItem(VideoFilterType.Prescale4x),
						GetVideoFilterMenuItem(VideoFilterType.Prescale6x),
						GetVideoFilterMenuItem(VideoFilterType.Prescale8x),
						GetVideoFilterMenuItem(VideoFilterType.Prescale10x),
						new ContextMenuSeparator(),
						new MainMenuAction() {
							ActionType = ActionType.ToggleBilinearInterpolation,
							IsSelected = () => ConfigManager.Config.Video.UseBilinearInterpolation,
							OnClick = () => {
								ConfigManager.Config.Video.UseBilinearInterpolation = !ConfigManager.Config.Video.UseBilinearInterpolation;
								ConfigManager.Config.Video.ApplyConfig();
							}
						}
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.AspectRatio,
					SubActions = new List<object>() {
						GetAspectRatioMenuItem(VideoAspectRatio.NoStretching),
						new ContextMenuSeparator(),
						GetAspectRatioMenuItem(VideoAspectRatio.Auto),
						new ContextMenuSeparator(),
						GetAspectRatioMenuItem(VideoAspectRatio.NTSC),
						GetAspectRatioMenuItem(VideoAspectRatio.PAL),
						GetAspectRatioMenuItem(VideoAspectRatio.Standard),
						GetAspectRatioMenuItem(VideoAspectRatio.Widescreen),
						new ContextMenuSeparator(),
						GetAspectRatioMenuItem(VideoAspectRatio.Custom),
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.Region,
					IsEnabled = () => IsGameRunning,
					IsVisible = () => (
						!IsGameRunning || 
						MainWindow.RomInfo.ConsoleType != ConsoleType.Gameboy && MainWindow.RomInfo.ConsoleType != ConsoleType.Gba
					),
					SubActions = new List<object>() {
						GetRegionMenuItem(ConsoleRegion.Auto),
						GetPcEngineModelMenuItem(PceConsoleType.Auto),
						GetWsModelMenuItem(WsModel.Auto),
						new ContextMenuSeparator(),
						GetRegionMenuItem(ConsoleRegion.Ntsc),
						GetRegionMenuItem(ConsoleRegion.NtscJapan),
						GetRegionMenuItem(ConsoleRegion.Pal),
						GetRegionMenuItem(ConsoleRegion.Dendy),
						GetPcEngineModelMenuItem(PceConsoleType.PcEngine),
						GetPcEngineModelMenuItem(PceConsoleType.SuperGrafx),
						GetPcEngineModelMenuItem(PceConsoleType.TurboGrafx),
						GetWsModelMenuItem(WsModel.Monochrome),
						GetWsModelMenuItem(WsModel.Color),
						GetWsModelMenuItem(WsModel.SwanCrystal),
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.Region,
					DynamicText = () => ResourceHelper.GetEnumText(ActionType.Region) + (MainWindow.RomInfo.ConsoleType != ConsoleType.Gameboy ? " (GB)" : ""),
					IsVisible = () => IsGameRunning && MainWindow.RomInfo.CpuTypes.Contains(CpuType.Gameboy),
					SubActions = new List<object>() {
						GetGameboyModelMenuItem(GameboyModel.AutoFavorGbc),
						GetGameboyModelMenuItem(GameboyModel.AutoFavorSgb),
						GetGameboyModelMenuItem(GameboyModel.AutoFavorGb),
						new ContextMenuSeparator(),
						GetGameboyModelMenuItem(GameboyModel.Gameboy),
						GetGameboyModelMenuItem(GameboyModel.GameboyColor),
						GetGameboyModelMenuItem(GameboyModel.SuperGameboy),
					}
				},

				new ContextMenuSeparator(),

				new MainMenuAction() {
					ActionType = ActionType.Audio,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Audio)
				},
				new MainMenuAction() {
					ActionType = ActionType.Emulation,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Emulation)
				},
				new MainMenuAction() {
					ActionType = ActionType.Input,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Input)
				},
				new MainMenuAction() {
					ActionType = ActionType.Video,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Video)
				},

				new ContextMenuSeparator(),

				new MainMenuAction() {
					ActionType = ActionType.Nes,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Nes)
				},
				new MainMenuAction() {
					ActionType = ActionType.Snes,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Snes)
				},
				new MainMenuAction() {
					ActionType = ActionType.Gameboy,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Gameboy)
				},
				new MainMenuAction() {
					ActionType = ActionType.Gba,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Gba)
				},
				new MainMenuAction() {
					ActionType = ActionType.PcEngine,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.PcEngine)
				},
				new MainMenuAction() {
					ActionType = ActionType.Sms,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Sms)
				},
				new MainMenuAction() {
					ActionType = ActionType.Ws,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Ws)
				},
				new MainMenuAction() {
					ActionType = ActionType.OtherConsoles,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.OtherConsoles)
				},
				new ContextMenuSeparator(),

				new MainMenuAction() {
					ActionType = ActionType.Preferences,
					OnClick = () => OpenConfig(wnd, ConfigWindowTab.Preferences)
				}
			};
		}

		private MainMenuAction GetAspectRatioMenuItem(VideoAspectRatio aspectRatio)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetEnumText(aspectRatio),
				IsSelected = () => {
					ConsoleOverrideConfig? overrides = ConsoleOverrideConfig.GetActiveOverride();
					if(overrides?.OverrideAspectRatio == true) {
						return aspectRatio == overrides.AspectRatio;
					}
					return aspectRatio == ConfigManager.Config.Video.AspectRatio;
				},
				OnClick = () => {
					ConsoleOverrideConfig? overrides = ConsoleOverrideConfig.GetActiveOverride();
					if(overrides?.OverrideAspectRatio == true) {
						overrides.AspectRatio = aspectRatio;
					} else {
						ConfigManager.Config.Video.AspectRatio = aspectRatio;
					}
					ConfigManager.Config.Video.ApplyConfig();
				}
			};
		}

		private MainMenuAction GetRegionMenuItem(ConsoleRegion region)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				DynamicText = () => {
					if(region == ConsoleRegion.Pal && MainWindow.RomInfo.Format == RomFormat.GameGear) {
						return "PAL (60 FPS)"; //GG is 60fps even when region is PAL
					}
					return ResourceHelper.GetEnumText(region);
				},
				IsVisible = () => {
					if(MainWindow.RomInfo.ConsoleType == ConsoleType.PcEngine || MainWindow.RomInfo.ConsoleType == ConsoleType.Gameboy || MainWindow.RomInfo.ConsoleType == ConsoleType.Ws) {
						return false;
					}

					return region switch {
						ConsoleRegion.Ntsc => true,
						ConsoleRegion.NtscJapan => MainWindow.RomInfo.Format == RomFormat.GameGear,
						ConsoleRegion.Pal => true,
						ConsoleRegion.Dendy => MainWindow.RomInfo.ConsoleType == ConsoleType.Nes,
						ConsoleRegion.Auto or _ => true
					};
				},
				IsSelected = () => MainWindow.RomInfo.ConsoleType switch {
					ConsoleType.Snes => ConfigManager.Config.Snes.Region == region,
					ConsoleType.Nes => ConfigManager.Config.Nes.Region == region,
					ConsoleType.Sms => (
						MainWindow.RomInfo.Format switch {
							RomFormat.ColecoVision => ConfigManager.Config.Cv.Region,
							RomFormat.GameGear => ConfigManager.Config.Sms.GameGearRegion,
							_ => ConfigManager.Config.Sms.Region
						} == region
					),
					_ => region == ConsoleRegion.Auto
				},
				OnClick = () => {
					switch(MainWindow.RomInfo.ConsoleType) {
						case ConsoleType.Snes:
							ConfigManager.Config.Snes.Region = region;
							ConfigManager.Config.Snes.ApplyConfig();
							break;

						case ConsoleType.Nes:
							ConfigManager.Config.Nes.Region = region;
							ConfigManager.Config.Nes.ApplyConfig();
							break;

						case ConsoleType.Sms:
							switch(MainWindow.RomInfo.Format) {
								default: case RomFormat.Sms: ConfigManager.Config.Sms.Region = region; break;
								case RomFormat.GameGear: ConfigManager.Config.Sms.GameGearRegion = region; break;
								case RomFormat.ColecoVision: ConfigManager.Config.Cv.Region = region; break;
							}
							ConfigManager.Config.Sms.ApplyConfig();
							ConfigManager.Config.Cv.ApplyConfig();
							break;

						default:
							break;
					}
				}
			};
		}

		private MainMenuAction GetPcEngineModelMenuItem(PceConsoleType model)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetEnumText(model),
				IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.PcEngine,
				IsSelected = () => ConfigManager.Config.PcEngine.ConsoleType == model,
				OnClick = () => {
					ConfigManager.Config.PcEngine.ConsoleType = model;
					ConfigManager.Config.PcEngine.ApplyConfig();
				}
			};
		}

		private MainMenuAction GetWsModelMenuItem(WsModel model)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetEnumText(model),
				IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Ws,
				IsSelected = () => ConfigManager.Config.Ws.Model == model,
				OnClick = () => {
					ConfigManager.Config.Ws.Model = model;
					ConfigManager.Config.Ws.ApplyConfig();
				}
			};
		}

		private MainMenuAction GetGameboyModelMenuItem(GameboyModel model)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetEnumText(model),
				IsSelected = () => ConfigManager.Config.Gameboy.Model == model,
				OnClick = () => {
					ConfigManager.Config.Gameboy.Model = model;
					ConfigManager.Config.Gameboy.ApplyConfig();
				}
			};
		}

		private bool AllowFilterType(VideoFilterType filter)
		{
			switch(filter) {
				case VideoFilterType.NtscBisqwit: return MainWindow.RomInfo.ConsoleType == ConsoleType.Nes;
				default: return true;
			}
		}

		private MainMenuAction GetVideoFilterMenuItem(VideoFilterType filter)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetEnumText(filter),
				IsEnabled = () => AllowFilterType(filter),
				IsSelected = () => {
					ConsoleOverrideConfig? overrides = ConsoleOverrideConfig.GetActiveOverride();
					if(overrides?.OverrideVideoFilter == true) {
						return filter == overrides.VideoFilter;
					}
					return filter == ConfigManager.Config.Video.VideoFilter;
				},
				OnClick = () => {
					ConsoleOverrideConfig? overrides = ConsoleOverrideConfig.GetActiveOverride();
					if(overrides?.OverrideVideoFilter == true) {
						overrides.VideoFilter = filter;
					} else {
						ConfigManager.Config.Video.VideoFilter = filter;
					}
					ConfigManager.Config.Video.ApplyConfig();
				}
			};
		}

		private MainMenuAction GetScaleMenuItem(int scale, EmulatorShortcut shortcut)
		{
			return new MainMenuAction(shortcut) {
				ActionType = ActionType.Custom,
				CustomText = scale + "x",
				IsSelected = () => (int)((double)MainWindow.RendererSize.Height / EmuApi.GetBaseScreenSize().Height) == scale
			};
		}

		private MainMenuAction GetSpeedMenuItem(ActionType action, int speed, EmulatorShortcut? shortcut = null)
		{
			MainMenuAction item = new MainMenuAction(shortcut) {
				ActionType = action,
				IsSelected = () => ConfigManager.Config.Emulation.EmulationSpeed == speed,
			};

			if(shortcut == null) {
				item.OnClick = () => {
					ConfigManager.Config.Emulation.EmulationSpeed = (uint)speed;
					ConfigManager.Config.Emulation.ApplyConfig();
				};
			}

			return item;
		}

		private MainMenuAction GetMoviesMenu(MainWindow wnd)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Movies,
				SubActions = new List<object> {
					new MainMenuAction() {
						ActionType = ActionType.Play,
						IsEnabled = () => IsGameRunning && !RecordApi.MovieRecording() && !RecordApi.MoviePlaying(),
						OnClick = async () => {
							string? filename = await FileDialogHelper.OpenFile(ConfigManager.MovieFolder, wnd, FileDialogHelper.MesenMovieExt);
							if(filename != null) {
								RecordApi.MoviePlay(filename);
							}
						}
					},
					new MainMenuAction() {
						ActionType = ActionType.Record,
						IsEnabled = () => IsGameRunning && !RecordApi.MovieRecording() && !RecordApi.MoviePlaying(),
						OnClick = () => {
							new MovieRecordWindow() {
								DataContext = new MovieRecordConfigViewModel()
							}.ShowCenteredDialog((Control)wnd);
						}
					},
					new MainMenuAction() {
						ActionType = ActionType.Stop,
						IsEnabled = () => IsGameRunning && (RecordApi.MovieRecording() || RecordApi.MoviePlaying()),
						OnClick = () => {
							RecordApi.MovieStop();
						}
					}
				}
			};
		}

		private void InitToolMenu(MainWindow wnd)
		{
			ToolsMenuItems = new List<object>() {
				new MainMenuAction() {
					ActionType = ActionType.Cheats,
					IsEnabled = () => IsGameRunning && MainWindow.RomInfo.ConsoleType.SupportsCheats(),
					OnClick = () => {
						ApplicationHelper.GetOrCreateUniqueWindow(wnd, () => new CheatListWindow());
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.HistoryViewer,
					IsEnabled = () => IsGameRunning && HistoryApi.HistoryViewerEnabled(),
					OnClick = () => {
						ApplicationHelper.GetOrCreateUniqueWindow(null, () => new HistoryViewerWindow());
					}
				},

				GetMoviesMenu(wnd),
				GetNetPlayMenu(wnd),

				new ContextMenuSeparator(),

				GetSoundRecorderMenu(wnd),
				GetVideoRecorderMenu(wnd),

				new ContextMenuSeparator() {
					IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Nes
				},

				new MainMenuAction() {
					ActionType = ActionType.HdPacks,
					IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Nes,
					SubActions = new List<object> {
						new MainMenuAction() {
							ActionType = ActionType.InstallHdPack,
							OnClick = () => InstallHdPack(wnd)
						},
						new MainMenuAction() {
							ActionType = ActionType.HdPackBuilder,
							OnClick = () => {
								ApplicationHelper.GetOrCreateUniqueWindow(wnd, () => new HdPackBuilderWindow());
							}
						}
					}
				},

				new ContextMenuSeparator(),
				
				new MainMenuAction() {
					ActionType = ActionType.LogWindow,
					OnClick = () => {
						ApplicationHelper.GetOrCreateUniqueWindow(wnd, () => new LogWindow());
					}
				},

				new MainMenuAction(EmulatorShortcut.TakeScreenshot) {
					ActionType = ActionType.TakeScreenshot,
				},
			};
		}

		private MainMenuAction GetVideoRecorderMenu(MainWindow wnd)
		{
			return new MainMenuAction() {
				ActionType = ActionType.VideoRecorder,
				SubActions = new List<object> {
					new MainMenuAction() {
						ActionType = ActionType.Record,
						IsEnabled = () => IsGameRunning && !RecordApi.AviIsRecording(),
						OnClick = () => {
							new VideoRecordWindow() {
								DataContext = new VideoRecordConfigViewModel()
							}.ShowCenteredDialog((Control)wnd);
						}
					},
					new MainMenuAction() {
						ActionType = ActionType.Stop,
						IsEnabled = () => IsGameRunning && RecordApi.AviIsRecording(),
						OnClick = () => {
							RecordApi.AviStop();
						}
					}
				}
			};
		}

		private MainMenuAction GetSoundRecorderMenu(MainWindow wnd)
		{
			return new MainMenuAction() {
				ActionType = ActionType.SoundRecorder,
				SubActions = new List<object> {
					new MainMenuAction() {
						ActionType = ActionType.Record,
						IsEnabled = () => IsGameRunning && !RecordApi.WaveIsRecording(),
						OnClick = async () => {
							string? filename = await FileDialogHelper.SaveFile(ConfigManager.WaveFolder, EmuApi.GetRomInfo().GetRomName() + ".wav", wnd, FileDialogHelper.WaveExt);
							if(filename != null) {
								RecordApi.WaveRecord(filename);
							}
						}
					},
					new MainMenuAction() {
						ActionType = ActionType.Stop,
						IsEnabled = () => IsGameRunning && RecordApi.WaveIsRecording(),
						OnClick = () => {
							RecordApi.WaveStop();
						}
					}
				}
			};
		}

		private MainMenuAction GetNetPlayMenu(MainWindow wnd)
		{
			_selectControllerAction = new MainMenuAction() {
				ActionType = ActionType.SelectController,
				IsEnabled = () => NetplayApi.IsConnected() || NetplayApi.IsServerRunning(),
				SubActions = _netPlayControllers
			};

			return new MainMenuAction() {
				ActionType = ActionType.NetPlay,
				SubActions = new List<object> {
					new MainMenuAction() {
						ActionType = ActionType.Connect,
						IsEnabled = () => !NetplayApi.IsConnected() && !NetplayApi.IsServerRunning(),
						OnClick = () => {
							new NetplayConnectWindow() {
								DataContext = ConfigManager.Config.Netplay.Clone()
							}.ShowCenteredDialog((Control)wnd);
						}
					},

					new MainMenuAction() {
						ActionType = ActionType.Disconnect,
						IsEnabled = () => NetplayApi.IsConnected(),
						OnClick = () => {
							NetplayApi.Disconnect();
						}
					},

					new ContextMenuSeparator(),

					new MainMenuAction() {
						ActionType = ActionType.StartServer,
						IsEnabled = () => !NetplayApi.IsConnected() && !NetplayApi.IsServerRunning(),
						OnClick = () => {
							new NetplayStartServerWindow() {
								DataContext = ConfigManager.Config.Netplay.Clone()
							}.ShowCenteredDialog((Control)wnd);
						}
					},

					new MainMenuAction() {
						ActionType = ActionType.StopServer,
						IsEnabled = () => NetplayApi.IsServerRunning(),
						OnClick = () => {
							NetplayApi.StopServer();
						}
					},

					new ContextMenuSeparator(),

					_selectControllerAction
				}
			};
		}

		private void InitDebugMenu(Window wnd)
		{
			Func<bool> isSuperGameBoy = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Snes && MainWindow.RomInfo.Format == RomFormat.Gb;
			Func<bool> isNesFormat = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Nes && (MainWindow.RomInfo.Format == RomFormat.iNes || MainWindow.RomInfo.Format == RomFormat.VsSystem || MainWindow.RomInfo.Format == RomFormat.VsDualSystem);

			DebugMenuItems = new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugger),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebuggerWindow.GetOrOpenWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType())
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSpcDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSpcDebugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Spc),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.Spc)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenCx4Debugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenCx4Debugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Cx4),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.Cx4)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenNecDspDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenNecDspDebugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.NecDsp),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.NecDsp)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenGsuDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenGsuDebugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Gsu),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.Gsu)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSa1Debugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSa1Debugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Sa1),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.Sa1)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSt018Debugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSt018Debugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.St018),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.St018)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenGameboyDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenGameboyDebugger),
					IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Snes && MainWindow.RomInfo.CpuTypes.Contains(CpuType.Gameboy),
					OnClick = () => DebuggerWindow.GetOrOpenWindow(CpuType.Gameboy)
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenEventViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenEventViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => EventViewerWindow.GetOrOpenWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType())
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenMemoryTools,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenMemoryTools),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new MemoryToolsWindow())
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenRegisterViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenRegisterViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new RegisterViewerWindow(new RegisterViewerWindowViewModel()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenTraceLogger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenTraceLogger),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.GetOrOpenDebugWindow(() => new TraceLoggerWindow(new TraceLoggerViewModel()))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenTilemapViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenTilemapViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TilemapViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenTileViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenTileViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TileViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSpriteViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSpriteViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new SpriteViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenPaletteViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenPaletteViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new PaletteViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenAssembler,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenAssembler),
					IsEnabled = () => IsGameRunning && MainWindow.RomInfo.ConsoleType.GetMainCpuType().SupportsAssembler(),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new AssemblerWindow(new AssemblerWindowViewModel(MainWindow.RomInfo.ConsoleType.GetMainCpuType())))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugLog,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugLog),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.GetOrOpenDebugWindow(() => new DebugLogWindow())
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenMemorySearch,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenMemorySearch),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.GetOrOpenDebugWindow(() => new MemorySearchWindow())
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenProfiler,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenProfiler),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.GetOrOpenDebugWindow(() => new ProfilerWindow())
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenScriptWindow,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenScriptWindow),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new ScriptWindow(new ScriptWindowViewModel(null)))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenWatchWindow,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenWatchWindow),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.GetOrOpenDebugWindow(() => new WatchWindow(new WatchWindowViewModel()))
				},
				new ContextMenuSeparator() { IsVisible = isSuperGameBoy },
				new ContextMenuAction() {
					ActionType = ActionType.OpenTilemapViewer,
					HintText = () => "GB",
					IsVisible = isSuperGameBoy,
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TilemapViewerWindow(CpuType.Gameboy))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenTileViewer,
					HintText = () => "GB",
					IsVisible = isSuperGameBoy,
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TileViewerWindow(CpuType.Gameboy))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSpriteViewer,
					HintText = () => "GB",
					IsVisible = isSuperGameBoy,
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new SpriteViewerWindow(CpuType.Gameboy))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenPaletteViewer,
					HintText = () => "GB",
					IsVisible = isSuperGameBoy,
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new PaletteViewerWindow(CpuType.Gameboy))
				},

				new ContextMenuSeparator() { IsVisible = isSuperGameBoy },
				new ContextMenuAction() {
					ActionType = ActionType.OpenEventViewer,
					HintText = () => "GB",
					IsVisible = isSuperGameBoy,
					IsEnabled = () => IsGameRunning,
					OnClick = () => EventViewerWindow.GetOrOpenWindow(CpuType.Gameboy)
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenAssembler,
					HintText = () => "GB",
					IsVisible = isSuperGameBoy,
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new AssemblerWindow(new AssemblerWindowViewModel(CpuType.Gameboy)))
				},

				new ContextMenuSeparator() { IsVisible = isNesFormat },
				new ContextMenuAction() {
					ActionType = ActionType.OpenNesHeaderEditor,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenNesHeaderEditor),
					IsVisible = isNesFormat,
					OnClick = () => new NesHeaderEditWindow().ShowCenteredDialog((Control)wnd)
				},

				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugSettings,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugSettings),
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.Debugger, wnd)
				}
			};

			DebugShortcutManager.RegisterActions(wnd, DebugMenuItems);
		}

		private void InitHelpMenu(Window wnd)
		{
			HelpMenuItems = new List<object>() {
				new MainMenuAction() {
					ActionType = ActionType.OnlineHelp,
					IsVisible = () => false,
					OnClick = () => ApplicationHelper.OpenBrowser("https://www.mesen.ca/documentation/")
				},
				new MainMenuAction() {
					ActionType = ActionType.CommandLineHelp,
					OnClick = () => { new CommandLineHelpWindow().ShowCenteredDialog((Control)wnd); }
				},
				new MainMenuAction() {
					ActionType = ActionType.CheckForUpdates,
					OnClick = () => CheckForUpdate(wnd, false)
				},
				new MainMenuAction() {
					ActionType = ActionType.ReportBug,
					IsVisible = () => false,
					OnClick = () => ApplicationHelper.OpenBrowser("https://www.mesen.ca/reportbug/")
				},
				new ContextMenuSeparator(),
				new MainMenuAction() {
					ActionType = ActionType.About,
					OnClick = () => {
						new AboutWindow().ShowCenteredDialog((Control)wnd);
					}
				},
			};
		}

		public void CheckForUpdate(Window mainWindow, bool silent)
		{
			Task.Run(async () => {
				UpdatePromptViewModel? updateInfo = await UpdatePromptViewModel.GetUpdateInformation(silent);
				if(updateInfo == null) {
					if(!silent) {
						Dispatcher.UIThread.Post(() => {
							MesenMsgBox.Show(null, "UpdateDownloadFailed", MessageBoxButtons.OK, MessageBoxIcon.Info);
						});
					}
					return;
				}

				if(updateInfo.LatestVersion > updateInfo.InstalledVersion) {
					Dispatcher.UIThread.Post(async () => {
						UpdatePromptWindow updatePrompt = new UpdatePromptWindow(updateInfo);
						if(await updatePrompt.ShowCenteredDialog<bool>(mainWindow) == true) {
							mainWindow.Close();
						}
					});
				} else if(!silent) {
					Dispatcher.UIThread.Post(() => {
						 MesenMsgBox.Show(null, "MesenUpToDate", MessageBoxButtons.OK, MessageBoxIcon.Info);
					});
				}
			});
		}

		private async void InstallHdPack(Window wnd)
		{
			string? filename = await FileDialogHelper.OpenFile(null, wnd, FileDialogHelper.ZipExt);
			if(filename == null) {
				return;
			}

			try {
				using(FileStream? stream = FileHelper.OpenRead(filename)) {
					if(stream == null) {
						return;
					}

					ZipArchive zip = new ZipArchive(stream);

					//Find the hires.txt file
					ZipArchiveEntry? hiresEntry = null;

					//Find the most top-level hires.txt file in the zip
					int minDepth = int.MaxValue;
					foreach(ZipArchiveEntry entry in zip.Entries) {
						if(entry.Name == "hires.txt") {
							string? folder = Path.GetDirectoryName(entry.FullName);
							int depth = 0;
							if(folder != null) {
								do {
									depth++;
								} while((folder = Path.GetDirectoryName(folder)) != null);
							}
							if(depth < minDepth) {
								minDepth = depth;
								hiresEntry = entry;
								if(depth == 0) {
									break;
								}
							}
						}
					}

					if(hiresEntry == null) {
						await MesenMsgBox.Show(wnd, "InstallHdPackInvalidPack", MessageBoxButtons.OK, MessageBoxIcon.Error);
						return;
					}

					using Stream entryStream = hiresEntry.Open();
					using StreamReader reader = new StreamReader(entryStream);
					string hiresData = reader.ReadToEnd();
					RomInfo romInfo = EmuApi.GetRomInfo();

					//If there's a "supportedRom" tag, check if it matches the current ROM
					Regex supportedRomRegex = new Regex("<supportedRom>([^\\n]*)");
					Match match = supportedRomRegex.Match(hiresData);
					if(match.Success) {
						if(!match.Groups[1].Value.ToUpper().Contains(EmuApi.GetRomHash(HashType.Sha1).ToUpper())) {
							await MesenMsgBox.Show(wnd, "InstallHdPackWrongRom", MessageBoxButtons.OK, MessageBoxIcon.Error);
							return;
						}
					}

					//Extract HD pack
					try {
						string targetFolder = Path.Combine(ConfigManager.HdPackFolder, romInfo.GetRomName());
						if(Directory.Exists(targetFolder)) {
							//Warn if the folder already exists
							if(await MesenMsgBox.Show(wnd, "InstallHdPackConfirmOverwrite", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, targetFolder) != DialogResult.OK) {
								return;
							}
						} else {
							Directory.CreateDirectory(targetFolder);
						}

						string hiresFileFolder = hiresEntry.FullName.Substring(0, hiresEntry.FullName.Length - "hires.txt".Length);
						foreach(ZipArchiveEntry entry in zip.Entries) {
							//Extract only the files in the same subfolder as the hires.txt file (and only if they have a name & size > 0)
							if(!string.IsNullOrWhiteSpace(entry.Name) && entry.Length > 0 && entry.FullName.StartsWith(hiresFileFolder)) {
								string filePath = Path.Combine(targetFolder, entry.FullName.Substring(hiresFileFolder.Length));
								string? fileFolder = Path.GetDirectoryName(filePath);
								if(fileFolder != null) {
									Directory.CreateDirectory(fileFolder);
								}
								entry.ExtractToFile(filePath, true);
							}
						}
					} catch(Exception ex) {
						await MesenMsgBox.Show(wnd, "InstallHdPackError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
						return;
					}

					//Turn on HD Pack support automatically after installation succeeds
					if(!ConfigManager.Config.Nes.EnableHdPacks) {
						ConfigManager.Config.Nes.EnableHdPacks = true;
						ConfigManager.Config.Nes.ApplyConfig();
					}

					if(await MesenMsgBox.Show(wnd, "InstallHdPackConfirmReset", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.OK) {
						//Power cycle game if the user agrees
						LoadRomHelper.PowerCycle();
					}
				}
			} catch {
				//Invalid file (file missing, not a zip file, etc.)
				await MesenMsgBox.Show(wnd, "InstallHdPackInvalidZipFile", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}

		public bool UpdateNetplayMenu()
		{
			if(!NetplayApi.IsServerRunning() && !NetplayApi.IsConnected()) {
				return false;
			}

			List<object> controllerActions = new();

			NetplayControllerInfo clientPort = NetplayApi.NetPlayGetControllerPort();

			int playerIndex = 1;
			NetplayControllerUsageInfo[] controllers = NetplayApi.NetPlayGetControllerList();

			for(int i = 0; i < controllers.Length; i++) {
				NetplayControllerUsageInfo controller = controllers[i];
				if(controller.Type == ControllerType.None) {
					//Skip this controller/port (when type is none, the "port" for hardware buttons, etc.)
					continue;
				}

				MainMenuAction action = new();
				action.ActionType = ActionType.Custom;
				action.CustomText = ResourceHelper.GetMessage("Player") + " " + playerIndex + " (" + ResourceHelper.GetEnumText(controller.Type) + ")";
				action.IsSelected = () => controller.Port.Port == clientPort.Port && controller.Port.SubPort == clientPort.SubPort;
				action.IsEnabled = () => !controller.InUse;
				action.OnClick = () => NetplayApi.NetPlaySelectController(controller.Port);
				controllerActions.Add(action);
				playerIndex++;
			}

			if(controllerActions.Count > 0) {
				controllerActions.Add(new ContextMenuSeparator());
			}

			controllerActions.Add(new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetEnumText(ControllerType.None),
				IsSelected = () => clientPort.Port == 0xFF,
				OnClick = () => NetplayApi.NetPlaySelectController(new NetplayControllerInfo() { Port = 0xFF })
			});

			_selectControllerAction.SubActions = controllerActions;

			return true;
		}
	}
}
