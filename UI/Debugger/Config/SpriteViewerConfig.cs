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
	public class SpriteViewerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public int ImageScale = 2;
		public int SplitterDistance = 514;
		public bool AutoRefresh = true;
		public bool HideOffscreenSprites = false;
		public int RefreshScanline = 240;
		public int RefreshCycle = 0;

		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Low;

		public SpriteViewerConfig()
		{
		}
	}
}
