using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class ScriptWindowViewModel : ViewModelBase
	{
		public ScriptWindowConfig Config { get; } = ConfigManager.Config.Debug.ScriptWindow;
		public FontConfig Font { get; } = ConfigManager.Config.Debug.Font;

		[Reactive] public string Code { get; set; } = "";
		[Reactive] public string FilePath { get; set; } = "";
		[Reactive] public int ScriptId { get; set; } = -1;
		[Reactive] public string Log { get; set; } = "";

		private string _originalText = "";
		private ScriptWindow? _wnd = null;

		private ContextMenuAction _recentScriptsAction = new();

		[Reactive] public List<object> FileMenuActions { get; private set; } = new();
		[Reactive] public List<object> ScriptMenuActions { get; private set; } = new();
		[Reactive] public List<object> HelpMenuActions { get; private set; } = new();
		[Reactive] public List<object> ToolbarActions { get; private set; } = new();

		public ScriptWindowViewModel()
		{
		}

		public void InitActions(ScriptWindow wnd)
		{
			if(wnd == null) {
				throw new Exception("Invalid parent window");
			}
			
			_wnd = wnd;

			_recentScriptsAction = new ContextMenuAction() {
				ActionType = ActionType.RecentScripts,
				SubActions = new()
			};
			_recentScriptsAction.IsEnabled = () => _recentScriptsAction.SubActions.Count > 0;

			ScriptMenuActions = GetScriptMenuAction();
			ToolbarActions = GetScriptMenuAction();

			FileMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.NewScript,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_NewScript),
					OnClick = () => {
						new ScriptWindow(new ScriptWindowViewModel()).Show();
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.Open,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_OpenScript),
					OnClick = () => OpenScript()
				},
				new ContextMenuAction() {
					ActionType = ActionType.Save,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_SaveScript),
					OnClick = async () => await SaveScript()
				},
				new ContextMenuAction() {
					ActionType = ActionType.SaveAs,
					OnClick = async () => await SaveAs(Path.GetFileName(FilePath))
				},
				new Separator(),
				_recentScriptsAction,
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => _wnd?.Close()
				}
			};

			HelpMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.HelpApiReference,
					OnClick = () => Process.Start(new ProcessStartInfo() { FileName = "https://www.mesen.ca/snes/ApiReference.php", UseShellExecute = true })
				}
			};

			UpdateRecentScriptsMenu();
			DebugShortcutManager.RegisterActions(_wnd, ScriptMenuActions);
			DebugShortcutManager.RegisterActions(_wnd, FileMenuActions);
		}

		private List<object> GetScriptMenuAction()
		{
			 return new() {
				new ContextMenuAction() {
					ActionType = ActionType.RunScript,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_RunScript),
					IsEnabled = () => ScriptId < 0,
					OnClick = () => {
						ScriptId = DebugApi.LoadScript(ScriptName, Code, ScriptId);
					}
				},
				new ContextMenuAction() {
					ActionType = ActionType.StopScript,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_StopScript),
					IsEnabled = () => ScriptId >= 0,
					OnClick = () => {
						DebugApi.RemoveScript(ScriptId);
						ScriptId = -1;
					}
				},
				new Separator(),
				new ContextMenuAction() {
					ActionType = ActionType.Preferences,
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.ScriptWindow, _wnd)
				}
			};
		}

		private string ScriptName
		{
			get
			{
				if(FilePath != null) {
					return Path.GetFileName(FilePath);
				} else {
					return "unnamed.lua";
				}
			}
		}

		private string? InitialFolder
		{
			get
			{
				if(ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count > 0) {
					return Path.GetDirectoryName(ConfigManager.Config.Debug.ScriptWindow.RecentScripts[0]);
				}
				return null;
			}
		}

		private void UpdateRecentScriptsMenu()
		{
			_recentScriptsAction.SubActions = ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Select(x => new ContextMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = x,
				OnClick = () => LoadScript(x)
			}).ToList<object>();
		}

		private async void OpenScript()
		{
			if(!await SavePrompt()) {
				return;
			}

			string? filename = await FileDialogHelper.OpenFile(InitialFolder, _wnd, FileDialogHelper.LuaFileExt);
			if(filename != null) {
				LoadScript(filename);
			}
		}

		private void AddRecentScript(string filename)
		{
			ConfigManager.Config.Debug.ScriptWindow.AddRecentScript(filename);
			UpdateRecentScriptsMenu();
		}

		private void LoadScript(string filename)
		{
			if(File.Exists(filename)) {
				Code = File.ReadAllText(filename);
				FilePath = filename;
				_originalText = Code;
				AddRecentScript(filename);
			}
		}

		private async Task<bool> SavePrompt()
		{
			if(_originalText != Code) {
				DialogResult result = await MesenMsgBox.Show(_wnd, "You have unsaved changes for this script - would you like to save them?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);
				if(result == DialogResult.Yes) {
					return !(await SaveScript());
				} else if(result == DialogResult.Cancel) {
					return false;
				}
			}
			return true;
		}

		private async Task<bool> SaveScript()
		{
			if(!string.IsNullOrWhiteSpace(FilePath)) {
				File.WriteAllText(FilePath, Code, Encoding.UTF8);
				_originalText = Code;
				return true;
			} else {
				return await SaveAs("NewScript.lua");
			}
		}

		private async Task<bool> SaveAs(string newName)
		{
			string? filename = await FileDialogHelper.SaveFile(InitialFolder, newName, _wnd, FileDialogHelper.LuaFileExt);
			if(filename != null) {
				FilePath = filename;
				File.WriteAllText(FilePath, Code, Encoding.UTF8);
				AddRecentScript(filename);
				return true;
			} else {
				return false;
			}
		}
	}
}
