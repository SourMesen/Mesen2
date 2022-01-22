using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
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
	public class MemoryToolsViewModel : DisposableViewModel
	{
		[Reactive] public HexEditorConfig Config { get; set; }
		[Reactive] public FontConfig FontConfig { get; set; }
		[Reactive] public int ScrollPosition { get; set; }
		[Reactive] public HexEditorDataProvider? DataProvider { get; set; }
		[Reactive] public TblByteCharConverter? TblConverter { get; set; }

		[Reactive] public Enum[] AvailableMemoryTypes { get; set; } = Array.Empty<Enum>();

		[Reactive] public int SelectionStart { get; set; }
		[Reactive] public int SelectionLength { get; set; }
		[Reactive] public string StatusBarText { get; private set; } = "";

		[ObservableAsProperty] public int MaxScrollValue { get; }

		public int[] AvailableWidths { get => new int[] { 4, 8, 16, 32, 48, 64, 80, 96, 112, 128 }; }

		public MemoryToolsViewModel()
		{
			Config = ConfigManager.Config.Debug.HexEditor;
			FontConfig = ConfigManager.Config.Debug.Font;
			ScrollPosition = 0;

			if(Design.IsDesignMode) {
				return;
			}

			UpdateAvailableMemoryTypes();

			AddDisposable(this.WhenAnyValue(x => x.SelectionStart, x => x.SelectionLength).Subscribe(((int start, int length) o) => {
				if(o.length <= 1) {
					StatusBarText = ResourceHelper.GetMessage("HexEditor_Location", o.start.ToString("X2"));
				} else {
					StatusBarText = ResourceHelper.GetMessage("HexEditor_Selection", o.start.ToString("X2"), (o.start + o.length - 1).ToString("X2"), o.length);
				}
			}));

			AddDisposable(this.WhenAnyValue(x => x.Config.MemoryType, x => x.TblConverter).Subscribe(((MemoryType memType, TblByteCharConverter? conv) o) => DataProvider = new HexEditorDataProvider(
				o.memType, Config, o.conv
			)));

			AddDisposable(this.WhenAnyValue(
				x => x.Config.MemoryType,
				x => x.Config.BytesPerRow
			).Select(((MemoryType memType, int bytesPerRow) o) => (DebugApi.GetMemorySize(o.memType) / o.bytesPerRow) - 1).ToPropertyEx(this, x => x.MaxScrollValue));
		}

		public void UpdateAvailableMemoryTypes()
		{
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.MemoryType)) {
				Config.MemoryType = (MemoryType)AvailableMemoryTypes.First();
			}
		}

		internal void SaveConfig()
		{
			ConfigManager.SaveConfig();
		}
	}
}
