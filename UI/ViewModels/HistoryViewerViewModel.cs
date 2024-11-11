using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class HistoryViewerViewModel : DisposableViewModel
	{
		public HistoryViewerConfig Config { get; init; }

		[Reactive] public bool IsPaused { get; set; }

		[Reactive] public string TotalTimeText { get; set; } = "00:00";
		[Reactive] public string CurrentTimeText { get; set; } = "00:00";
		
		[Reactive] public uint MaxPosition { get; set; }
		[Reactive] public uint CurrentPosition { get; set; }
		
		[Reactive] public Size RendererSize { get; set; }

		[Reactive] public List<ContextMenuAction> FileMenuItems { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> OptionsMenuItems { get; private set; } = new();
		
		public SoftwareRendererViewModel SoftwareRenderer { get; } = new();
		[Reactive] public bool IsSoftwareRendererVisible { get; set; } = false;

		private bool _blockCoreUpdates = false;
		private uint[] _segments = Array.Empty<uint>();
		private double _fps = 60.0;

		public HistoryViewerViewModel()
		{
			Config = ConfigManager.Config.HistoryViewer;

			_blockCoreUpdates = true;
			AddDisposable(this.WhenAnyValue(x => x.CurrentPosition).Subscribe(x => {
				if(!_blockCoreUpdates) {
					HistoryApi.HistoryViewerSetPosition((uint)CurrentPosition);
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.Volume, x => x.IsPaused, x => x.RendererSize).Subscribe(x => {
				if(!_blockCoreUpdates) {
					SetCoreOptions();
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.SoftwareRenderer.FrameSurface).Subscribe(x => {
				IsSoftwareRendererVisible = SoftwareRenderer.FrameSurface != null;
			}));

			_blockCoreUpdates = false;
		}

		public void SetCoreOptions()
		{
			HistoryApi.HistoryViewerSetOptions(new HistoryViewerOptions() {
				IsPaused = IsPaused,
				Volume = (uint)Config.Volume,
				Width = (uint)RendererSize.Width ,
				Height = (uint)RendererSize.Height
			});
		}

		public void InitActions(HistoryViewerWindow wnd)
		{
			FileMenuItems = new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.ExportMovie,
					SubActions = InitExportItems(wnd)
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.CreateSaveState,
					OnClick = () => CreateSaveState(wnd)
				},
				new ContextMenuAction() {
					ActionType = ActionType.ResumeGameplay,
					OnClick = () => {
						HistoryApi.HistoryViewerResumeGameplay(CurrentPosition);
					}
				},
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd.Close()
				},
			};

			OptionsMenuItems = new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.VideoScale,
					SubActions = new List<object>() { 
						GetScaleMenuItem(wnd, 1),
						GetScaleMenuItem(wnd, 2),
						GetScaleMenuItem(wnd, 3),
						GetScaleMenuItem(wnd, 4),
						GetScaleMenuItem(wnd, 5),
						GetScaleMenuItem(wnd, 6),
						GetScaleMenuItem(wnd, 7),
						GetScaleMenuItem(wnd, 8),
						GetScaleMenuItem(wnd, 9),
						GetScaleMenuItem(wnd, 10)
					}
				}
			};

			DebugShortcutManager.RegisterActions(wnd, OptionsMenuItems);
		}

		private List<object> InitExportItems(HistoryViewerWindow wnd)
		{
			uint segmentStart = 0;
			int itemCount = 0;
			List<object> actions = new();
			for(int i = 0; i < _segments.Length; i++) {
				if((_segments[i] - segmentStart) / _fps > 2) {
					//Only list segments that are at least 2 seconds long
					UInt32 segStart = segmentStart;
					UInt32 segEnd = _segments[i];
					TimeSpan start = new TimeSpan(0, 0, (int)(segmentStart / _fps));
					TimeSpan end = new TimeSpan(0, 0, (int)(segEnd / _fps));

					string segmentName = ResourceHelper.GetMessage("MovieSegment", (itemCount + 1).ToString());
					string label = segmentName + ", " + start.ToString() + " - " + end.ToString();

					actions.Add(new ContextMenuAction() {
						ActionType = ActionType.Custom,
						CustomText = label,
						SubActions = new() {
							new ContextMenuAction() {
								ActionType = ActionType.Custom,
								CustomText = ResourceHelper.GetMessage("MovieExportEntireSegment"),
								OnClick = () => ExportMovie(wnd, segStart, segEnd)
							},
							new ContextMenuAction() {
								ActionType = ActionType.Custom,
								CustomText = ResourceHelper.GetMessage("MovieExportSpecificRange"),
								OnClick = async () => {
									HistoryViewerRangePickerWindow rangePicker = new(start, end);
									if(await rangePicker.ShowCenteredDialog<bool>(wnd)) {
										ExportMovie(wnd, (uint)(rangePicker.StartTime * _fps), (uint)(rangePicker.EndTime * _fps));
									}
								}
							}
						}
					});

					itemCount++;
				}

				//Next segment starts 60 "frames" after the end of the current one
				segmentStart = _segments[i] + 60;
			}

			return actions;
		}

		private async void ExportMovie(HistoryViewerWindow wnd, UInt32 segStart, UInt32 segEnd)
		{
			string initialFile = MainWindowViewModel.Instance.RomInfo.GetRomName();
			string? file = await FileDialogHelper.SaveFile(ConfigManager.MovieFolder, initialFile, wnd, FileDialogHelper.MesenMovieExt);
			if(file != null) {
				if(!HistoryApi.HistoryViewerSaveMovie(file, segStart, segEnd)) {
					await MesenMsgBox.Show(wnd, "MovieSaveError", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
		}

		private async void CreateSaveState(HistoryViewerWindow wnd)
		{
			uint position = CurrentPosition;
			string initialFile = MainWindowViewModel.Instance.RomInfo.GetRomName();
			string? file = await FileDialogHelper.SaveFile(ConfigManager.SaveStateFolder, initialFile, wnd, FileDialogHelper.MesenSaveStateExt);
			if(file != null) {
				if(!HistoryApi.HistoryViewerCreateSaveState(file, position)) {
					await MesenMsgBox.Show(wnd, "FileSaveError", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			}
		}

		private ContextMenuAction GetScaleMenuItem(HistoryViewerWindow wnd, int scale)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.Custom,
				CustomText = scale + "x",
				Shortcut = () => new DbgShortKeys(KeyModifiers.Alt, scale == 10 ? Key.D0 : Key.D0 + scale),
				OnClick = () => wnd.SetScale(scale),
				IsSelected = () => (int)((double)RendererSize.Height / EmuApi.GetBaseScreenSize().Height) == scale
			};
		}

		public void Update()
		{
			HistoryViewerState state = HistoryApi.HistoryViewerGetState();

			_blockCoreUpdates = true;

			_fps = state.Fps;

			IsPaused = state.IsPaused;
			Config.Volume = (int)state.Volume;
			CurrentPosition = state.Position;
			MaxPosition = state.Length;

			Array.Resize(ref state.Segments, (int)state.SegmentCount);
			_segments = state.Segments;

			TimeSpan currentPosition = new TimeSpan(0, 0, (int)(CurrentPosition / _fps));
			TimeSpan totalLength = new TimeSpan(0, 0, (int)(MaxPosition / _fps));
			CurrentTimeText = currentPosition.Minutes.ToString("00") + ":" + currentPosition.Seconds.ToString("00");
			TotalTimeText = totalLength.Minutes.ToString("00") + ":" + totalLength.Seconds.ToString("00");

			_blockCoreUpdates = false;
		}

		public void TogglePause()
		{
			if(IsPaused && CurrentPosition == MaxPosition) {
				CurrentPosition = 0;
			}
			IsPaused = !IsPaused;
		}
	}
}
