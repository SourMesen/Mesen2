using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using Mesen.GUI.Interop;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Utilities
{
	public static class RomTestHelper
	{
		public static void RunTest()
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.SetFilter(ResourceHelper.GetMessage("FilterTest"));
				ofd.InitialDirectory = ConfigManager.TestFolder;
				if(ofd.ShowDialog(frmMain.Instance) == DialogResult.OK) {
					int result = TestApi.RunRecordedTest(ofd.FileName, false);

					if(result == 0) {
						MessageBox.Show("Test passed.", "", MessageBoxButtons.OK, MessageBoxIcon.Information);
					} else {
						MessageBox.Show("ERROR: Test failed! (" + result + ")", "", MessageBoxButtons.OK, MessageBoxIcon.Error);
					}
				}
			}
		}

		public static void RecordTest()
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter(ResourceHelper.GetMessage("FilterTest"));
				sfd.InitialDirectory = ConfigManager.TestFolder;
				sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".mtp";
				if(sfd.ShowDialog(frmMain.Instance) == DialogResult.OK) {
					TestApi.RomTestRecord(sfd.FileName, true);
				}
			}
		}

		public static void RunAllTests()
		{
			Task.Run(() => {
				ConcurrentDictionary<string, int> results = new ConcurrentDictionary<string, int>();

				List<string> testFiles = Directory.EnumerateFiles(ConfigManager.TestFolder, "*.mtp", SearchOption.AllDirectories).ToList();
				Parallel.ForEach(testFiles, new ParallelOptions() { MaxDegreeOfParallelism = 16 }, (string testFile) => {
					int result = TestApi.RunRecordedTest(testFile, true);
					results[Path.GetFileNameWithoutExtension(testFile)] = result;
				});

				frmMain.Instance.BeginInvoke((Action)(() => {
					EmuApi.WriteLogEntry("==================");
					List<string> failedTests = new List<string>();
					foreach(var kvp in results) {
						if(kvp.Value > 0) {
							EmuApi.WriteLogEntry("[Test] Failed: " + kvp.Key + " (" + kvp.Value.ToString() + ")");
							failedTests.Add(kvp.Key);
						} else {
							EmuApi.WriteLogEntry("[Test] Passed: " + kvp.Key);
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

					new frmLogWindow().Show();
				}));
			});
		}

		public static void StopRecording()
		{
			TestApi.RomTestStop();
		}
	}
}
