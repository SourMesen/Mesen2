using Avalonia.Controls;
using AvaloniaEdit;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using AvaloniaEdit.TextMate;
using Markdown.Avalonia.SyntaxHigh;
using System.Collections.Generic;
using System.Reflection;
using System.Xml;
using TextMateSharp.Grammars;

namespace Mesen;

public class MarkdownCodeBlockHelper
{
	private static XshdSyntaxDefinition _syntaxDef = null!;
	private static IHighlightingDefinition _highlighting = null!;
	private static Dictionary<string, Language> cache = new Dictionary<string, Language>();
	private static RegistryOptions options = new RegistryOptions(ThemeName.LightPlus);
	public static void Install()
	{
		using XmlReader reader = XmlReader.Create(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.HighlightLua.xshd")!);
		_syntaxDef = HighlightingLoader.LoadXshd(reader);
		_highlighting = HighlightingLoader.Load(_syntaxDef, HighlightingManager.Instance);

		SyntaxSetup.CreateCodeBlock = CreateCodeBlock;
	}

	private static Control CreateCodeBlock(string lang, string code)
	{
		var textEditor = new TextEditor() {
			Tag = lang,
			Text = code,
			IsReadOnly = true,
		};

		if("lua".Equals(lang)) {
			//
			// It seems that TextMate renders slowly.
			// Syntax definition may duplicate with `ScriptWindow`.
			// And this way works fine.
			// 
			// It's suggested to write another syntax definition for this.
			// Because some of this code doesn't belong to lua.
			// Such as type annotation for parameter `i` in following:
			// ```lua
			// function foo(i: integer)
			// end
			// ```
			//
			// I think all of these work is adequate for a simple code editing.
			//
			textEditor.SyntaxHighlighting = _highlighting;
		} else {
			var installation = textEditor.InstallTextMate(options);
			var ok = cache.TryGetValue(lang, out var language);
			if(!ok) {
				language = options.GetLanguageByExtension("." + lang);
				cache[lang] = language;
			}
			installation.SetGrammar(options.GetScopeByLanguageId(language?.Id));
		}

		return textEditor;
	}
}
