using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Debugger.Utilities;
using Mesen.Config;
using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Windows;
using Avalonia.Collections;
using Mesen.Interop;

namespace Mesen.ViewModels
{
	public class CheatListWindowViewModel : ViewModelBase
	{
		[Reactive] public AvaloniaList<CheatCode> Cheats { get; private set; } = new();
		[Reactive] public int SelectedIndex { get; set; } = -1;
		[Reactive] public List<ContextMenuAction> ToolbarActions { get; private set; } = new();
		[Reactive] public bool DisableAllCheats { get; set; } = false;

		private CheatCodes _cheatCodes = new();

		public CheatListWindowViewModel()
		{
			LoadCheats();
			DisableAllCheats = ConfigManager.Config.Cheats.DisableAllCheats;
		}

		public void LoadCheats()
		{
			_cheatCodes = CheatCodes.LoadCheatCodes();
			Cheats = new AvaloniaList<CheatCode>(_cheatCodes.Cheats);
		}

		public void SaveCheats()
		{
			ConfigManager.Config.Cheats.DisableAllCheats = DisableAllCheats;
			_cheatCodes.Cheats = new List<CheatCode>(Cheats);
			_cheatCodes.Save();
		}

		public void ApplyCheats()
		{
			if(DisableAllCheats) {
				EmuApi.ClearCheats();
			} else {
				CheatCodes.ApplyCheats(Cheats);
			}
		}

		public void InitActions(DataGrid grid)
		{
			ToolbarActions = GetActions(grid);
			DebugShortcutManager.CreateContextMenu(grid, GetActions(grid));
		}

		private List<ContextMenuAction> GetActions(DataGrid grid)
		{
			return new List<ContextMenuAction> {
				new ContextMenuAction() {
					ActionType = ActionType.Add,
					AlwaysShowLabel = true,
					Shortcut = () => new DbgShortKeys(Key.Insert),
					OnClick = async () => {
						CheatCode newCheat = new CheatCode();
						if(await CheatEditWindow.EditCheat(newCheat, grid)) {
							Cheats.Add(newCheat);
							ApplyCheats();
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					AlwaysShowLabel = true,
					IsEnabled = () => grid.SelectedItem is CheatCode,
					OnClick = async () => {
						if(grid.SelectedItem is CheatCode cheat) {
							await CheatEditWindow.EditCheat(cheat, grid);
							ApplyCheats();
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					AlwaysShowLabel = true,
					Shortcut = () => new DbgShortKeys(Key.Delete),
					IsEnabled = () => grid.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(CheatCode cheat in grid.SelectedItems.Cast<CheatCode>().ToList()) {
							Cheats.Remove(cheat);
						}
						ApplyCheats();
					}
				}
			};
		}
	}
}
