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

namespace Mesen.Debugger.Windows
{
	public class MemoryToolsWindow : Window
	{
		private NotificationListener _listener;
		private HexEditor _editor;
		private MemoryToolsViewModel _model;

		[Obsolete("For designer only")]
		public MemoryToolsWindow() : this(new MemoryToolsViewModel()) { }

		public MemoryToolsWindow(MemoryToolsViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = model;
			DataContext = model;
			_editor = this.FindControl<HexEditor>("Hex");
			_listener = new NotificationListener();

			if(Design.IsDesignMode) {
				return;
			}

			_model.Config.LoadWindowSettings(this);
			_editor.ByteUpdated += editor_ByteUpdated;
			_listener.OnNotification += listener_OnNotification;
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_listener?.Dispose();
			_model.Config.SaveWindowSettings(this);
			_model.SaveConfig();
			_model.Dispose();
			DataContext = null;
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			InitializeActions();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnSettingsClick(object sender, RoutedEventArgs e)
		{
			_model.Config.ShowOptionPanel = !_model.Config.ShowOptionPanel;
		}

		private async void OnLoadTblFileClick(object sender, RoutedEventArgs e)
		{
			string? filename = await FileDialogHelper.OpenFile(null, VisualRoot, FileDialogHelper.TblExt);
			if(filename != null) {
				_model.TblConverter = TblLoader.Load(File.ReadAllLines(filename));
			}
		}

		private void editor_ByteUpdated(object? sender, ByteUpdatedEventArgs e)
		{
			DebugApi.SetMemoryValue(_model.Config.MemoryType, (uint)e.ByteOffset, e.Value);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(this.DataContext is MemoryToolsViewModel model) {
				_model = model;
			}
		}

		private void InitializeActions()
		{
			DebugConfig cfg = ConfigManager.Config.Debug;

			DebugShortcutManager.CreateContextMenu(_editor, new object[] {
				GetMarkSelectionAction(),
				new Separator(),
				GetAddWatchAction(),
				GetEditBreakpointAction(),
				GetEditLabelAction(),
				new Separator(),
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
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.SelectAll,
					OnClick = () => _editor.SelectAll(),
					Shortcut = () => cfg.Shortcuts.Get(DebuggerShortcut.SelectAll)
				},
			});
		}

		private ContextMenuAction GetEditLabelAction()
		{
			AddressInfo? GetAddress(SnesMemoryType memType, int address)
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
						label = new CodeLabel() {
							Address = (uint)addr.Value.Address,
							MemoryType = addr.Value.Type
						};
					}

					LabelEditWindow.EditLabel(this, label);
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

				OnClick = () => {
					uint startAddress = (uint)_editor.SelectionStart;
					uint endAddress = (uint)(_editor.SelectionStart + Math.Max(1, _editor.SelectionLength) - 1);

					SnesMemoryType memType = _model.Config.MemoryType;
					Breakpoint? bp = BreakpointManager.GetMatchingBreakpoint(startAddress, endAddress, memType);
					if(bp == null) {
						bp = new Breakpoint() { 
							MemoryType = memType, 
							CpuType = memType.ToCpuType(), 
							StartAddress = startAddress,
							EndAddress = endAddress,
							BreakOnWrite = true,
							BreakOnRead = true
						};
						if(bp.IsCpuBreakpoint) {
							bp.BreakOnExec = true;
						}
					}

					BreakpointEditWindow.EditBreakpoint(bp, this);
				}
			};
		}

		private ContextMenuAction GetMarkSelectionAction()
		{
			return new ContextMenuAction() {
				ActionType = ActionType.MarkSelectionAs,
				HintText = () => GetAddressRange(),
				SubActions = new() {
					new ContextMenuAction() {
						ActionType = ActionType.MarkAsCode,
						IsEnabled = () => GetMarkStartEnd(out _, out _),
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MarkAsCode),
						OnClick = () => MarkSelectionAs(CdlFlags.Code)
					},
					new ContextMenuAction() {
						ActionType = ActionType.MarkAsData,
						IsEnabled = () => GetMarkStartEnd(out _, out _),
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MarkAsData),
						OnClick = () => MarkSelectionAs(CdlFlags.Data)
					},
					new ContextMenuAction() {
						ActionType = ActionType.MarkAsUnidentified,
						IsEnabled = () => GetMarkStartEnd(out _, out _),
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MarkAsUnidentified),
						OnClick = () => MarkSelectionAs(CdlFlags.None)
					}
				}
			};
		}

		private bool GetMarkStartEnd(out int start, out int end)
		{
			SnesMemoryType memType = _model.Config.MemoryType;
			start = _editor.SelectionStart;
			end = start + Math.Max(1, _editor.SelectionLength) - 1;

			if(memType.IsRelativeMemory()) {
				AddressInfo startAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = start, Type = memType });
				AddressInfo endAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = end, Type = memType });
				if(startAddr.Type == endAddr.Type && startAddr.Type.SupportsCdl()) {
					start = startAddr.Address;
					end = endAddr.Address;
				} else {
					return false;
				}
			} else if(!memType.SupportsCdl()) {
				return false;
			}

			return start >= 0 && end >= 0 && start <= end;
		}

		private void MarkSelectionAs(CdlFlags type)
		{
			if(GetMarkStartEnd(out int start, out int end)) {
				DebugApi.MarkBytesAs(_model.Config.MemoryType.ToCpuType(), (UInt32)start, (UInt32)end, type);
			}
		}

		private string GetAddressRange()
		{
			string address = "$" + _editor.SelectionStart.ToString("X2");
			if(_editor.SelectionLength > 1) {
				address += "-$" + (_editor.SelectionStart + _editor.SelectionLength - 1).ToString("X2");
			}
			return address;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.PpuFrameDone:
				case ConsoleNotificationType.CodeBreak:
					Dispatcher.UIThread.Post(() => {
						_editor.InvalidateVisual();
					});
					break;

				case ConsoleNotificationType.GameLoaded:
					_model.UpdateAvailableMemoryTypes();
					break;
			}
		}
	}
}
