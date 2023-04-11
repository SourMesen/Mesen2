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
using Mesen.Utilities;
using Avalonia.VisualTree;
using Mesen.Debugger;

namespace Mesen.ViewModels
{
	public class CheatListWindowViewModel : ViewModelBase
	{
		[Reactive] public MesenList<CheatCode> Cheats { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> ToolbarActions { get; private set; } = new();
		[Reactive] public bool DisableAllCheats { get; set; } = false;

		[Reactive] public SelectionModel<CheatCode> Selection { get; set; } = new();
		[Reactive] public SortState SortState { get; set; } = new();

		public CheatWindowConfig Config { get; }

		private CheatCodes _cheatCodes = new();

		public CheatListWindowViewModel()
		{
			Config = ConfigManager.Config.Cheats;
			Selection.SingleSelect = false;

			LoadCheats();
			DisableAllCheats = Config.DisableAllCheats;

			SortState.SetColumnSort("Description", ListSortDirection.Ascending, true);
			Sort();
		}

		private Dictionary<string, Func<CheatCode, CheatCode, int>> _comparers = new() {
			{ "Enabled", (a, b) => a.Enabled.CompareTo(b.Enabled) },
			{ "Description", (a, b) => string.Compare(a.Description, b.Description, StringComparison.OrdinalIgnoreCase) },
			{ "Codes", (a, b) => string.Compare(a.Codes, b.Codes, StringComparison.OrdinalIgnoreCase) }
		};

		public void Sort(object? param = null)
		{
			List<int> selectedIndexes = Selection.SelectedIndexes.ToList();
			List<CheatCode> sortedCheats = new List<CheatCode>(Cheats);
			SortHelper.SortList(sortedCheats, SortState.SortOrder, _comparers, "Codes");
			Cheats.Replace(sortedCheats);
			if(selectedIndexes.Count >= 0) {
				foreach(int index in selectedIndexes) {
					if(index < Cheats.Count) {
						Selection.Select(index);
					} else {
						Selection.SelectedIndex = Cheats.Count - 1;
					}
				}
			}
		}

		public void LoadCheats()
		{
			_cheatCodes = CheatCodes.LoadCheatCodes();
			Cheats = new MesenList<CheatCode>(_cheatCodes.Cheats);
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
			List<ContextMenuAction> toolbarActions = GetActions(parent);
			toolbarActions.Add(new ContextMenuSeparator());
			toolbarActions.Add(new ContextMenuAction() {
				ActionType = ActionType.CheatDatabase,
				AlwaysShowLabel = true,
				IsEnabled = () => MainWindowViewModel.Instance.RomInfo.ConsoleType switch {
					ConsoleType.Nes => true,
					ConsoleType.Snes => true,
					_ => false
				},
				OnClick = async () => {
					ConsoleType consoleType = MainWindowViewModel.Instance.RomInfo.ConsoleType;
					CheatDbGameEntry? dbEntry = await CheatDatabaseWindow.Show(consoleType, parent);
					if(dbEntry != null && consoleType == MainWindowViewModel.Instance.RomInfo.ConsoleType) {
						List<CheatCode> newCheats = new();
						foreach(CheatDbCheatEntry cheatEntry in dbEntry.Cheats) {
							CheatCode newCheat = new CheatCode();
							newCheat.Description = cheatEntry.Desc;
							newCheat.Enabled = false;
							newCheat.Type = GetCheatType(consoleType, cheatEntry.Code);
							newCheat.Codes = string.Join(Environment.NewLine, cheatEntry.Code.Split(";", StringSplitOptions.RemoveEmptyEntries));
							newCheats.Add(newCheat);
						}
						Cheats.AddRange(newCheats);
						Sort();
						ApplyCheats();
					}
				}
			});
			ToolbarActions = toolbarActions;

			DebugShortcutManager.CreateContextMenu(parent, GetActions(parent));
		}

		private CheatType GetCheatType(ConsoleType consoleType, string code)
		{
			switch(consoleType) {
				case ConsoleType.Snes:
					return code.Contains("-") ? CheatType.SnesGameGenie : CheatType.SnesProActionReplay;

				case ConsoleType.Nes:
					return code.Contains(":") ? CheatType.NesCustom : CheatType.NesGameGenie;

				default:
					throw new Exception("Unsupported cheat type");
			}
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
						foreach(CheatCode cheat in Selection.SelectedItems.Cast<CheatCode>().ToArray()) {
							Cheats.Remove(cheat);
						}
						ApplyCheats();
					}
				}
			};
		}
	}
}
