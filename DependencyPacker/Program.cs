using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.IO.Compression;

namespace DependencyPacker
{
	class Program
	{
		static void Main(string[] args)
		{
			if(File.Exists("Dependencies.zip")) {
				File.Delete("Dependencies.zip");
			}
			ZipFile.CreateFromDirectory("Dependencies", "Dependencies.zip");
		}
	}
}
