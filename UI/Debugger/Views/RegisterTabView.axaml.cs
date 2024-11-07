using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Windows;
using System;
using static Mesen.Debugger.ViewModels.LabelListViewModel;
using Avalonia.Input;
using DataBoxControl;
using Mesen.Debugger.Utilities;
using System.Collections.Generic;
using AvaloniaEdit.Editing;
using Mesen.Interop;
using Mesen.Config;
using Mesen.Utilities;

namespace Mesen.Debugger.Views
{
	public class RegisterTabView : MesenUserControl
	{
		public RegisterViewerTab Model => (RegisterViewerTab)DataContext!;

		public RegisterTabView()
		{
			InitializeComponent();

			DataBox dataBox = this.GetControl<DataBox>("lstRegisterTab");
			AddDisposables(DebugShortcutManager.CreateContextMenu(dataBox, new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.EditBreakpoint,
					IsEnabled = () => Model.CpuType.HasValue && Model.MemoryType.HasValue && Model.Selection.SelectedItem is RegEntry entry && entry.StartAddress >= 0,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.RegisterViewer_EditBreakpoint),
					HintText = () => {
						if(Model.CpuType.HasValue && Model.MemoryType.HasValue && Model.Selection.SelectedItem is RegEntry entry && entry.StartAddress >= 0) {
							string hint = $"${entry.StartAddress:X4}";
							if(entry.EndAddress > entry.StartAddress) {
								hint += $" - ${entry.EndAddress:X4}";
							}
							return hint;
						}
						return "";
					},
					OnClick = async () => {
						if(Model.CpuType.HasValue && Model.MemoryType.HasValue && Model.Selection.SelectedItem is RegEntry entry) {
							uint startAddress = (uint)entry.StartAddress;
							uint endAddress = (uint)entry.EndAddress;

							Breakpoint? bp = BreakpointManager.GetMatchingBreakpoint(startAddress, endAddress, Model.MemoryType.Value);
							if(bp == null) {
								bp = new Breakpoint() {
									BreakOnRead = true,
									BreakOnWrite = true,
									CpuType = Model.CpuType.Value,
									StartAddress = (uint)entry.StartAddress,
									EndAddress = (uint)entry.EndAddress,
									MemoryType = Model.MemoryType.Value
								};
							}

							bool result = await BreakpointEditWindow.EditBreakpointAsync(bp, this);
							if(result && DebugWindowManager.GetDebugWindow<DebuggerWindow>(x => x.CpuType == Model.CpuType) == null) {
								DebuggerWindow.GetOrOpenWindow(Model.CpuType.Value);
							}
						}
					}
				}
			}));
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
