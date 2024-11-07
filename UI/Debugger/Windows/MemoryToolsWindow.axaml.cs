using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System.ComponentModel;
using Avalonia.Interactivity;
using Mesen.Debugger.Utilities;
using System.IO;
using Mesen.Utilities;
using Mesen.Config;
using Mesen.Debugger.Labels;
using System.Linq;
using System.Collections.Generic;
using Mesen.Localization;
using Avalonia.Input;
using Mesen.Debugger.Disassembly;

namespace Mesen.Debugger.Windows
{
	public class MemoryToolsWindow : MesenWindow, INotificationHandler
	{
		private HexEditor _editor;
		private MemoryToolsViewModel _model;
		private MemoryViewerFindWindow? _searchWnd;
		private Point _prevMousePos;
		private int _prevByteOffset;
		private DynamicTooltip? _prevTooltip;

		public MemoryToolsWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_editor = this.GetControl<HexEditor>("Hex");
			_model = new MemoryToolsViewModel(_editor);
			DataContext = _model;

			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
			_editor.ByteUpdated += editor_ByteUpdated;
			_editor.PointerMoved += editor_PointerMoved;
		}

		public static void ShowInMemoryTools(MemoryType memType, int address)
		{
			MemoryToolsWindow wnd = DebugWindowManager.GetOrOpenDebugWindow(() => new MemoryToolsWindow());
			wnd.SetCursorPosition(memType, address);
		}

		public void SetCursorPosition(MemoryType memType, int address)
		{
			if(_model.AvailableMemoryTypes.Contains(memType)) {
				_model.Config.MemoryType = memType;
				_editor.SetCursorPosition(address, scrollToTop: true);
				_editor.Focus();
			}
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
			ConfigManager.Config.Debug.HexEditor = _model.Config;
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			InitializeActions();
			_editor.Focus();
		}

		protected override void OnGotFocus(GotFocusEventArgs e)
		{
			base.OnGotFocus(e);
			if(FocusManager?.GetFocusedElement() == this) {
				//Focus on editor whenever the window itself is focused
				_editor.Focus();
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowOptionPanel = !_model.Config.ShowOptionPanel;
		}

		private void editor_PointerMoved(object? sender, PointerEventArgs e)
		{
			Point point = e.GetPosition(_editor);
			if(point == _prevMousePos) {
				return;
			}
			_prevMousePos = point;

			int byteOffset = _model.Config.ShowTooltips ? _editor.GetByteOffset(point) : -1;
			if(byteOffset >= 0) {
				if(byteOffset != _prevByteOffset) {
					CpuType cpuType = _model.Config.MemoryType.ToCpuType();
					
					AddressInfo addr = new AddressInfo() { Address = byteOffset, Type = _model.Config.MemoryType };
					AddressInfo relAddr;
					AddressInfo absAddr;
					if(addr.Type.IsRelativeMemory()) {
						relAddr = addr;
						absAddr = DebugApi.GetAbsoluteAddress(relAddr);
					} else {
						absAddr = addr;
						relAddr = DebugApi.GetRelativeAddress(absAddr, cpuType);
					}

					CodeLabel? label = LabelManager.GetLabel(addr);
					LocationInfo locInfo = new() {
						RelAddress = relAddr.Address >= 0 ? relAddr : null,
						AbsAddress = absAddr.Address >= 0 ? absAddr : null,
						Label = label,
						LabelAddressOffset = label?.Length > 1 && absAddr.Address >= label.Address ? (absAddr.Address - (int)label.Address) : null
					};

					_prevTooltip = CodeTooltipHelper.GetCodeAddressTooltip(cpuType, locInfo, !addr.Type.IsRelativeMemory());
					TooltipHelper.ShowTooltip(_editor, _prevTooltip, 15);
				} else {
					TooltipHelper.ShowTooltip(_editor, _prevTooltip, 15);
				}
			} else {
				_prevTooltip = null;
				TooltipHelper.HideTooltip(_editor);
			}

			_prevByteOffset = byteOffset;
		}

		private void editor_ByteUpdated(object? sender, ByteUpdatedEventArgs e)
		{
			if(e.Values != null) {
				DebugApi.SetMemoryValues(_model.Config.MemoryType, (uint)e.ByteOffset, e.Values, e.Values.Length);
			} else if(e.Value != null) {
				if(e.Length > 0) {
					byte[] data = new byte[e.Length];
					Array.Fill<byte>(data, e.Value.Value);
					DebugApi.SetMemoryValues(_model.Config.MemoryType, (uint)e.ByteOffset, data, data.Length);
				} else {
					DebugApi.SetMemoryValue(_model.Config.MemoryType, (uint)e.ByteOffset, e.Value.Value);
				}
			}
		}

		private void InitializeActions()
		{
			DebugConfig cfg = ConfigManager.Config.Debug;

			_model.AddDisposables(DebugShortcutManager.CreateContextMenu(_editor, new ContextMenuAction[] {
				GetMarkSelectionAction(),
				new ContextMenuSeparator(),
				GetAddWatchAction(),
				GetEditBreakpointAction(),
				GetEditLabelAction(),
				new ContextMenuSeparator(),
				GetViewInDebuggerAction(),
				GetViewInMemoryAction(),
				new ContextMenuSeparator() { IsVisible = () => _model.Config.MemoryType.SupportsFreezeAddress() },
				GetFreezeAction(ActionType.FreezeMemory, DebuggerShortcut.MemoryViewer_Freeze),
				GetFreezeAction(ActionType.UnfreezeMemory, DebuggerShortcut.MemoryViewer_Unfreeze),
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Undo,
					IsEnabled = () => DebugApi.HasUndoHistory(),
					Shortcut = () => cfg.Shortcuts.Get(DebuggerShortcut.Undo),
					OnClick = () => {
						if(DebugApi.HasUndoHistory()){
							DebugApi.PerformUndo();
							_editor.InvalidateVisual();
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Copy,
					IsEnabled = () => _editor.SelectionLength > 0,
					OnClick = () => _editor.CopySelection(),
					Shortcut = () => cfg.Shortcuts.Get(DebuggerShortcut.Copy)
				},
				new ContextMenuAction() {
					ActionType = ActionType.Paste,
					OnClick = () => _editor.PasteSelection(),
					Shortcut = () => cfg.Shortcuts.Get(DebuggerShortcut.Paste)
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.SelectAll,
					OnClick = () => _editor.SelectAll(),
					Shortcut = () => cfg.Shortcuts.Get(DebuggerShortcut.SelectAll)
				},
			}));

			_model.FileMenuItems = _model.AddDisposables(new List<ContextMenuAction>() {
				GetImportAction(),
				GetExportAction(),
				new ContextMenuSeparator(),
				SaveRomActionHelper.GetSaveRomAction(this),
				SaveRomActionHelper.GetSaveRomAsAction(this),
				SaveRomActionHelper.GetSaveEditsAsIpsAction(this),
				new ContextMenuSeparator(),
				GetResetAccessCountersAction(),
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.LoadTblFile,
					OnClick = async () => {
						string? filename = await FileDialogHelper.OpenFile(null, this, FileDialogHelper.TblExt);
						if(filename != null) {
							string[] tblData = File.ReadAllLines(filename);
							_model.TblConverter = TblLoader.Load(tblData);
							DebugWorkspaceManager.Workspace.TblMappings = tblData;
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.ResetTblMappings,
					OnClick = () => {
						_model.TblConverter = null;
						DebugWorkspaceManager.Workspace.TblMappings = Array.Empty<string>();
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => Close()
				}
			});

			_model.ViewMenuItems = _model.AddDisposables(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.ShowSettingsPanel,
					Shortcut =  () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ToggleSettingsPanel),
					IsSelected = () => _model.Config.ShowOptionPanel,
					OnClick = () => _model.Config.ShowOptionPanel = !_model.Config.ShowOptionPanel
				},
			});

			_model.SearchMenuItems = _model.AddDisposables(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.GoToAddress,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.GoToAddress),
					OnClick = async () => {
						MemoryType memType = _model.Config.MemoryType;
						int? address = await new GoToWindow(memType.ToCpuType(), memType, DebugApi.GetMemorySize(memType) - 1).ShowCenteredDialog<int?>(this);
						if(address != null) {
							_editor.SetCursorPosition(address.Value, scrollToTop: true);
							_editor.Focus();
						}
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.GoToAll,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.GoToAll),
					OnClick = async () => {
						GoToDestination? dest = await GoToAllWindow.Open(this, _model.Config.MemoryType.ToCpuType(), GoToAllOptions.ShowOutOfScope, DebugWorkspaceManager.SymbolProvider);
						if(dest?.RelativeAddress?.Type == _model.Config.MemoryType) {
							SetCursorPosition(dest.RelativeAddress.Value.Type, dest.RelativeAddress.Value.Address);
						} else if(dest?.AbsoluteAddress != null) {
							SetCursorPosition(dest.AbsoluteAddress.Value.Type, dest.AbsoluteAddress.Value.Address);
						}
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Find,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.Find),
					OnClick = () => OpenSearchWindow()
				},
				new ContextMenuAction() {
					ActionType = ActionType.FindPrev,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindPrev),
					OnClick = () => Find(SearchDirection.Backward)
				},
				new ContextMenuAction() {
					ActionType = ActionType.FindNext,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindNext),
					OnClick = () => Find(SearchDirection.Forward)
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.NavigateTo,
					SubActions = new List<object> {
						new ContextMenuAction() {
							ActionType = ActionType.GoToPrevRead,
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToPrevRead),
							OnClick = () => _model.NavigateTo(NavType.PrevRead)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToNextRead,
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToNextRead),
							OnClick = () => _model.NavigateTo(NavType.NextRead)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToPrevWrite,
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToPrevWrite),
							OnClick = () => _model.NavigateTo(NavType.PrevWrite)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToNextWrite,
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToNextWrite),
							OnClick = () => _model.NavigateTo(NavType.NextWrite)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToPrevExec,
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToPrevExec),
							OnClick = () => _model.NavigateTo(NavType.PrevExec)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToNextExec,
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToNextExec),
							OnClick = () => _model.NavigateTo(NavType.NextExec)
						},
						new ContextMenuSeparator(),
						new ContextMenuAction() {
							ActionType = ActionType.GoToPrevCode,
							IsEnabled = () => _model.Config.MemoryType.SupportsCdl(),
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToPrevCode),
							OnClick = () => _model.NavigateTo(NavType.PrevCode)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToNextCode,
							IsEnabled = () => _model.Config.MemoryType.SupportsCdl(),
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToNextCode),
							OnClick = () => _model.NavigateTo(NavType.NextCode)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToPrevData,
							IsEnabled = () => _model.Config.MemoryType.SupportsCdl(),
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToPrevData),
							OnClick = () => _model.NavigateTo(NavType.PrevData)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToNextData,
							IsEnabled = () => _model.Config.MemoryType.SupportsCdl(),
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToNextData),
							OnClick = () => _model.NavigateTo(NavType.NextData)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToPrevUnknown,
							IsEnabled = () => _model.Config.MemoryType.SupportsCdl(),
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToPrevUnknown),
							OnClick = () => _model.NavigateTo(NavType.PrevUnknown)
						},
						new ContextMenuAction() {
							ActionType = ActionType.GoToNextUnknown,
							IsEnabled = () => _model.Config.MemoryType.SupportsCdl(),
							Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_GoToNextUnknown),
							OnClick = () => _model.NavigateTo(NavType.NextUnknown)
						},
					}
				}
			});

			_model.ToolbarItems = _model.AddDisposables(new List<ContextMenuAction>() {
				GetImportAction(),
				GetExportAction(),
				new ContextMenuSeparator(),
				GetResetAccessCountersAction()
			});

			DebugShortcutManager.RegisterActions(this, _model.FileMenuItems);
			DebugShortcutManager.RegisterActions(this, _model.ViewMenuItems);
			DebugShortcutManager.RegisterActions(this, _model.SearchMenuItems);
		}

		private void OpenSearchWindow()
		{
			if(_searchWnd == null) {
				_searchWnd = new MemoryViewerFindWindow(_model.Search, _model);
				_searchWnd.Closed += (s, e) => _searchWnd = null;
				_searchWnd.ShowCenteredWithParent(this);
			} else {
				_searchWnd.BringToFront();
			}
		}

		private void Find(SearchDirection direction)
		{
			if(!_model.Search.IsValid) {
				OpenSearchWindow();
			} else {
				_model.Find(direction);
			}
		}

		private ContextMenuAction GetViewInMemoryAction()
		{
			AddressInfo? GetAddress()
			{
				MemoryType memType = _model.Config.MemoryType;
				if(_editor.SelectionLength <= 1) {
					if(memType.IsRelativeMemory()) {
						AddressInfo absAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = _model.SelectionStart, Type = memType });
						return absAddr.Address >= 0 ? absAddr : null;
					} else {
						AddressInfo relAddr = DebugApi.GetRelativeAddress(new AddressInfo() { Address = _model.SelectionStart, Type = memType }, memType.ToCpuType());
						return relAddr.Address >= 0 ? relAddr : null;
					}
				}
				return null;
			}

			return new ContextMenuAction() {
				ActionType = ActionType.ViewInMemoryViewer,
				DynamicText = () => ResourceHelper.GetMessage("ViewInAction", (GetAddress() is AddressInfo addr) ? ResourceHelper.GetEnumText(addr.Type) : "[n/a]"),
				IsVisible = () => GetAddress() != null,
				HintText = () => "$" + _editor.SelectionStart.ToString("X2"),
				OnClick = () => {
					if(GetAddress() is AddressInfo addr) {
						SetCursorPosition(addr.Type, addr.Address);
					}
				},
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_ViewInMemoryType)
			};
		}

		private ContextMenuAction GetViewInDebuggerAction()
		{
			AddressInfo? GetAddress()
			{
				MemoryType memType = _model.Config.MemoryType;
				if(_editor.SelectionLength <= 1 && !memType.IsPpuMemory()) {
					AddressInfo relAddr;
					if(!memType.IsRelativeMemory()) {
						relAddr = DebugApi.GetRelativeAddress(new AddressInfo() { Address = _model.SelectionStart, Type = memType }, memType.ToCpuType());
						return relAddr.Address >= 0 ? relAddr : null;
					} else {
						return new AddressInfo() { Address = _model.SelectionStart, Type = memType };
					}
				}
				return null;
			}

			return new ContextMenuAction() {
				ActionType = ActionType.ViewInDebugger,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_ViewInDebugger),
				IsEnabled = () => GetAddress() != null,
				HintText = () => "$" + _editor.SelectionStart.ToString("X2"),
				OnClick = () => {
					AddressInfo? relAddr = GetAddress();
					if(relAddr?.Address >= 0) {
						CpuType cpuType = relAddr.Value.Type.ToCpuType();
						DebuggerWindow.OpenWindowAtAddress(cpuType, relAddr.Value.Address);
					}
				}
			};
		}

		private ContextMenuAction GetResetAccessCountersAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.ResetAccessCounters,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_ResetAccessCounters),
				OnClick = () => {
					DebugApi.ResetMemoryAccessCounts();
					_editor.InvalidateVisual();
				}
			};
		}

		private ContextMenuAction GetImportAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.Import,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_Import),
				IsEnabled = () => !_model.Config.MemoryType.IsRelativeMemory(),
				OnClick = () => Import()
			};
		}

		private ContextMenuAction GetExportAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.Export,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_Export),
				OnClick = () => Export()
			};
		}

		public async void Import()
		{
			string? filename = await FileDialogHelper.OpenFile(ConfigManager.DebuggerFolder, this, FileDialogHelper.DmpExt);
			if(filename != null) {
				byte[]? dmpData = FileHelper.ReadAllBytes(filename);
				if(dmpData != null) {
					DebugApi.SetMemoryState(_model.Config.MemoryType, dmpData, dmpData.Length);
				}
			}
		}

		public async void Export()
		{
			string name = EmuApi.GetRomInfo().GetRomName() + " - " + _model.Config.MemoryType.ToString() + ".dmp";
			string? filename = await FileDialogHelper.SaveFile(ConfigManager.DebuggerFolder, name, this, FileDialogHelper.DmpExt);
			if(filename != null) {
				FileHelper.WriteAllBytes(filename, DebugApi.GetMemoryState(_model.Config.MemoryType));
			}
		}

		private ContextMenuAction GetEditLabelAction()
		{
			AddressInfo? GetAddress(MemoryType memType, int address)
			{
				if(memType.IsRelativeMemory()) {
					AddressInfo relAddress = new AddressInfo() {
						Address = address,
						Type = memType
					};

					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					return absAddress.Address >= 0 && absAddress.Type.SupportsLabels() ? absAddress : null;
				} else {
					return memType.SupportsLabels() ? new AddressInfo() { Address = address, Type = memType } : null;
				}
			}

			return new ContextMenuAction() {
				ActionType = ActionType.EditLabel,
				HintText = () => "$" + _editor.SelectionStart.ToString("X2"),
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_EditLabel),

				IsEnabled = () => GetAddress(_model.Config.MemoryType, _editor.SelectionStart) != null,

				OnClick = () => {
					AddressInfo? addr = GetAddress(_model.Config.MemoryType, _editor.SelectionStart);
					if(addr == null) {
						return;
					}

					CodeLabel? label = LabelManager.GetLabel((uint)addr.Value.Address, addr.Value.Type);
					if(label == null) {
						int length = 1;
						if(_editor.SelectionLength > 1) {
							int end = addr.Value.Address + _editor.SelectionLength;
							int memSize = DebugApi.GetMemorySize(addr.Value.Type);
							if(end >= memSize) {
								end = memSize;
							}
							length = end - addr.Value.Address;
						}

						label = new CodeLabel() {
							Address = (uint)addr.Value.Address,
							MemoryType = addr.Value.Type,
							Length = (uint)length
						};
					}

					LabelEditWindow.EditLabel(label.MemoryType.ToCpuType(), this, label);
				}
			};
		}

		private ContextMenuAction GetAddWatchAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.AddWatch,
				HintText = () => GetAddressRange(),
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_AddToWatch),

				IsEnabled = () => _model.Config.MemoryType.SupportsWatch(),

				OnClick = () => {
					string[] toAdd = Enumerable.Range(_editor.SelectionStart, Math.Max(1, _editor.SelectionLength)).Select((num) => $"[${num.ToString("X2")}]").ToArray();
					WatchManager.GetWatchManager(_model.Config.MemoryType.ToCpuType()).AddWatch(toAdd);
				}
			};
		}

		private ContextMenuAction GetEditBreakpointAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.EditBreakpoint,
				HintText = () => GetAddressRange(),
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MemoryViewer_EditBreakpoint),

				OnClick = async () => {
					uint startAddress = (uint)_editor.SelectionStart;
					uint endAddress = (uint)(_editor.SelectionStart + Math.Max(1, _editor.SelectionLength) - 1);

					MemoryType memType = _model.Config.MemoryType;
					Breakpoint? bp = BreakpointManager.GetMatchingBreakpoint(startAddress, endAddress, memType);
					CpuType cpuType = memType.ToCpuType();
					if(bp == null) {
						bp = new Breakpoint() { 
							MemoryType = memType, 
							CpuType = cpuType, 
							StartAddress = startAddress,
							EndAddress = endAddress,
							BreakOnWrite = true,
							BreakOnRead = true
						};
						if(bp.SupportsExec) {
							bp.BreakOnExec = true;
						}
					}

					bool result = await BreakpointEditWindow.EditBreakpointAsync(bp, this);
					if(result && DebugWindowManager.GetDebugWindow<DebuggerWindow>(x => x.CpuType == cpuType) == null) {
						DebuggerWindow.GetOrOpenWindow(cpuType);
					}
				}
			};
		}

		private ContextMenuAction GetFreezeAction(ActionType action, DebuggerShortcut shortcut)
		{
			DebugConfig cfg = ConfigManager.Config.Debug;
			return new ContextMenuAction() {
				ActionType = action,
				HintText = () => GetAddressRange(),
				IsVisible = () => _model.Config.MemoryType.SupportsFreezeAddress(),
				OnClick = () => {
					UInt32 start = (UInt32)_editor.SelectionStart;
					UInt32 end = (UInt32)(_editor.SelectionStart + Math.Max(1, _editor.SelectionLength) - 1);
					DebugApi.UpdateFrozenAddresses(_model.Config.MemoryType.ToCpuType(), start, end, action == ActionType.FreezeMemory);
				},
				Shortcut = () => cfg.Shortcuts.Get(shortcut)
			};
		}

		private ContextMenuAction GetMarkSelectionAction()
		{
			return MarkSelectionHelper.GetAction(
				() => _model.Config.MemoryType,
				() => _model.SelectionStart,
				() => _model.SelectionStart + Math.Max(0, _model.SelectionLength - 1),
				() => { }
			);
		}

		private string GetAddressRange()
		{
			string address = "$" + _editor.SelectionStart.ToString("X2");
			if(_editor.SelectionLength > 1) {
				address += "-$" + (_editor.SelectionStart + _editor.SelectionLength - 1).ToString("X2");
			}
			return address;
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
				case ConsoleNotificationType.StateLoaded:
					Dispatcher.UIThread.Post(() => {
						_editor.InvalidateVisual();
					});
					break;

				case ConsoleNotificationType.PpuFrameDone:
					if(!ToolRefreshHelper.LimitFps(this, 80)) {
						Dispatcher.UIThread.Post(() => {
							_editor.InvalidateVisual();
						});
					}
					break;

				case ConsoleNotificationType.GameLoaded:
					Dispatcher.UIThread.Post(() => {
						_model.UpdateAvailableMemoryTypes();
					});
					break;
			}
		}
	}
}
