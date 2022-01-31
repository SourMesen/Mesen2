using Avalonia.Controls;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class MainMenuViewModel : ViewModelBase
	{
		public MainWindowViewModel MainWindow { get; set; }

		[Reactive] public bool IsGameRunning { get; private set; }
		[Reactive] public bool IsFdsGame { get; private set; }
		[Reactive] public bool IsVsSystemGame { get; private set; }
		[Reactive] public bool IsVsDualSystemGame { get; private set; }
		[Reactive] public bool IsNesGame { get; private set; }
		[Reactive] public bool IsGbGame { get; private set; }

		[Reactive] public bool IsMovieActive { get; private set; }
		[Reactive] public bool IsVideoRecording { get; private set; }
		[Reactive] public bool IsSoundRecording { get; private set; }

		[Reactive] public bool IsNetPlayActive { get; private set; }
		[Reactive] public bool IsNetPlayClient { get; private set; }
		[Reactive] public bool IsNetPlayServer { get; private set; }

		[Reactive] public ObservableCollection<RecentItem> RecentItems { get; private set; }
		[Reactive] public bool HasRecentItems { get; private set; }
		public ReactiveCommand<RecentItem, Unit> OpenRecentCommand { get; }

		[Reactive] public List<object> DebugMenuItems { get; set; } = new();

		[Obsolete("For designer only")]
		public MainMenuViewModel() : this(new MainWindowViewModel()) { }

		public MainMenuViewModel(MainWindowViewModel windowModel)
		{
			MainWindow = windowModel;

			OpenRecentCommand = ReactiveCommand.Create<RecentItem>(OpenRecent);

			RecentItems = ConfigManager.Config.RecentFiles.Items;
			this.WhenAnyValue(x => x.RecentItems.Count).Subscribe(count => {
				HasRecentItems = count > 0;
			});

			this.WhenAnyValue(x => x.MainWindow.RomInfo).Subscribe(x => {
				IsGameRunning = x.Format != RomFormat.Unknown;

				IsFdsGame = x.Format == RomFormat.Fds;
				IsVsSystemGame = x.Format == RomFormat.VsSystem || x.Format == RomFormat.VsDualSystem;
				IsVsDualSystemGame = x.Format == RomFormat.VsDualSystem;
				IsNesGame = x.ConsoleType == ConsoleType.Nes;
				IsGbGame = x.ConsoleType == ConsoleType.GameboyColor || x.ConsoleType == ConsoleType.Gameboy;
			});
		}

		private void OpenRecent(RecentItem recent)
		{
			//Avalonia bug - Run in another thread to allow menu to close properly
			//See: https://github.com/AvaloniaUI/Avalonia/issues/5376
			Task.Run(() => {
				LoadRomHelper.LoadRom(recent.RomFile, recent.PatchFile);
			});
		}

		internal void UpdateToolSubMenus()
		{
			IsVideoRecording = RecordApi.AviIsRecording();
			IsSoundRecording = RecordApi.WaveIsRecording();
			IsMovieActive = RecordApi.MovieRecording() || RecordApi.MoviePlaying();
			
			IsNetPlayClient = NetplayApi.IsConnected();
			IsNetPlayServer = NetplayApi.IsServerRunning();
			IsNetPlayActive = IsNetPlayClient || IsNetPlayServer;
		}

		public void Initialize()
		{
			InitDebugMenu();
		}

		private void InitDebugMenu()
		{
			DebugMenuItems = new List<object>() {
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugger),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(null))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSpcDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSpcDebugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Spc),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(CpuType.Spc))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenCx4Debugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenCx4Debugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Cx4),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(CpuType.Cx4))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenNecDspDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenNecDspDebugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.NecDsp),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(CpuType.NecDsp))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenGsuDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenGsuDebugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Gsu),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(CpuType.Gsu))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSa1Debugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSa1Debugger),
					IsVisible = () => MainWindow.RomInfo.CpuTypes.Contains(CpuType.Sa1),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(CpuType.Sa1))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenGameboyDebugger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenGameboyDebugger),
					IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Snes && MainWindow.RomInfo.CpuTypes.Contains(CpuType.Gameboy),
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new DebuggerWindow(CpuType.Gameboy))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenEventViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenEventViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new EventViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenMemoryTools,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenMemoryTools),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new MemoryToolsWindow(new MemoryToolsViewModel()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenRegisterViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenRegisterViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new RegisterViewerWindow(new RegisterViewerWindowViewModel()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenTraceLogger,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenTraceLogger),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TraceLoggerWindow(new TraceLoggerViewModel()))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenTilemapViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenTilemapViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TilemapViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenTileViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenTileViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new TileViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenSpriteViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenSpriteViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new SpriteViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenPaletteViewer,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenPaletteViewer),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new PaletteViewerWindow(MainWindow.RomInfo.ConsoleType.GetMainCpuType()))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenAssembler,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenAssembler),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new AssemblerWindow(new AssemblerWindowViewModel(MainWindow.RomInfo.ConsoleType.GetMainCpuType())))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenProfiler,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenProfiler),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new ProfilerWindow(new ProfilerWindowViewModel()))
				},
				new ContextMenuAction() {
					ActionType = ActionType.OpenScriptWindow,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenScriptWindow),
					IsEnabled = () => IsGameRunning,
					OnClick = () => DebugWindowManager.OpenDebugWindow(() => new ScriptWindow(new ScriptWindowViewModel()))
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugSettings,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugSettings),
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.Debugger, ApplicationHelper.GetMainWindow())
				}
			};

			DebugShortcutManager.RegisterActions(ApplicationHelper.GetMainWindow()!, DebugMenuItems);
		}
	}
}
