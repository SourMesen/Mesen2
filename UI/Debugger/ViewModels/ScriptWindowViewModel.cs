using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Reactive.Linq;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Mesen.Localization;
using Avalonia.Threading;

namespace Mesen.Debugger.ViewModels
{
	public class ScriptWindowViewModel : ViewModelBase
	{
		public ScriptWindowConfig Config { get; } = ConfigManager.Config.Debug.ScriptWindow;

		[Reactive] public string Code { get; set; } = "";
		[Reactive] public string FilePath { get; set; } = "";
		[Reactive] public int ScriptId { get; set; } = -1;
		[Reactive] public string Log { get; set; } = "";
		[Reactive] public string ScriptName { get; set; } = "";

		[ObservableAsProperty] public string WindowTitle { get; } = "";

		private string _originalText = "";
		private ScriptWindow? _wnd = null;
		private FileSystemWatcher _fileWatcher = new();

		private ContextMenuAction _recentScriptsAction = new();

		[Reactive] public List<ContextMenuAction> FileMenuActions { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> ScriptMenuActions { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> HelpMenuActions { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> ToolbarActions { get; private set; } = new();

		public ScriptWindowViewModel(ScriptStartupBehavior? behavior = null)
		{
			this.WhenAnyValue(x => x.ScriptName).Select(x => {
				string wndTitle = ResourceHelper.GetViewLabel(nameof(ScriptWindow), "wndTitle");
				if(!string.IsNullOrWhiteSpace(x)) {
					return wndTitle + " - " + x;
				}
				return wndTitle;
			}).ToPropertyEx(this, x => x.WindowTitle);

			switch(behavior ?? Config.ScriptStartupBehavior) {
				case ScriptStartupBehavior.ShowBlankWindow: break;
				case ScriptStartupBehavior.ShowTutorial: LoadScriptFromResource("Mesen.Debugger.Utilities.LuaScripts.Example.lua"); break;
				case ScriptStartupBehavior.LoadLastScript:
					if(Config.RecentScripts.Count > 0) {
						LoadScript(Config.RecentScripts[0]);
					}
					break;
			}
		}

		public void InitActions(ScriptWindow wnd)
		{
			_wnd = wnd;

			ScriptMenuActions = GetScriptMenuActions();
			ToolbarActions = GetToolbarActions();

			FileMenuActions = GetSharedFileActions();
			FileMenuActions.AddRange(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.SaveAs,
					OnClick = async () => await SaveAs(Path.GetFileName(FilePath))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.BuiltInScripts,
					SubActions = GetBuiltInScriptActions()
				},
				new ContextMenuAction() {
					ActionType = ActionType.RecentScripts,
					IsEnabled = () => ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count > 0,
					SubActions = new() {
						GetRecentMenuItem(0),
						GetRecentMenuItem(1),
						GetRecentMenuItem(2),
						GetRecentMenuItem(3),
						GetRecentMenuItem(4),
						GetRecentMenuItem(5),
						GetRecentMenuItem(6),
						GetRecentMenuItem(7),
						GetRecentMenuItem(8),
						GetRecentMenuItem(9)
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => _wnd?.Close()
				}
			});

			HelpMenuActions = new() {
				new ContextMenuAction() {
					ActionType = ActionType.HelpApiReference,
					OnClick = () => {
						string tmpDoc = Path.Combine(ConfigManager.HomeFolder, "MesenLuaApiDoc.html");
						if(FileHelper.WriteAllText(tmpDoc, CodeCompletionHelper.GenerateHtmlDocumentation())) {
							ApplicationHelper.OpenBrowser(tmpDoc);
						}
					}
				}
			};

			DebugShortcutManager.RegisterActions(_wnd, ScriptMenuActions);
			DebugShortcutManager.RegisterActions(_wnd, FileMenuActions);
		}

		private MainMenuAction GetRecentMenuItem(int index)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				DynamicText = () => index < ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count ? ConfigManager.Config.Debug.ScriptWindow.RecentScripts[index] : "",
				IsVisible = () => index < ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count,
				OnClick = () => {
					if(index < ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count) {
						LoadScript(ConfigManager.Config.Debug.ScriptWindow.RecentScripts[index]);
					}
				}
			};
		}

		private List<ContextMenuAction> GetSharedFileActions()
		{
			return new() {
				new ContextMenuAction() {
					ActionType = ActionType.NewScript,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_NewScript),
					OnClick = () => {
						new ScriptWindow(new ScriptWindowViewModel(ScriptStartupBehavior.ShowBlankWindow)).Show();
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
				}
			};
		}

		private List<ContextMenuAction> GetToolbarActions()
		{
			List<ContextMenuAction> actions = GetSharedFileActions();
			actions.Add(new ContextMenuSeparator());
			actions.AddRange(GetScriptMenuActions());
			actions.Add(new ContextMenuSeparator());
			actions.Add(new ContextMenuAction() {
				ActionType = ActionType.BuiltInScripts,
				AlwaysShowLabel = true,
				SubActions = GetBuiltInScriptActions()
			});			
			return actions;
		}

		private List<object> GetBuiltInScriptActions()
		{
			List<object> actions = new();
			Assembly assembly = Assembly.GetExecutingAssembly();
			foreach(string name in assembly.GetManifestResourceNames()) {
				if(Path.GetExtension(name).ToLower() == ".lua") {
					string scriptName = name.Substring(name.LastIndexOf('.', name.Length - 5) + 1);

					actions.Add(new ContextMenuAction() {
						ActionType = ActionType.Custom,
						CustomText = scriptName,
						OnClick = () => LoadScriptFromResource(name)
					});
				}
			}
			actions.Sort((a, b) => ((ContextMenuAction)a).Name.CompareTo(((ContextMenuAction)b).Name));
			return actions;
		}

		private void LoadScriptFromResource(string resName)
		{
			Assembly assembly = Assembly.GetExecutingAssembly();
			string scriptName = resName.Substring(resName.LastIndexOf('.', resName.Length - 5) + 1);
			using Stream? stream = assembly.GetManifestResourceStream(resName);
			if(stream != null) {
				using StreamReader sr = new StreamReader(stream);
				LoadScriptFromString(sr.ReadToEnd());
				ScriptName = scriptName;
				FilePath = "";
			}
		}

		public async void RunScript()
		{
			if(Config.SaveScriptBeforeRun && !string.IsNullOrWhiteSpace(FilePath)) {
				await SaveScript();
			}

			string path = (Path.GetDirectoryName(FilePath) ?? Program.OriginalFolder) + Path.DirectorySeparatorChar;
			UpdateScriptId(DebugApi.LoadScript(ScriptName.Length == 0 ? "DefaultName" : ScriptName, path, Code, ScriptId));
		}

		private void UpdateScriptId(int scriptId)
		{
			if(Dispatcher.UIThread.CheckAccess()) {
				ScriptId = scriptId;
			} else {
				Dispatcher.UIThread.Post(() => ScriptId = scriptId);
			}
		}

		private List<ContextMenuAction> GetScriptMenuActions()
		{
			 return new() {
				new ContextMenuAction() {
					ActionType = ActionType.RunScript,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_RunScript),
					OnClick = RunScript
				},
				new ContextMenuAction() {
					ActionType = ActionType.StopScript,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.ScriptWindow_StopScript),
					IsEnabled = () => ScriptId >= 0,
					OnClick = StopScript
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugSettings,
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.ScriptWindow, _wnd)
				}
			};
		}

		public void StopScript()
		{
			DebugApi.RemoveScript(ScriptId);
			UpdateScriptId(-1);
		}

		public void RestartScript()
		{
			DebugApi.RemoveScript(ScriptId);
			string path = (Path.GetDirectoryName(FilePath) ?? Program.OriginalFolder) + Path.DirectorySeparatorChar;
			UpdateScriptId(DebugApi.LoadScript(ScriptName.Length == 0 ? "DefaultName" : ScriptName, path, Code, -1));
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

		private async void OpenScript()
		{
			if(!await SavePrompt()) {
				return;
			}

			string? filename = await FileDialogHelper.OpenFile(InitialFolder, _wnd, FileDialogHelper.LuaExt);
			if(filename != null) {
				LoadScript(filename);
			}
		}

		private void AddRecentScript(string filename)
		{
			ConfigManager.Config.Debug.ScriptWindow.AddRecentScript(filename);
		}

		private void LoadScriptFromString(string scriptContent)
		{
			Code = scriptContent;
			_originalText = Code;

			if(Config.AutoStartScriptOnLoad) {
				RunScript();
			}
		}

		public void LoadScript(string filename)
		{
			if(File.Exists(filename)) {
				string? code = FileHelper.ReadAllText(filename);
				if(code != null) {
					AddRecentScript(filename);
					SetFilePath(filename);
					LoadScriptFromString(code);
				}
			}
		}

		private void SetFilePath(string filename)
		{
			FilePath = filename;
			ScriptName = Path.GetFileName(filename);

			_fileWatcher.EnableRaisingEvents = false;

			_fileWatcher = new(Path.GetDirectoryName(FilePath) ?? "", Path.GetFileName(FilePath));
			_fileWatcher.Changed += (s, e) => {
				if(Config.AutoReloadScriptWhenFileChanges) {
					System.Threading.Thread.Sleep(100);
					Dispatcher.UIThread.Post(() => {
						LoadScript(FilePath);
					});
				}
			};
			_fileWatcher.EnableRaisingEvents = true;
		}

		public async Task<bool> SavePrompt()
		{
			if(_originalText != Code) {
				DialogResult result = await MesenMsgBox.Show(_wnd, "ScriptSaveConfirmation", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning);
				if(result == DialogResult.Yes) {
					return await SaveScript();
				} else if(result == DialogResult.Cancel) {
					return false;
				}
			}
			return true;
		}

		private async Task<bool> SaveScript()
		{
			if(!string.IsNullOrWhiteSpace(FilePath)) {
				if(_originalText != Code) {
					if(FileHelper.WriteAllText(FilePath, Code, Encoding.UTF8)) {
						_originalText = Code;
					} else {
						return false;
					}
				}
				return true;
			} else {
				return await SaveAs("NewScript.lua");
			}
		}

		private async Task<bool> SaveAs(string newName)
		{
			string? filename = await FileDialogHelper.SaveFile(InitialFolder, newName, _wnd, FileDialogHelper.LuaExt);
			if(filename != null) {
				if(FileHelper.WriteAllText(filename, Code, Encoding.UTF8)) {
					AddRecentScript(filename);
					SetFilePath(filename);
					return true;
				}
			}
			return false;
		}
	}
}
