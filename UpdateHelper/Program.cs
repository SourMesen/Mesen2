using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Diagnostics;
using System.Windows.Forms;

namespace MesenUpdater
{
	class Program
	{
		static void Main(string[] args)
		{
			if(args.Length > 2) {
				string srcFile = args[0];
				string destFile = args[1];
				string backupDestFile = args[2];
				bool isAdmin = args.Length > 3 && args[3] == "admin";

				int retryCount = 0;
				while(retryCount < 10) {
					try {
						using(FileStream file = File.Open(destFile, FileMode.Open, FileAccess.ReadWrite, FileShare.Delete | FileShare.ReadWrite)) { }
						break;
					} catch {
						retryCount++;
						System.Threading.Thread.Sleep(1000);
					}
				}

				try {
					File.Copy(destFile, backupDestFile, true);
					File.Copy(srcFile, destFile, true);
				} catch {
					try {
						if(!isAdmin) {
							ProcessStartInfo proc = new ProcessStartInfo();
							proc.WindowStyle = ProcessWindowStyle.Normal;
							proc.FileName = System.Reflection.Assembly.GetEntryAssembly().Location;
							proc.Arguments = string.Format("\"{0}\" \"{1}\" \"{2}\" admin", srcFile, destFile, backupDestFile);
							proc.UseShellExecute = true;
							proc.Verb = "runas";
							Process.Start(proc);
							return;
						} else {
							MessageBox.Show("Update failed. Please try downloading and installing the new version manually.", "Mesen-S", MessageBoxButtons.OK, MessageBoxIcon.Error);
							return;
						}
					} catch {
						MessageBox.Show("Update failed. Please try downloading and installing the new version manually.", "Mesen-S", MessageBoxButtons.OK, MessageBoxIcon.Error);
						return;
					}
				}
				Process.Start(destFile);
			} else {
				MessageBox.Show("Please run Mesen-S directly to update.", "Mesen-S");
			}
		}
	}
}
