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
				Parallel.ForEach(testFiles, new ParallelOptions() { MaxDegreeOfParallelism = 6 }, (string testFile) => {
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
					EmuApi.WriteLogEntry("Tests passed: " + (testFiles.Count - failedTests.Count));
					EmuApi.WriteLogEntry("Tests failed: " + failedTests.Count);
					foreach(string failedTest in failedTests) {
						EmuApi.WriteLogEntry("  Failed: " + failedTest);
					}
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
