using Mesen.Config;
using Mesen.Interop;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class UpdateHelper
	{
		public static bool LaunchUpdate(string srcFile)
		{
			string destFile = Program.ExePath;
			Version installedVersion = EmuApi.GetMesenVersion();
			string backupFilePath = Path.Combine(ConfigManager.BackupFolder, "Mesen." + installedVersion.ToString(3) + ".exe");

			string exeName = Path.GetFileName(Program.ExePath);
			string updateHelper = Path.Combine(ConfigManager.BackupFolder, exeName);

			try {
				//Create a copy of the current .exe to use as an updater
				File.Copy(Program.ExePath, updateHelper, true);

				FileInfo fileInfo = new FileInfo(srcFile);
				if(fileInfo.Length > 0) {
					Process.Start(updateHelper, string.Format("--update \"{0}\" \"{1}\" \"{2}\"", srcFile, destFile, backupFilePath));
					return true;
				} else {
					//Download failed, mismatching hashes
					MesenMsgBox.Show(null, "UpdateDownloadFailed", MessageBoxButtons.OK, MessageBoxIcon.Error);
				}
			} catch(Exception ex) {
				MesenMsgBox.ShowException(ex);
			}
			return false;
		}

		public static void AttemptUpdate(string srcFile, string destFile, string backupFile, bool isAdmin)
		{
			//Wait a bit for the application to shut down before trying to kill it
			System.Threading.Thread.Sleep(1000);
			try {
				Process currentProcess = Process.GetCurrentProcess();
				foreach(Process process in Process.GetProcesses()) {
					if(currentProcess != process) {
						try {
							if(process.MainModule?.FileName == destFile) {
								process.Kill();
							}
						} catch { }
					}
				}
			} catch { }

			int retryCount = 0;
			while(retryCount < 10) {
				try {
					using(FileStream file = File.Open(destFile, FileMode.Open, FileAccess.ReadWrite, FileShare.Delete | FileShare.ReadWrite)) { }
					break;
				} catch {
					retryCount++;
					System.Threading.Thread.Sleep(200);
				}
			}

			try {
				//Backup current version 
				File.Copy(destFile, backupFile, true);
				
				//Update with downloaded version
				File.Copy(srcFile, destFile, true);

				//Start new version
				Process.Start(destFile);
			} catch {
				try {
					//Something failed, try again with admin rights (if we aren't already running as an admin)
					if(!isAdmin) {
						ProcessStartInfo proc = new ProcessStartInfo();
						proc.WindowStyle = ProcessWindowStyle.Normal;
						proc.FileName = Program.ExePath;
						proc.Arguments = string.Format("--update \"{0}\" \"{1}\" \"{2}\" admin", srcFile, destFile, backupFile);
						proc.UseShellExecute = true;
						proc.Verb = "runas";
						Process.Start(proc);
					}
				} catch {
				}
			}
		}
	}
}
