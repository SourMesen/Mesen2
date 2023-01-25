using Avalonia.Threading;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reactive.Linq;
using System.Runtime.InteropServices;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class HdPackBuilderViewModel : DisposableViewModel
	{
		[Reactive] public string SaveFolder { get; set; }
		[Reactive] public bool IsRecording { get; set; }
		[Reactive] public bool IsBankSizeVisible { get; set; }
		[Reactive] public bool IsOpenFolderEnabled { get; set; }
		[Reactive] public HdPackBuilderConfig Config { get; set; }
		[Reactive] public FilterInfo? SelectedFilter { get; set; }
		[Reactive] public BankSizeInfo SelectedBankSize { get; set; }
		
		[Reactive] public FilterInfo[] Filters { get; private set; } = Array.Empty<FilterInfo>();

		public BankSizeInfo[] BankSizes { get; } = {
			new BankSizeInfo() { Name = "1 KB", BankSize = 0x400 },
			new BankSizeInfo() { Name = "2 KB", BankSize = 0x800 },
			new BankSizeInfo() { Name = "4 KB", BankSize = 0x1000 },
		};

		public HdPackBuilderViewModel()
		{
			Config = ConfigManager.Config.HdPackBuilder;
			SaveFolder = Path.Join(ConfigManager.HdPackFolder, EmuApi.GetRomInfo().GetRomName());

			UpdateFilterDropdown();

			SelectedFilter = Filters.Where(x => x.FilterType == Config.FilterType && x.Scale == Config.Scale).FirstOrDefault() ?? Filters[0];
			SelectedBankSize = BankSizes.Where(x => x.BankSize == Config.ChrRamBankSize).FirstOrDefault() ?? BankSizes[0];
			IsBankSizeVisible = EmuApi.GetGameMemorySize(MemoryType.NesChrRam) > 0;

			AddDisposable(this.WhenAnyValue(x => x.SelectedFilter).Subscribe(x => {
				if(x != null) {
					Config.FilterType = x.FilterType;
					Config.Scale = x.Scale;
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.SelectedBankSize).Subscribe(x => {
				Config.ChrRamBankSize = x.BankSize;
			}));

			AddDisposable(this.WhenAnyValue(x => x.SaveFolder).Subscribe(x => {
				IsOpenFolderEnabled = File.Exists(SaveFolder);
				UpdateFilterDropdown();
			}));
		}

		private void UpdateFilterDropdown()
		{
			string hdDefFile = Path.Combine(SaveFolder, "hires.txt");
			FilterInfo? selectedFilter = SelectedFilter;
			if(File.Exists(hdDefFile)) {
				string fileContent = File.ReadAllText(hdDefFile);
				Match match = Regex.Match(fileContent, "<scale>(\\d*)");
				if(match.Success) {
					int scale;
					if(Int32.TryParse(match.Groups[1].ToString(), out scale)) {
						Filters = _allFilters.Where(x => x.Scale == scale).ToArray();
						SelectedFilter = Filters.Contains(selectedFilter) ? selectedFilter : Filters[0];
						return;
					}
				}
			}

			Filters = _allFilters;
			SelectedFilter = selectedFilter;
		}

		public void StartRecording()
		{
			if(IsRecording) {
				return;
			}

			IsRecording = true;

			Task.Run(() => {
				HdPackBuilderOptions options = Config.ToInterop(SaveFolder);
				if(!IsBankSizeVisible) {
					options.ChrRamBankSize = 0x1000;
				}

				IntPtr optionsPtr = Marshal.AllocHGlobal(Marshal.SizeOf(options));
				try {
					Marshal.StructureToPtr(options, optionsPtr, false);
					EmuApi.ExecuteShortcut(new ExecuteShortcutParams() {
						Shortcut = EmulatorShortcut.StartRecordHdPack,
						ParamPtr = optionsPtr
					});
				} finally {
					Marshal.FreeHGlobal(optionsPtr);
				}
			});
		}

		public void StopRecording()
		{
			if(!IsRecording) {
				return;
			}

			IsRecording = false;

			Task.Run(() => {
				EmuApi.ExecuteShortcut(new ExecuteShortcutParams() { Shortcut = EmulatorShortcut.StopRecordHdPack });

				Dispatcher.UIThread.Post(() => {
					IsOpenFolderEnabled = true;
					UpdateFilterDropdown();
				});
			});
		}

		public void OpenFolder()
		{
			if(Directory.Exists(SaveFolder)) {
				System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo() {
					FileName = SaveFolder + Path.DirectorySeparatorChar,
					UseShellExecute = true,
					Verb = "open"
				});
			}
		}

		public class FilterInfo
		{
			public string Name { get; set; } = "";
			public ScaleFilterType FilterType { get; set; }
			public UInt32 Scale { get; set; }

			public override string ToString()
			{
				return Name;
			}
		}

		public class BankSizeInfo
		{
			public string Name { get; set; } = "";
			public UInt32 BankSize { get; set; }

			public override string ToString()
			{
				return Name;
			}
		}

		static private FilterInfo[] _allFilters = {
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.None) + " (1x)", FilterType = ScaleFilterType.Prescale, Scale = 1 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Prescale2x), FilterType = ScaleFilterType.Prescale, Scale = 2 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Prescale3x), FilterType = ScaleFilterType.Prescale, Scale = 3 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Prescale4x), FilterType = ScaleFilterType.Prescale, Scale = 4 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Prescale6x), FilterType = ScaleFilterType.Prescale, Scale = 6 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Prescale8x), FilterType = ScaleFilterType.Prescale, Scale = 8 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Prescale10x), FilterType = ScaleFilterType.Prescale, Scale = 10 },

			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.HQ2x), FilterType = ScaleFilterType.HQX, Scale = 2 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.HQ3x), FilterType = ScaleFilterType.HQX, Scale = 3 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.HQ4x), FilterType = ScaleFilterType.HQX, Scale = 4 },

			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Scale2x), FilterType = ScaleFilterType.Scale2x, Scale = 2 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Scale3x), FilterType = ScaleFilterType.Scale2x, Scale = 3 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Scale4x), FilterType = ScaleFilterType.Scale2x, Scale = 4 },

			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.Super2xSai), FilterType = ScaleFilterType.Super2xSai, Scale = 2 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.SuperEagle), FilterType = ScaleFilterType.SuperEagle, Scale = 2 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType._2xSai), FilterType = ScaleFilterType._2xSai, Scale = 2 },

			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.xBRZ2x), FilterType = ScaleFilterType.xBRZ, Scale = 2 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.xBRZ3x), FilterType = ScaleFilterType.xBRZ, Scale = 3 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.xBRZ4x), FilterType = ScaleFilterType.xBRZ, Scale = 4 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.xBRZ5x), FilterType = ScaleFilterType.xBRZ, Scale = 5 },
			new FilterInfo() { Name = ResourceHelper.GetEnumText(VideoFilterType.xBRZ6x), FilterType = ScaleFilterType.xBRZ, Scale = 6 },
		};
	}
}
