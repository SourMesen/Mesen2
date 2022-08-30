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

namespace Mesen.Debugger.Views
{
	public class RegisterTabView : UserControl
	{
		public RegisterViewerTab Model => (RegisterViewerTab)DataContext!;

		public RegisterTabView()
		{
			InitializeComponent();

			DataBox dataBox = this.GetControl<DataBox>("lstRegisterTab");
			DebugShortcutManager.CreateContextMenu(dataBox, new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.AddBreakpoint,
					IsEnabled = () => Model.CpuType.HasValue && Model.MemoryType.HasValue && Model.Selection.SelectedItem is RegEntry entry && entry.StartAddress >= 0,
					HintText = () => {
						if(Model.CpuType.HasValue && Model.MemoryType.HasValue && Model.Selection.SelectedItem is RegEntry entry) {
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
							Breakpoint bp = new Breakpoint() {
								BreakOnRead = true,
								BreakOnWrite = true,
								CpuType = Model.CpuType.Value,
								StartAddress = (uint)entry.StartAddress,
								EndAddress = (uint)entry.EndAddress,
								MemoryType = Model.MemoryType.Value
							};

							bool result = await BreakpointEditWindow.EditBreakpointAsync(bp, this);
							if(result && DebugWindowManager.GetDebugWindow<DebuggerWindow>(x => x.CpuType == Model.CpuType) == null) {
								DebuggerWindow.GetOrOpenWindow(Model.CpuType.Value);
							}
						}
					}
				}
			});
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
