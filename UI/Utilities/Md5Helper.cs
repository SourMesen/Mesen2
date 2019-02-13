using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Utilities
{
	public class Md5Helper
	{
		public static string GetMD5Hash(string filename)
		{
			if(File.Exists(filename)) {
				var md5 = System.Security.Cryptography.MD5.Create();
				return BitConverter.ToString(md5.ComputeHash(File.ReadAllBytes(filename))).Replace("-", "");
			}
			return null;
		}
	}
}
