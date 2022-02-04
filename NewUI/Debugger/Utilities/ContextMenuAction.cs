using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Controls;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Reactive;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public abstract class BaseMenuAction : ViewModelBase, IDisposable
	{
		public ActionType ActionType;
		public string? CustomText { get; set; }

		public virtual string Name
		{
			get
			{
				if(ActionType == ActionType.Custom) {
					return CustomText ?? "";
				}

				string label = ResourceHelper.GetEnumText(ActionType);
				if(HintText != null) {
					label += " (" + HintText() + ")";
				}
				return label;
			}
		}

		public Image? Icon
		{
			get
			{
				IconFileAttribute? attr = ActionType.GetAttribute<IconFileAttribute>();
				if(!string.IsNullOrEmpty(attr?.Icon)) {
					return ImageUtilities.FromAsset(attr.Icon);
				} else if(IsSelected?.Invoke() == true) {
					return ImageUtilities.FromAsset("Assets/MenuItemChecked.png");
				}
				return null;
			}
		}

		List<object>? _subActions;
		public List<object>? SubActions
		{
			get => _subActions;
			set
			{
				_subActions = value;

				if(_subActions != null) {
					IsEnabled = () => {
						foreach(object subAction in _subActions) {
							if(subAction is BaseMenuAction act) {
								if(act.IsEnabled == null || act.IsEnabled()) {
									return true;
								}
							}
						}
						return false;
					};
				}
			}
		}

		public Func<string>? HintText { get; set; }
		public Func<bool>? IsEnabled { get; set; }
		public Func<bool>? IsSelected { get; set; }
		public Func<bool>? IsVisible { get; set; }

		public abstract string ShortcutText { get; }

		[Reactive] public bool Enabled { get; set; }
		[Reactive] public bool Visible { get; set; }

		private ReactiveCommand<Unit, Unit>? _clickCommand;
		public ReactiveCommand<Unit, Unit>? ClickCommand
		{
			get
			{
				Visible = IsVisible?.Invoke() ?? true;
				Enabled = IsEnabled?.Invoke() ?? true;
				return _clickCommand;
			}
		}

		private Action _onClick = () => { };
		public Action OnClick
		{
			get => _onClick;
			set
			{
				_onClick = () => {
					if((IsVisible == null || IsVisible()) && (IsEnabled == null || IsEnabled())) {
						if(ActionType == ActionType.Exit) {
							//When using exit, the command is disposed while the command is running, which causes a crash
							//Run the code in a posted action to prevent the crash
							Dispatcher.UIThread.Post(() => { value(); });
						} else {
							value();
						}
					}
				};
				_clickCommand = ReactiveCommand.Create(_onClick, this.WhenAnyValue(x => x.Enabled));
			}
		}

		public void Update()
		{
			Enabled = IsEnabled?.Invoke() ?? true;
			Visible = IsVisible?.Invoke() ?? true;
		}

		public virtual void Dispose()
		{
			_onClick = () => { };
			_clickCommand?.Dispose();
			_clickCommand = null;
			IsSelected = null;
			IsEnabled = null;
			IsVisible = null;
			if(_subActions != null) {
				foreach(object subAction in _subActions) {
					if(subAction is BaseMenuAction action) {
						action.Dispose();
					}
				}
			}
		}
	}

	public class MainMenuAction : BaseMenuAction
	{
		public EmulatorShortcut? Shortcut { get; set; }
		public uint ShortcutParam { get; set; }

		public MainMenuAction()
		{
		}

		public MainMenuAction(EmulatorShortcut shortcut)
		{
			Shortcut = shortcut;

			IsEnabled = () => EmuApi.IsShortcutAllowed(shortcut);

			OnClick = () => {
				//Run outside the UI thread to avoid deadlocks, etc.
				Task.Run(() => {
					EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = shortcut, Param = ShortcutParam });
				});
			};
		}

		public override string ShortcutText
		{
			get
			{
				return Shortcut.HasValue ? ShortcutMenuItem.GetShortcutKeys(Shortcut.Value)?.ToString() ?? "" : "";
			}
		}
	}

	public class ContextMenuAction : BaseMenuAction
	{
		public Func<DbgShortKeys>? Shortcut { get; set; }
		public override string ShortcutText => Shortcut?.Invoke().ToString() ?? "";

		public override void Dispose()
		{
			base.Dispose();
			Shortcut = null;
		}
	}

	public class ContextMenuSeparator : ContextMenuAction
	{
		public override string Name => "-";
	}

	public enum ActionType
	{
		Custom,

		[IconFile("Copy")]
		Copy,

		[IconFile("Paste")]
		Paste,

		[IconFile("SelectAll")]
		SelectAll,

		[IconFile("EditLabel")]
		EditLabel,

		[IconFile("Add")]
		AddWatch,

		[IconFile("BreakpointEnableDisable")]
		EditBreakpoint,

		MarkSelectionAs,

		[IconFile("Accept")]
		MarkAsCode,

		[IconFile("CheatCode")]
		MarkAsData,
		
		[IconFile("Help")]
		MarkAsUnidentified,

		[IconFile("Add")]
		Add,

		[IconFile("Edit")]
		Edit,

		[IconFile("Close")]
		Delete,

		[IconFile("Breakpoint")]
		AddBreakpoint,

		[IconFile("MoveUp")]
		MoveUp,

		[IconFile("MoveDown")]
		MoveDown,

		WatchDecimalDisplay,
		WatchHexDisplay,
		WatchBinaryDisplay,
		
		RowDisplayFormat,
		RowFormatBinary,
		RowFormatHex8Bits,
		RowFormatHex16Bits,
		RowFormatHex24Bits,
		RowFormatSigned8Bits,
		RowFormatSigned16Bits,
		RowFormatSigned24Bits,
		RowFormatUnsigned,

		[IconFile("Close")]
		ClearFormat,

		[IconFile("Import")]
		Import,

		[IconFile("Export")]
		Export,

		ShowConsoleStatus,
		ShowBreakpointList,
		ShowWatchList,
		ShowLabelList,
		ShowCallStack,

		ShowSettingsPanel,
		ShowMemoryMappings,

		[IconFile("Settings")]
		Preferences,

		[IconFile("MediaPlay")]
		Continue,

		[IconFile("MediaPause")]
		Break,

		[IconFile("StepInto")]
		StepInto,
		
		[IconFile("StepOver")]
		StepOver,
		
		[IconFile("StepOut")]
		StepOut,

		[IconFile("StepBack")]
		StepBack,

		[IconFile("RunPpuCycle")]
		RunPpuCycle,

		[IconFile("RunPpuScanline")]
		RunPpuScanline,

		[IconFile("RunPpuFrame")]
		RunPpuFrame,

		BreakIn,
		BreakOn,

		[IconFile("Refresh")]
		Reset,

		[IconFile("PowerCycle")]
		PowerCycle,

		[IconFile("Script")]
		NewScript,

		[IconFile("MediaPlay")]
		RunScript,

		[IconFile("MediaStop")]
		StopScript,

		[IconFile("Folder")]
		Open,

		[IconFile("SaveFloppy")]
		Save,
		
		SaveAs,

		[IconFile("Exit")]
		Exit,

		[IconFile("Help")]
		HelpApiReference,

		RecentScripts,

		[IconFile("Refresh")]
		Refresh,
		EnableAutoRefresh,
		RefreshOnBreakPause,
		
		ZoomIn,
		ZoomOut,

		[IconFile("Expand")]
		GoToLocation,

		[IconFile("Export")]
		ExportToPng,

		[IconFile("VerticalLayout")]
		ViewInTileViewer,

		[IconFile("CheatCode")]
		ViewInMemoryViewer,
		
		LoadTblFile,
		ResetTblMappings,
		GoToAddress,
		GoToAll,

		[IconFile("Debugger")]
		OpenDebugger,

		[IconFile("EventViewer")]
		OpenEventViewer,

		[IconFile("CheatCode")]
		OpenMemoryTools,

		[IconFile("RegisterIcon")]
		OpenRegisterViewer,

		[IconFile("LogWindow")]
		OpenTraceLogger,

		[IconFile("VideoOptions")]
		OpenTilemapViewer,

		[IconFile("VerticalLayout")]
		OpenTileViewer,

		[IconFile("PerfTracker")]
		OpenSpriteViewer,

		[IconFile("VideoFilter")]
		OpenPaletteViewer,

		[IconFile("Chip")]
		OpenAssembler,

		[IconFile("Speed")]
		OpenProfiler,

		[IconFile("Script")]
		OpenScriptWindow,

		[IconFile("Settings")]
		OpenDebugSettings,

		[IconFile("SpcDebugger")]
		OpenSpcDebugger,
		[IconFile("Cx4Debugger")]
		OpenCx4Debugger,
		[IconFile("NecDspDebugger")]
		OpenNecDspDebugger,
		[IconFile("GsuDebugger")]
		OpenGsuDebugger,
		[IconFile("Sa1Debugger")]
		OpenSa1Debugger,
		[IconFile("GameboyDebugger")]
		OpenGameboyDebugger,

		[IconFile("CheatCode")]
		Cheats,
		[IconFile("HistoryViewer")]
		HistoryViewer,
		[IconFile("Movie")]
		Movies,
		[IconFile("MediaPlay")]
		Play,
		[IconFile("Record")]
		Record,
		[IconFile("MediaStop")]
		Stop,
		
		[IconFile("Network")]
		NetPlay,
		Connect,
		Disconnect,
		StartServer,
		StopServer,

		[IconFile("Microphone")]
		SoundRecorder,
		[IconFile("VideoRecorder")]
		VideoRecorder,

		[IconFile("HdPack")]
		HdPacks,
		[IconFile("Import")]
		InstallHdPack,
		[IconFile("HdPack")]
		HdPackBuilder,

		[IconFile("LogWindow")]
		LogWindow,

		[IconFile("Camera")]
		TakeScreenshot,

		[IconFile("Help")]
		OnlineHelp,
		[IconFile("Update")]
		CheckForUpdates,
		[IconFile("Exclamation")]
		About,
		[IconFile("Comment")]
		ReportBug,
		SelectController,
	}
}
