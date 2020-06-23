using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmDbgPreferences : BaseConfigForm
	{
		public frmDbgPreferences()
		{
			InitializeComponent();

			ctrlDbgShortcutsShared.Shortcuts = new FieldInfo[] {
				GetMember(nameof(DebuggerShortcutsConfig.IncreaseFontSize)),
				GetMember(nameof(DebuggerShortcutsConfig.DecreaseFontSize)),
				GetMember(nameof(DebuggerShortcutsConfig.ResetFontSize)),
				GetMember(nameof(DebuggerShortcutsConfig.ZoomIn)),
				GetMember(nameof(DebuggerShortcutsConfig.ZoomOut)),
				GetMember(nameof(DebuggerShortcutsConfig.SaveAsPng)),
				GetMember(nameof(DebuggerShortcutsConfig.GoTo)),
				GetMember(nameof(DebuggerShortcutsConfig.Find)),
				GetMember(nameof(DebuggerShortcutsConfig.FindNext)),
				GetMember(nameof(DebuggerShortcutsConfig.FindPrev)),
				GetMember(nameof(DebuggerShortcutsConfig.Undo)),
				GetMember(nameof(DebuggerShortcutsConfig.Cut)),
				GetMember(nameof(DebuggerShortcutsConfig.Copy)),
				GetMember(nameof(DebuggerShortcutsConfig.Paste)),
				GetMember(nameof(DebuggerShortcutsConfig.SelectAll)),
				GetMember(nameof(DebuggerShortcutsConfig.Refresh)),
				GetMember(nameof(DebuggerShortcutsConfig.MarkAsCode)),
				GetMember(nameof(DebuggerShortcutsConfig.MarkAsData)),
				GetMember(nameof(DebuggerShortcutsConfig.MarkAsUnidentified)),
				GetMember(nameof(DebuggerShortcutsConfig.GoToAll)),
				GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_EditInMemoryViewer)),
				GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_ViewInDisassembly)),

				GetMember(nameof(DebuggerShortcutsConfig.OpenAssembler)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenDebugger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenEventViewer)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenMemoryTools)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenRegisterViewer)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenProfiler)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenScriptWindow)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenTraceLogger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenDebugLog)),

				GetMember(nameof(DebuggerShortcutsConfig.OpenTilemapViewer)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenTileViewer)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenSpriteViewer)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenPaletteViewer)),

				GetMember(nameof(DebuggerShortcutsConfig.OpenSpcDebugger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenSa1Debugger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenGameboyDebugger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenGsuDebugger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenNecDspDebugger)),
				GetMember(nameof(DebuggerShortcutsConfig.OpenCx4Debugger)),
			};

			ctrlDbgShortcutsMemoryViewer.Shortcuts = new FieldInfo[] {
				//GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_Freeze)),
				//GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_Unfreeze)),
				GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_AddToWatch)),
				GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_EditBreakpoint)),
				GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_EditLabel)),
				GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_Import)),
				GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_Export)),
				//GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_ViewInCpuMemory)),
				//GetMember(nameof(DebuggerShortcutsConfig.MemoryViewer_ViewInMemoryType))
			};

			ctrlDbgShortcutsScriptWindow.Shortcuts = new FieldInfo[] {
				GetMember(nameof(DebuggerShortcutsConfig.ScriptWindow_OpenScript)),
				GetMember(nameof(DebuggerShortcutsConfig.ScriptWindow_SaveScript)),
				GetMember(nameof(DebuggerShortcutsConfig.ScriptWindow_RunScript)),
				GetMember(nameof(DebuggerShortcutsConfig.ScriptWindow_StopScript))
			};

			ctrlDbgShortcutsDebugger.Shortcuts = new FieldInfo[] {
				GetMember(nameof(DebuggerShortcutsConfig.ReloadRom)),
				GetMember(nameof(DebuggerShortcutsConfig.Reset)),
				GetMember(nameof(DebuggerShortcutsConfig.PowerCycle)),
				GetMember(nameof(DebuggerShortcutsConfig.Continue)),
				GetMember(nameof(DebuggerShortcutsConfig.Break)),
				GetMember(nameof(DebuggerShortcutsConfig.ToggleBreakContinue)),
				GetMember(nameof(DebuggerShortcutsConfig.StepInto)),
				GetMember(nameof(DebuggerShortcutsConfig.StepOver)),
				GetMember(nameof(DebuggerShortcutsConfig.StepOut)),
				//GetMember(nameof(DebuggerShortcutsConfig.StepBack)),
				GetMember(nameof(DebuggerShortcutsConfig.RunPpuCycle)),
				GetMember(nameof(DebuggerShortcutsConfig.RunPpuScanline)),
				GetMember(nameof(DebuggerShortcutsConfig.RunPpuFrame)),
				GetMember(nameof(DebuggerShortcutsConfig.BreakIn)),
				GetMember(nameof(DebuggerShortcutsConfig.BreakOn)),
				//GetMember(nameof(DebuggerShortcutsConfig.FindOccurrences)),
				GetMember(nameof(DebuggerShortcutsConfig.GoToProgramCounter)),
				//GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_SetNextStatement)),
				//GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_EditSubroutine)),
				GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_EditSelectedCode)),
				//GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_EditSourceFile)),
				GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_EditLabel)),
				//GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_NavigateBack)),
				//GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_NavigateForward)),
				GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_ToggleBreakpoint)),
				GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_DisableEnableBreakpoint)),
				GetMember(nameof(DebuggerShortcutsConfig.CodeWindow_SwitchView)),
				//GetMember(nameof(DebuggerShortcutsConfig.FunctionList_EditLabel)),
				//GetMember(nameof(DebuggerShortcutsConfig.FunctionList_AddBreakpoint)),
				//GetMember(nameof(DebuggerShortcutsConfig.FunctionList_FindOccurrences)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_Add)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_Edit)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_Delete)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_AddBreakpoint)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_AddToWatch)),
				//GetMember(nameof(DebuggerShortcutsConfig.LabelList_FindOccurrences)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_ViewInCpuMemory)),
				GetMember(nameof(DebuggerShortcutsConfig.LabelList_ViewInMemoryType)),
				GetMember(nameof(DebuggerShortcutsConfig.BreakpointList_Add)),
				GetMember(nameof(DebuggerShortcutsConfig.BreakpointList_Edit)),
				GetMember(nameof(DebuggerShortcutsConfig.BreakpointList_GoToLocation)),
				GetMember(nameof(DebuggerShortcutsConfig.BreakpointList_Delete)),
				GetMember(nameof(DebuggerShortcutsConfig.WatchList_Delete)),
				GetMember(nameof(DebuggerShortcutsConfig.WatchList_MoveUp)),
				GetMember(nameof(DebuggerShortcutsConfig.WatchList_MoveDown)),
				//GetMember(nameof(DebuggerShortcutsConfig.SaveRom)),
				GetMember(nameof(DebuggerShortcutsConfig.SaveRomAs)),
				GetMember(nameof(DebuggerShortcutsConfig.SaveEditAsIps)),
				//GetMember(nameof(DebuggerShortcutsConfig.RevertPrgChrChanges)),
				//GetMember(nameof(DebuggerShortcutsConfig.ToggleVerifiedData)),
				//GetMember(nameof(DebuggerShortcutsConfig.ToggleUnidentifiedCodeData))
			};
		}

		private FieldInfo GetMember(string name)
		{
			return typeof(DebuggerShortcutsConfig).GetField(name);
		}

		protected override void OnFormClosed(FormClosedEventArgs e)
		{
			base.OnFormClosed(e);
			if(DialogResult == DialogResult.OK) {
				DebuggerShortcutsConfig.UpdateMenus();
			}
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			DebuggerShortcutsConfig defaults = new DebuggerShortcutsConfig();
			foreach(FieldInfo field in typeof(DebuggerShortcutsConfig).GetFields()) {
				field.SetValue(ConfigManager.Config.Debug.Shortcuts, field.GetValue(defaults));
			}
			ctrlDbgShortcutsDebugger.InitializeGrid();
			ctrlDbgShortcutsMemoryViewer.InitializeGrid();
			ctrlDbgShortcutsScriptWindow.InitializeGrid();
			ctrlDbgShortcutsShared.InitializeGrid();
		}
	}
}
