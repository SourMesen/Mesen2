using Avalonia.Controls;
using Mesen.GUI;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class TileViewerViewModel : ViewModelBase
	{
		[Reactive] public SnesMemoryType MemoryType { get; set; }
		[Reactive] public TileFormat TileFormat { get; set; }
		[Reactive] public TileLayout TileLayout { get; set; }
		[Reactive] public TileBackground TileBackground { get; set; }
		[Reactive] public bool ShowGrid { get; set; }

		public TileViewerViewModel()
		{
			this.MemoryType = SnesMemoryType.VideoRam;
			this.TileFormat = TileFormat.Bpp2;
			this.TileLayout = TileLayout.Normal;
			this.TileBackground = TileBackground.Default;
			this.ShowGrid = false;
		}
   }
}
