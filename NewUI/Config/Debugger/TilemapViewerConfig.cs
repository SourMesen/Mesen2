using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Avalonia;
using Mesen.GUI.Debugger;

namespace Mesen.GUI.Config
{
	public class TilemapViewerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public int ImageScale = 1;
		public bool ShowScrollOverlay = false;
		public bool ShowTileGrid = false;

		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Low;

		public bool AutoRefresh = true;
		public int RefreshScanline = 240;
		public int RefreshCycle = 0;

		public TilemapViewerConfig()
		{
		}
	}
}
