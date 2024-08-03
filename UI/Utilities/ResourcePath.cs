using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public struct ResourcePath : IEquatable<ResourcePath>
	{
		public string Path { get; set; }
		public string InnerFile { get; set; }
		public int InnerFileIndex { get; set; }

		public bool Exists { get { return File.Exists(Path); } }
		public bool Compressed { get { return !string.IsNullOrWhiteSpace(InnerFile); } }

		public string FileName { get { return Compressed ? InnerFile : System.IO.Path.GetFileName(Path); } }
		public string Folder { get { return System.IO.Path.GetDirectoryName(Path) ?? ""; } }
		
		public string ReadablePath
		{
			get
			{
				if(Compressed) {
					return $"{System.IO.Path.GetFileName(InnerFile)} ({System.IO.Path.GetFileName(Path)})";
				} else {
					return System.IO.Path.GetFileName(Path);
				}
			}
		}

		public override string ToString()
		{
			string resPath = Path;
			if(Compressed) {
				resPath += "\x1" + InnerFile;
				if(InnerFileIndex > 0) {
					resPath += "\x1" + (InnerFileIndex - 1).ToString();
				}
			}
			return resPath;
		}

		static public implicit operator ResourcePath(string path)
		{
			string[] tokens = path.Split('\x1');
			return new ResourcePath() {
				Path = tokens[0],
				InnerFile = tokens.Length > 1 ? tokens[1] : "",
				InnerFileIndex = tokens.Length > 2 ? (int.Parse(tokens[2]) + 1) : 0
			};
		}

		static public implicit operator string(ResourcePath resourcePath)
		{
			return resourcePath.ToString();
		}

		bool IEquatable<ResourcePath>.Equals(ResourcePath other)
		{
			return other.ToString() == this.ToString();
		}
	}
}
