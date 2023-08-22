using Avalonia.Threading;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	internal class FileHelper
	{
		public static string? ReadAllText(string filepath)
		{
			return AttemptOperation(() => File.ReadAllText(filepath));
		}

		public static bool WriteAllText(string filepath, string content)
		{
			return WriteAllText(filepath, content, Encoding.UTF8);
		}

		public static bool WriteAllText(string filepath, string content, Encoding encoding)
		{
			return AttemptOperation(() => {
				File.WriteAllText(filepath, content, encoding);
				return true;
			});
		}

		public static byte[]? ReadAllBytes(string filepath)
		{
			return AttemptOperation(() => File.ReadAllBytes(filepath));
		}

		public static bool WriteAllBytes(string filepath, byte[] data)
		{
			return AttemptOperation(() => {
				File.WriteAllBytes(filepath, data);
				return true;
			});
		}

		public static FileStream? OpenRead(string filepath)
		{
			return AttemptOperation(() => {
				return File.Open(filepath, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
			});
		}

		private static T? AttemptOperation<T>(Func<T> action)
		{
			int retry = 3;
			while(retry > 0) {
				try {
					return action();
				} catch(Exception ex) {
					retry--;
					if(retry == 0) {
						if(Dispatcher.UIThread.CheckAccess()) {
							MesenMsgBox.ShowException(ex);
						} else {
							Dispatcher.UIThread.Post(() => {
								MesenMsgBox.ShowException(ex);
							});
						}
						return default;
					} else {
						System.Threading.Thread.Sleep(50);
					}
				}
			}
			return default;
		}
	}
}
