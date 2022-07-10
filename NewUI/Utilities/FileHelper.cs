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

		private static T? AttemptOperation<T>(Func<T> action)
		{
			int retry = 2;
			while(retry > 0) {
				try {
					return action();
				} catch(Exception e) {
					if(retry == 0) {
						//TODO
						//MesenMsgBox.ShowException(e);
						return default;
					} else {
						System.Threading.Thread.Sleep(50);
						retry--;
					}
				}
			}
			return default;
		}
	}
}
