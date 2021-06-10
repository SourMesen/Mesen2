using Avalonia;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
		public class EventViewerConfig : BaseConfig<EventViewerConfig>
	{
		public Size WindowSize { get; set; } = new Size(0, 0);
		public Point WindowLocation { get; set; } = new Point(0, 0);

		public int ImageScale { get; set; } = 1;
		public bool RefreshOnBreakPause { get; set; } = true;

		public bool AutoRefresh { get; set; } = true;
		public RefreshSpeed AutoRefreshSpeed { get; set; } = RefreshSpeed.Normal;

		public SnesEventViewerConfig SnesConfig { get; set; } = new SnesEventViewerConfig();
		public NesEventViewerConfig NesConfig { get; set; } = new NesEventViewerConfig();
		public GbEventViewerConfig GbConfig { get; set; } = new GbEventViewerConfig();
	}
}
