using Avalonia.Controls;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Text.RegularExpressions;
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

		[Reactive] public ObservableCollection<RecentItem> RecentItems { get; private set; }
		[Reactive] public bool HasRecentItems { get; private set; }
		public ReactiveCommand<RecentItem, Unit> OpenRecentCommand { get; }

		[Reactive] public List<object> ToolsMenuItems { get; set; } = new();
		[Reactive] public List<object> DebugMenuItems { get; set; } = new();
		[Reactive] public List<object> HelpMenuItems { get; set; } = new();

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

		public void Initialize(MainWindow wnd)
		{
			InitToolMenu(wnd);
			InitDebugMenu(wnd);
			InitHelpMenu(wnd);
		}

		private void InitToolMenu(MainWindow wnd)
		{
			ToolsMenuItems = new List<object>() {
				new MainMenuAction() {
					ActionType = ActionType.Cheats,
					IsEnabled = () => IsGameRunning,
					OnClick = () => { }
				},

				new MainMenuAction() {
					ActionType = ActionType.HistoryViewer,
					IsEnabled = () => IsGameRunning,
					OnClick = () => { }
				},

				new MainMenuAction() {
					ActionType = ActionType.Movies,
					SubActions = new List<object> {
						new MainMenuAction() {
							ActionType = ActionType.Play,
							IsEnabled = () => IsGameRunning && !RecordApi.MovieRecording() && !RecordApi.MoviePlaying(),
							OnClick = async () => {
								string? filename = await FileDialogHelper.OpenFile(ConfigManager.MovieFolder, wnd, FileDialogHelper.MesenMovieExt);
								if(filename != null) {
									RecordApi.MoviePlay(filename);
								}
							}
						},
						new MainMenuAction() {
							ActionType = ActionType.Record,
							IsEnabled = () => IsGameRunning && !RecordApi.MovieRecording() && !RecordApi.MoviePlaying(),
							OnClick = () => {
								new MovieRecordWindow() {
									DataContext = new MovieRecordConfigViewModel()
								}.ShowCenteredDialog((Control)wnd);
							}
						},
						new MainMenuAction() {
							ActionType = ActionType.Stop,
							IsEnabled = () => IsGameRunning && (RecordApi.MovieRecording() || RecordApi.MoviePlaying()),
							OnClick = async () => {
								string? filename = await FileDialogHelper.OpenFile(ConfigManager.MovieFolder, wnd, FileDialogHelper.MesenMovieExt);
								if(filename != null) {
									RecordApi.MovieStop();
								}
							}
						}
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.NetPlay,
					SubActions = new List<object> {
						new MainMenuAction() {
							ActionType = ActionType.Connect,
							IsEnabled = () => !NetplayApi.IsConnected() && !NetplayApi.IsServerRunning(),
							OnClick = () => {
								new NetplayConnectWindow() {
									DataContext = ConfigManager.Config.Netplay.Clone()
								}.ShowCenteredDialog((Control)wnd);
							}
						},

						new MainMenuAction() {
							ActionType = ActionType.Disconnect,
							IsEnabled = () => NetplayApi.IsConnected(),
							OnClick = () => {
								NetplayApi.Disconnect();
							}
						},

						new ContextMenuSeparator(),

						new MainMenuAction() {
							ActionType = ActionType.StartServer,
							IsEnabled = () => !NetplayApi.IsConnected() && !NetplayApi.IsServerRunning(),
							OnClick = () => {
								new NetplayStartServerWindow() {
									DataContext = ConfigManager.Config.Netplay.Clone()
								}.ShowCenteredDialog((Control)wnd);
							}
						},

						new MainMenuAction() {
							ActionType = ActionType.StopServer,
							IsEnabled = () => NetplayApi.IsServerRunning(),
							OnClick = () => {
								NetplayApi.StopServer();
							}
						},

						new ContextMenuSeparator(),

						new MainMenuAction() {
							ActionType = ActionType.SelectController,
							IsEnabled = () => NetplayApi.IsConnected() || NetplayApi.IsServerRunning(),
							SubActions = new List<object> {
								GetSelectControllerAction(0),
								GetSelectControllerAction(1),
								GetSelectControllerAction(2),
								GetSelectControllerAction(3),
								GetSelectControllerAction(4),
							}
						}
					}
				},
				
				new ContextMenuSeparator(),

				new MainMenuAction() {
					ActionType = ActionType.SoundRecorder,
					SubActions = new List<object> {
						new MainMenuAction() {
							ActionType = ActionType.Record,
							IsEnabled = () => IsGameRunning && !RecordApi.WaveIsRecording(),
							OnClick = async () => {
								string? filename = await FileDialogHelper.SaveFile(ConfigManager.WaveFolder, EmuApi.GetRomInfo().GetRomName() + ".wav", wnd, FileDialogHelper.WaveExt);
								if(filename != null) {
									RecordApi.WaveRecord(filename);
								}
							}
						},
						new MainMenuAction() {
							ActionType = ActionType.Stop,
							IsEnabled = () => IsGameRunning && RecordApi.WaveIsRecording(),
							OnClick = () => {
								RecordApi.WaveStop();
							}
						}
					}
				},

				new MainMenuAction() {
					ActionType = ActionType.VideoRecorder,
					SubActions = new List<object> {
						new MainMenuAction() {
							ActionType = ActionType.Record,
							IsEnabled = () => IsGameRunning && !RecordApi.AviIsRecording(),
							OnClick = () => {
								new VideoRecordWindow() {
									DataContext = new VideoRecordConfigViewModel()
								}.ShowCenteredDialog((Control)wnd);
							}
						},
						new MainMenuAction() {
							ActionType = ActionType.Stop,
							IsEnabled = () => IsGameRunning && RecordApi.AviIsRecording(),
							OnClick = () => {
								RecordApi.AviStop();
							}
						}
					}
				},

				new ContextMenuSeparator() {
					IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Nes
				},

				new MainMenuAction() {
					ActionType = ActionType.HdPacks,
					IsVisible = () => MainWindow.RomInfo.ConsoleType == ConsoleType.Nes,
					SubActions = new List<object> {
						new MainMenuAction() {
							ActionType = ActionType.InstallHdPack,
							OnClick = () => InstallHdPack(wnd)
						},
						new MainMenuAction() {
							ActionType = ActionType.HdPackBuilder,
							IsEnabled = () => false,
							OnClick = () => { }
						}
					}
				},

				new ContextMenuSeparator(),
				
				new MainMenuAction() {
					ActionType = ActionType.LogWindow,
					OnClick = () => {
						LogWindow? logWindow = ApplicationHelper.GetExistingWindow<LogWindow>();
						if(logWindow != null) {
							logWindow.Activate();
						} else {
							new LogWindow().ShowCentered((Control)wnd);
						}
					}
				},

				new MainMenuAction(EmulatorShortcut.TakeScreenshot) {
					ActionType = ActionType.TakeScreenshot,
				},
			};
		}

		private MainMenuAction GetSelectControllerAction(int i)
		{
			return new MainMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = ResourceHelper.GetMessage("Player") + " " + (i + 1) + " (" + ResourceHelper.GetEnumText(ConfigApi.GetControllerType(i)) + ")",
				IsSelected = () => i == NetplayApi.NetPlayGetControllerPort(),
				IsEnabled = () => (NetplayApi.NetPlayGetAvailableControllers() & (1 << i)) != 0,
				OnClick = () => NetplayApi.NetPlaySelectController(i)
			};
		}

		private void InitDebugMenu(Window wnd)
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
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.Debugger, wnd)
				}
			};

			DebugShortcutManager.RegisterActions(wnd, DebugMenuItems);
		}

		private void InitHelpMenu(Window wnd)
		{
			HelpMenuItems = new List<object>() {
				new MainMenuAction() {
					ActionType = ActionType.OnlineHelp,
					OnClick = () => ApplicationHelper.OpenBrowser("https://www.mesen.ca/docs/")
				},
				new MainMenuAction() {
					ActionType = ActionType.CheckForUpdates,
					OnClick = () => { }
				},
				new MainMenuAction() {
					ActionType = ActionType.ReportBug,
					OnClick = () => { }
				},
				new ContextMenuSeparator(),
				new MainMenuAction() {
					ActionType = ActionType.About,
					OnClick = () => {
						new AboutWindow().ShowCenteredDialog((Control)wnd);
					}
				},
			};
		}

		private async void InstallHdPack(Window wnd)
		{
			string? filename = await FileDialogHelper.OpenFile(null, wnd, FileDialogHelper.ZipExt);
			if(filename == null) {
				return;
			}

			try {
				using(FileStream stream = File.Open(filename, FileMode.Open)) {
					ZipArchive zip = new ZipArchive(stream);

					//Find the hires.txt file
					ZipArchiveEntry? hiresEntry = null;
					foreach(ZipArchiveEntry entry in zip.Entries) {
						if(entry.Name == "hires.txt") {
							hiresEntry = entry;
							break;
						}
					}

					if(hiresEntry == null) {
						await MesenMsgBox.Show(wnd, "InstallHdPackInvalidPack", MessageBoxButtons.OK, MessageBoxIcon.Error);
						return;
					}

					using Stream entryStream = hiresEntry.Open();
					using StreamReader reader = new StreamReader(entryStream);
					string hiresData = reader.ReadToEnd();
					RomInfo romInfo = EmuApi.GetRomInfo();

					//If there's a "supportedRom" tag, check if it matches the current ROM
					Regex supportedRomRegex = new Regex("<supportedRom>([^\\n]*)");
					Match match = supportedRomRegex.Match(hiresData);
					if(match.Success) {
						if(!match.Groups[1].Value.ToUpper().Contains(romInfo.Sha1.ToUpper())) {
							await MesenMsgBox.Show(wnd, "InstallHdPackWrongRom", MessageBoxButtons.OK, MessageBoxIcon.Error);
							return;
						}
					}

					//Extract HD pack
					try {
						string targetFolder = Path.Combine(ConfigManager.HdPackFolder, romInfo.GetRomName());
						if(Directory.Exists(targetFolder)) {
							//Warn if the folder already exists
							if(await MesenMsgBox.Show(wnd, "InstallHdPackConfirmOverwrite", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, targetFolder) != DialogResult.OK) {
								return;
							}
						} else {
							Directory.CreateDirectory(targetFolder);
						}

						string hiresFileFolder = hiresEntry.FullName.Substring(0, hiresEntry.FullName.Length - "hires.txt".Length);
						foreach(ZipArchiveEntry entry in zip.Entries) {
							//Extract only the files in the same subfolder as the hires.txt file (and only if they have a name & size > 0)
							if(!string.IsNullOrWhiteSpace(entry.Name) && entry.Length > 0 && entry.FullName.StartsWith(hiresFileFolder)) {
								entry.ExtractToFile(Path.Combine(targetFolder, entry.Name), true);
							}
						}
					} catch(Exception ex) {
						await MesenMsgBox.Show(wnd, "InstallHdPackError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
						return;
					}

					//Turn on HD Pack support automatically after installation succeeds
					if(!ConfigManager.Config.Nes.EnableHdPacks) {
						ConfigManager.Config.Nes.EnableHdPacks = true;
						ConfigManager.Config.Nes.ApplyConfig();
					}

					if(await MesenMsgBox.Show(wnd, "InstallHdPackConfirmReset", MessageBoxButtons.OKCancel, MessageBoxIcon.Question) == DialogResult.OK) {
						//Power cycle game if the user agrees
						EmuApi.PowerCycle();
					}
				}
			} catch {
				//Invalid file (file missing, not a zip file, etc.)
				await MesenMsgBox.Show(wnd, "InstallHdPackInvalidZipFile", MessageBoxButtons.OK, MessageBoxIcon.Error);
			}
		}
	}
}
