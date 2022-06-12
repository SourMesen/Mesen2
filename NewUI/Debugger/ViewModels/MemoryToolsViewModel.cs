using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
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

		[Reactive] public List<ContextMenuAction> FileMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> SearchMenuItems { get; set; } = new();
		[Reactive] public List<ContextMenuAction> ToolbarItems { get; set; } = new();

		[ObservableAsProperty] public int MaxScrollValue { get; }

		public MemoryToolsDisplayOptionsViewModel Options { get; }

		public MemoryToolsViewModel()
		{
			Options = AddDisposable(new MemoryToolsDisplayOptionsViewModel());
			Config = ConfigManager.Config.Debug.HexEditor;
			FontConfig = ConfigManager.Config.Debug.Font;
			ScrollPosition = 0;

			if(Design.IsDesignMode) {
				return;
			}

			if(DebugWorkspaceManager.Workspace.TblMappings.Length > 0) {
				TblConverter = TblLoader.Load(DebugWorkspaceManager.Workspace.TblMappings);
			}

			UpdateAvailableMemoryTypes();

			AddDisposable(this.WhenAnyValue(x => x.SelectionStart, x => x.SelectionLength).Subscribe(((int start, int length) o) => {
				if(o.length <= 1) {
					StatusBarText = ResourceHelper.GetMessage("HexEditor_Location", o.start.ToString("X2"));
				} else {
					StatusBarText = ResourceHelper.GetMessage("HexEditor_Selection", o.start.ToString("X2"), (o.start + o.length - 1).ToString("X2"), o.length);
				}
			}));

			AddDisposable(ReactiveHelper.RegisterRecursiveObserver(Config, (s, e) => UpdateDataProvider()));
			AddDisposable(this.WhenAnyValue(x => x.TblConverter).Subscribe(x => UpdateDataProvider()));

			AddDisposable(this.WhenAnyValue(
				x => x.Config.MemoryType,
				x => x.Config.BytesPerRow
			).Select(((MemoryType memType, int bytesPerRow) o) => (DebugApi.GetMemorySize(o.memType) / o.bytesPerRow) - 1).ToPropertyEx(this, x => x.MaxScrollValue));
		}

		private void UpdateDataProvider()
		{
			DataProvider = new HexEditorDataProvider(
				Config.MemoryType,
				Config,
				TblConverter
			);
		}

		public void UpdateAvailableMemoryTypes()
		{
			AvailableMemoryTypes = Enum.GetValues<MemoryType>().Where(t => t.SupportsMemoryViewer() && DebugApi.GetMemorySize(t) > 0).Cast<Enum>().ToArray();
			if(!AvailableMemoryTypes.Contains(Config.MemoryType)) {
				Config.MemoryType = (MemoryType)AvailableMemoryTypes.First();
			}

			Options.UpdateAvailableOptions();
		}
	}
}
