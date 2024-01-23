using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Windows;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
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
				RomTestResult result = TestApi.RunRecordedTest(filename, false);

				string msg = "[Test] " + result.State.ToString();
				if(result.State != RomTestState.Passed) {
					msg += ": (" + result.ErrorCode.ToString() + ")";
				}

				await MessageBox.Show(null, msg, "", MessageBoxButtons.OK, result.State == RomTestState.Failed ? MessageBoxIcon.Error : MessageBoxIcon.Info);
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

		public static void RunGbMicroTests(bool all)
		{
			Task.Run(() => {
				ConcurrentDictionary<string, UInt32> results = new();

				List<string> testFiles = Directory.EnumerateFiles(@"C:\Code\gbmicrotest-main\bin" + (all ? "" : "\\pass"), "*.gb", SearchOption.AllDirectories).ToList();
				Parallel.ForEach(testFiles, new ParallelOptions() { MaxDegreeOfParallelism = Environment.ProcessorCount - 2 }, (string testFile) => {
					string entryName = Path.GetFileName(testFile);
					results[entryName] = TestApi.RunTest(testFile);
				});

				EmuApi.WriteLogEntry("==================");
				List<string> failedTests = new List<string>();
				List<string> entries = results.Keys.ToList();
				entries.Sort();

				foreach(var entry in entries) {
					UInt32 result = results[entry];
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
	}
}
