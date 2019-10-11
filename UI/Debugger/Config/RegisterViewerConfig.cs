using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Mesen.GUI.Debugger;
using Mesen.GUI.Controls;
using Mesen.GUI.Utilities;

namespace Mesen.GUI.Config
{
	public class RegisterViewerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public bool AutoRefresh = true;
		public int RefreshScanline = 240;
		public int RefreshCycle = 0;

		public RegisterViewerConfig()
		{
		}
	}
}
