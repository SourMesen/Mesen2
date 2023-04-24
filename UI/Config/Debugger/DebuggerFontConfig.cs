using Avalonia;
using Avalonia.Media;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class DebuggerFontConfig : BaseConfig<DebuggerFontConfig>
	{
		[Reactive] public FontConfig DisassemblyFont { get; set; } = new() { FontFamily = "Consolas", FontSize = 14 };
		[Reactive] public FontConfig MemoryViewerFont { get; set; } = new() { FontFamily = "Consolas", FontSize = 14 };
		[Reactive] public FontConfig AssemblerFont { get; set; } = new() { FontFamily = "Consolas", FontSize = 14 };
		[Reactive] public FontConfig ScriptWindowFont { get; set; } = new() { FontFamily = "Consolas", FontSize = 14 };
		[Reactive] public FontConfig OtherMonoFont { get; set; } = new() { FontFamily = "Consolas", FontSize = 12 };

		public void ApplyConfig()
		{
			if(Application.Current is Application app) {
				string disFont = Configuration.GetValidFontFamily(DisassemblyFont.FontFamily, true);
				string memViewerFont = Configuration.GetValidFontFamily(MemoryViewerFont.FontFamily, true);
				string assemblerFont = Configuration.GetValidFontFamily(AssemblerFont.FontFamily, true);
				string scriptFont = Configuration.GetValidFontFamily(ScriptWindowFont.FontFamily, true);
				string otherFont = Configuration.GetValidFontFamily(OtherMonoFont.FontFamily, true);

				if((app.Resources["MesenDisassemblyFont"] as FontFamily)?.Name != disFont) {
					app.Resources["MesenDisassemblyFont"] = new FontFamily(disFont);
				}
				if((app.Resources["MesenMemoryViewerFont"] as FontFamily)?.Name != memViewerFont) {
					app.Resources["MesenMemoryViewerFont"] = new FontFamily(memViewerFont);
				}
				if((app.Resources["MesenAssemblerFont"] as FontFamily)?.Name != assemblerFont) {
					app.Resources["MesenAssemblerFont"] = new FontFamily(assemblerFont);
				}
				if((app.Resources["MesenScriptWindowFont"] as FontFamily)?.Name != scriptFont) {
					app.Resources["MesenScriptWindowFont"] = new FontFamily(scriptFont);
				}
				if((app.Resources["MesenMonospaceFont"] as FontFamily)?.Name != otherFont) {
					app.Resources["MesenMonospaceFont"] = new FontFamily(otherFont);
				}

				if((double?)app.Resources["MesenDisassemblyFontSize"] != DisassemblyFont.FontSize) {
					app.Resources["MesenDisassemblyFontSize"] = (double)DisassemblyFont.FontSize;
				}
				if((double?)app.Resources["MesenMemoryViewerFontSize"] != MemoryViewerFont.FontSize) {
					app.Resources["MesenMemoryViewerFontSize"] = (double)MemoryViewerFont.FontSize;
				}
				if((double?)app.Resources["MesenAssemblerFontSize"] != AssemblerFont.FontSize) {
					app.Resources["MesenAssemblerFontSize"] = (double)AssemblerFont.FontSize;
				}
				if((double?)app.Resources["MesenScriptWindowFontSize"] != ScriptWindowFont.FontSize) {
					app.Resources["MesenScriptWindowFontSize"] = (double)ScriptWindowFont.FontSize;
				}
				if((double?)app.Resources["MesenMonospaceFontSize"] != OtherMonoFont.FontSize) {
					app.Resources["MesenMonospaceFontSize"] = (double)OtherMonoFont.FontSize;
				}
			}
		}
	}
}
