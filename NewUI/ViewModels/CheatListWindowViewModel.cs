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
using Avalonia.Controls.Selection;
using DataBoxControl;
using System.ComponentModel;

namespace Mesen.ViewModels
{
	public class CheatListWindowViewModel : ViewModelBase
	{
		[Reactive] public AvaloniaList<CheatCode> Cheats { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> ToolbarActions { get; private set; } = new();
		[Reactive] public bool DisableAllCheats { get; set; } = false;
		
		[Reactive] public SelectionModel<CheatCode> Selection { get; set; } = new();
		[Reactive] public SortState SortState { get; set; } = new();

		private CheatCodes _cheatCodes = new();

		public CheatListWindowViewModel()
		{
			LoadCheats();
			DisableAllCheats = ConfigManager.Config.Cheats.DisableAllCheats;

			SortState.SetColumnSort("Description", ListSortDirection.Ascending, true);
			Sort();
		}

		public void Sort(object? param = null)
		{
			List<CheatCode> sortedCheats = new List<CheatCode>(Cheats);
			sortedCheats.Sort((a, b) => {
				int result = 0;

				foreach((string column, ListSortDirection order) in SortState.SortOrder) {
					void Compare(string name, Func<int> compare)
					{
						if(result == 0 && column == name) {
							result = compare() * (order == ListSortDirection.Ascending ? 1 : -1);
						}
					}

					Compare("Enabled", () => a.Enabled.CompareTo(b.Enabled));
					Compare("Description", () => a.Description.CompareTo(b.Description));
					Compare("Codes", () => a.Codes.CompareTo(b.Codes));

					if(result != 0) {
						return result;
					}
				}

				return result;
			});

			Cheats.Clear();
			Cheats.AddRange(sortedCheats);
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

		public void InitActions(Control parent)
		{
			ToolbarActions = GetActions(parent);
			DebugShortcutManager.CreateContextMenu(parent, GetActions(parent));
		}

		private List<ContextMenuAction> GetActions(Control parent)
		{
			return new List<ContextMenuAction> {
				new ContextMenuAction() {
					ActionType = ActionType.Add,
					AlwaysShowLabel = true,
					Shortcut = () => new DbgShortKeys(Key.Insert),
					OnClick = async () => {
						CheatCode newCheat = new CheatCode();
						if(await CheatEditWindow.EditCheat(newCheat, parent)) {
							Cheats.Add(newCheat);
							Sort();
							ApplyCheats();
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Edit,
					AlwaysShowLabel = true,
					IsEnabled = () => Selection.SelectedItems.Count == 1,
					OnClick = async () => {
						if(Selection.SelectedItem is CheatCode cheat) {
							await CheatEditWindow.EditCheat(cheat, parent);
							Sort();
							ApplyCheats();
						}
					}
				},

				new ContextMenuAction() {
					ActionType = ActionType.Delete,
					AlwaysShowLabel = true,
					Shortcut = () => new DbgShortKeys(Key.Delete),
					IsEnabled = () => Selection.SelectedItems.Count > 0,
					OnClick = () => {
						foreach(CheatCode cheat in Selection.SelectedItems.ToArray()) {
							Cheats.Remove(cheat);
						}
						ApplyCheats();
					}
				}
			};
		}
	}
}
