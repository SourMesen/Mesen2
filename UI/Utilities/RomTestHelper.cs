using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using Mesen.Windows;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class RomTestHelper
	{
		public static async void RunTest()
		{
			string? filename = await FileDialogHelper.OpenFile(ConfigManager.TestFolder, null, "mtp");
			if(filename != null) {
				MainWindowViewModel.Instance.RecentGames.Visible = false;

				_ = Task.Run(() => {
					RomTestResult result = TestApi.RunRecordedTest(filename, true);

					Dispatcher.UIThread.Post(async () => {
						string msg = "[Test] " + result.State.ToString();
						if(result.State != RomTestState.Passed) {
							msg += ": (" + result.ErrorCode.ToString() + ")";
						}

						await MessageBox.Show(null, msg, "", MessageBoxButtons.OK, result.State == RomTestState.Failed ? MessageBoxIcon.Error : MessageBoxIcon.Info);
						
						MainWindowViewModel.Instance.RecentGames.Visible = ConfigManager.Config.Preferences.GameSelectionScreenMode != GameSelectionMode.Disabled;
					});
				});
			}
		}

		public static void RecordTest()
		{
			string filename = Path.Join(ConfigManager.TestFolder, EmuApi.GetRomInfo().GetRomName() + ".mtp");
			TestApi.RomTestRecord(filename, true);
		}

		public static void RunAllTests()
		{
			Task.Run(() => {
				ConcurrentDictionary<string, RomTestResult> results = new();

				List<string> testFiles = Directory.EnumerateFiles(ConfigManager.TestFolder, "*.mtp", SearchOption.AllDirectories).ToList();
				Parallel.ForEach(testFiles, new ParallelOptions() { MaxDegreeOfParallelism = Environment.ProcessorCount - 2 }, (string testFile) => {
					string entryName = testFile.Substring(ConfigManager.TestFolder.Length);
					results[entryName] = TestApi.RunRecordedTest(testFile, true);
				});

				EmuApi.WriteLogEntry("==================");
				List<string> failedTests = new List<string>();
				List<string> entries = results.Keys.ToList();
				entries.Sort();

				foreach(var entry in entries) {
					RomTestResult result = results[entry];
					string msg = "[Test] " + result.State.ToString() + ": " + entry;
					if(result.State != RomTestState.Passed) {
						msg +=" (" + result.ErrorCode.ToString() + ")";
					}
					EmuApi.WriteLogEntry(msg);

					if(result.State == RomTestState.Failed) {
						failedTests.Add(entry);
					}
				}

				EmuApi.WriteLogEntry("==================");
				if(failedTests.Count > 0) {
					foreach(string failedTest in failedTests) {
						EmuApi.WriteLogEntry("  Failed: " + failedTest);
					}
					EmuApi.WriteLogEntry("==================");
					EmuApi.WriteLogEntry("Tests passed: " + (testFiles.Count - failedTests.Count));
					EmuApi.WriteLogEntry("Tests failed: " + failedTests.Count);
				} else {
					EmuApi.WriteLogEntry("All " + testFiles.Count + " tests passed!");
				}
				EmuApi.WriteLogEntry("==================");

				Dispatcher.UIThread.Post(() => {
					ApplicationHelper.GetOrCreateUniqueWindow<LogWindow>(null, () => new LogWindow());
				});
			});
		}

		public static void RunGbMicroTests()
		{
			Task.Run(() => {
				ConcurrentDictionary<string, UInt64> results = new();

				List<string> testFiles = Directory.EnumerateFiles(@"C:\Code\gbmicrotest-main\bin", "*.gb", SearchOption.AllDirectories).ToList();
				Parallel.ForEach(testFiles, new ParallelOptions() { MaxDegreeOfParallelism = Environment.ProcessorCount - 2 }, (string testFile) => {
					string entryName = Path.GetFileName(testFile);
					results[entryName] = TestApi.RunTest(testFile, 0x02, MemoryType.GbHighRam);
				});

				EmuApi.WriteLogEntry("==================");
				List<string> failedTests = new List<string>();
				List<string> entries = results.Keys.ToList();
				entries.Sort();

				foreach(var entry in entries) {
					UInt64 result = results[entry] & 0xFF;
					string msg = "[Test] ";
					switch(result) {
						case 1: msg += "Pass"; break;
						case 0xFF: msg += "FAIL"; break;
						default: msg += "UNKNOWN"; break;
					}
					msg += ": " + entry;

					EmuApi.WriteLogEntry(msg);

					if(result == 0xFF) {
						failedTests.Add(entry);
					}
				}

				EmuApi.WriteLogEntry("==================");
				if(failedTests.Count > 0) {
					foreach(string failedTest in failedTests) {
						EmuApi.WriteLogEntry("  Failed: " + failedTest);
					}
					EmuApi.WriteLogEntry("==================");
					EmuApi.WriteLogEntry("Tests passed: " + (testFiles.Count - failedTests.Count));
					EmuApi.WriteLogEntry("Tests failed: " + failedTests.Count);
				} else {
					EmuApi.WriteLogEntry("All " + testFiles.Count + " tests passed!");
				}
				EmuApi.WriteLogEntry("==================");

				Dispatcher.UIThread.Post(() => {
					ApplicationHelper.GetOrCreateUniqueWindow<LogWindow>(null, () => new LogWindow());
				});
			});
		}

		public static void RunGambatteTests()
		{
			Task.Run(() => {
				ConcurrentDictionary<string, UInt64> results = new();

				Regex regex = new Regex("dmg08_(cgb04c_){0,1}out([a-f0-9]+)[.]", RegexOptions.Compiled | RegexOptions.IgnoreCase);
				string folder = @"C:\Code\gambatte-tests\";
				List<string> testFiles = Directory.EnumerateFiles(folder, "*.gb*", SearchOption.AllDirectories).Where(x=>x.Contains("dmg08_") && regex.IsMatch(x)).ToList();
				Parallel.ForEach(testFiles, new ParallelOptions() { MaxDegreeOfParallelism = Environment.ProcessorCount - 2 }, (string testFile) => {
					string entryName = testFile.Substring(folder.Length);
					results[entryName] = TestApi.RunTest(testFile, 0x1800, MemoryType.GbVideoRam);
				});

				EmuApi.WriteLogEntry("==================");
				List<string> failedTests = new List<string>();
				List<string> entries = results.Keys.ToList();
				entries.Sort();

				foreach(var entry in entries) {
					int resultSize = regex.Match(entry).Groups[2].Value.Length;
					if(resultSize > 16) {
						continue;
					}
					UInt64 mask = UInt64.Parse(new string('F', resultSize), System.Globalization.NumberStyles.AllowHexSpecifier);

					UInt64 decodedResult = 0;
					for(int i = 0; i < resultSize; i++) {
						decodedResult |= (results[entry] & ((ulong)0xF << i * 8)) >> (i * 4);
					}

					UInt64 result = decodedResult & mask;
					string hexResult = string.Join(null, result.ToString("X" + resultSize).Reverse());

					bool passed = string.Compare(regex.Match(entry).Groups[2].Value, hexResult, true) == 0;
					if(!passed) {
						string msg = "[Test] FAIL [" + hexResult + "]: " + entry;
						EmuApi.WriteLogEntry(msg);
						failedTests.Add(entry);
					}
				}

				EmuApi.WriteLogEntry("==================");
				if(failedTests.Count > 0) {
					EmuApi.WriteLogEntry("==================");
					EmuApi.WriteLogEntry("Tests passed: " + (testFiles.Count - failedTests.Count));
					EmuApi.WriteLogEntry("Tests failed: " + failedTests.Count);
				} else {
					EmuApi.WriteLogEntry("All " + testFiles.Count + " tests passed!");
				}
				EmuApi.WriteLogEntry("==================");

				Dispatcher.UIThread.Post(() => {
					ApplicationHelper.GetOrCreateUniqueWindow<LogWindow>(null, () => new LogWindow());
				});
			});
		}
	}
}
