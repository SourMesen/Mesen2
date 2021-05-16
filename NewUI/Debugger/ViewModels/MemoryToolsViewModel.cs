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
	public class MemoryToolsViewModel : ViewModelBase
	{
		[Reactive] public SnesMemoryType MemoryType { get; set; }
		[Reactive] public int BytesPerRow { get; set; }
		[Reactive] public int ScrollPosition { get; set; }
		[Reactive] public HexEditorDataProvider? DataProvider { get; set; }

		[ObservableAsProperty] public int MaxScrollValue { get; }

		public int[] AvailableWidths { get => new int[] { 4, 8, 16, 32, 48, 64, 80, 96, 112, 128 }; }

		public MemoryToolsViewModel()
		{
			this.MemoryType = SnesMemoryType.CpuMemory;
			this.BytesPerRow = 16;
			this.ScrollPosition = 0;

			if(Design.IsDesignMode) {
				return;
			}

			this.WhenAnyValue(x => x.MemoryType).Subscribe(x => DataProvider = new HexEditorDataProvider(
				x,
				true,
				true,
				true,
				60,
				false,
				false,
				false,
				false,
				true,
				true,
				true,
				true
			));

			this.WhenAnyValue(
				x => x.MemoryType,
				x => x.BytesPerRow
			).Select(((SnesMemoryType memType, int bytesPerRow) o) => (DebugApi.GetMemorySize(o.memType) / o.bytesPerRow) - 1).ToPropertyEx(this, x => x.MaxScrollValue);
		}
   }
}
