using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Reactive;

namespace Mesen.Debugger.Utilities
{
	public class ContextMenuAction : ViewModelBase, IDisposable
	{
		public ActionType ActionType;
		public string? CustomText { get; set; }

		public string Name
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
							if(subAction is ContextMenuAction act) {
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

		public Func<DbgShortKeys>? Shortcut { get; set; }
		public string ShortcutText => Shortcut?.Invoke().ToString() ?? "";

		[Reactive] public bool Enabled { get; set; }

		private ReactiveCommand<Unit, Unit>? _clickCommand;
		public ReactiveCommand<Unit, Unit>? ClickCommand
		{
			get
			{
				Enabled = IsEnabled?.Invoke() ?? true;
				return _clickCommand;
			}
		}

		private Action _onClick = () => { };
		public Action OnClick
		{
			get => _onClick;
			set {
				_onClick = () => {
					if(IsEnabled == null || IsEnabled()) {
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
		}

		public void Dispose()
		{
			_onClick = () => { };
			_clickCommand?.Dispose();
			_clickCommand = null;
			Shortcut = null;
			IsSelected = null;
			IsEnabled = null;
			if(_subActions != null) {
				foreach(object subAction in _subActions) {
					if(subAction is ContextMenuAction action) {
						action.Dispose();
					}
				}
			}
		}
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
	}
}
