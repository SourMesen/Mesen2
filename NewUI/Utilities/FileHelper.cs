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
			if(File.Exists(filepath)) {
				int retry = 2;
				while(retry > 0) {
					try {
						return File.ReadAllText(filepath);
					} catch(Exception e) {
						if(retry == 0) {
							//TODO
							//MesenMsgBox.ShowException(e);
							return null;
						} else {
							System.Threading.Thread.Sleep(50);
							retry--;
						}
					}
				}
			}
			return null;
		}

		public static bool WriteAllText(string filepath, string content)
		{
			return WriteAllText(filepath, content, Encoding.UTF8);
		}

		public static bool WriteAllText(string filepath, string content, Encoding encoding)
		{
			int retry = 2;
			while(retry > 0) {
				try {
					File.WriteAllText(filepath, content, encoding);
					return true;
				} catch(Exception e) {
					if(retry == 0) {
						//TODO
						//MesenMsgBox.ShowException(e);
						return false;
					} else {
						System.Threading.Thread.Sleep(50);
						retry--;
					}
				}
			}
			return false;
		}
	}
}
