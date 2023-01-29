using Avalonia;
using Avalonia.Media;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class DebuggerFontConfig : BaseConfig<DebuggerFontConfig>
	{
		[Reactive] public FontConfig DisassemblyFont { get; set; } = PreferencesConfig.GetDefaultMonospaceFont();
		[Reactive] public FontConfig MemoryViewerFont { get; set; } = PreferencesConfig.GetDefaultMonospaceFont();
		[Reactive] public FontConfig AssemblerFont { get; set; } = PreferencesConfig.GetDefaultMonospaceFont();
		[Reactive] public FontConfig ScriptWindowFont { get; set; } = PreferencesConfig.GetDefaultMonospaceFont();
		[Reactive] public FontConfig OtherMonoFont { get; set; } = PreferencesConfig.GetDefaultMonospaceFont(true);

		public void ApplyConfig()
		{
			if(Application.Current is Application app) {
				if((app.Resources["MesenDisassemblyFont"] as FontFamily)?.Name != DisassemblyFont.FontFamily) {
					app.Resources["MesenDisassemblyFont"] = new FontFamily(DisassemblyFont.FontFamily);
				}
				if((app.Resources["MesenMemoryViewerFont"] as FontFamily)?.Name != MemoryViewerFont.FontFamily) {
					app.Resources["MesenMemoryViewerFont"] = new FontFamily(MemoryViewerFont.FontFamily);
				}
				if((app.Resources["MesenAssemblerFont"] as FontFamily)?.Name != AssemblerFont.FontFamily) {
					app.Resources["MesenAssemblerFont"] = new FontFamily(AssemblerFont.FontFamily);
				}
				if((app.Resources["MesenScriptWindowFont"] as FontFamily)?.Name != ScriptWindowFont.FontFamily) {
					app.Resources["MesenScriptWindowFont"] = new FontFamily(ScriptWindowFont.FontFamily);
				}
				if((app.Resources["MesenMonospaceFont"] as FontFamily)?.Name != OtherMonoFont.FontFamily) {
					app.Resources["MesenMonospaceFont"] = new FontFamily(OtherMonoFont.FontFamily);
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
