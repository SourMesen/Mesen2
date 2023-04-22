using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Media;
using System.Text.RegularExpressions;

namespace Markdown.Avalonia.SyntaxHigh;

// It seems that the Markdown.Avalonia 11.0.0-b1 doesn't have a good support for lua 
// syntax highlight, and in version v0.10.14 I can find the ways to custom it 
// but there is no way to custom in 11.0.0-b1.
//
// So I write this class as a "plugin" to resolve this problem.
// And this should be followed as below:
//		https://github.com/whistyun/Markdown.Avalonia/blob/8aec868dafe7798e0a71667bb4d587cd05760b7a/Markdown.Avalonia.Tight/Markdown.cs#L43-L47
//		https://github.com/whistyun/Markdown.Avalonia/blob/v11.0.0-b1/Markdown.Avalonia.SyntaxHigh/SyntaxSetup.cs
// 
// What's more, styles haven't been adapted, which can be referenced from:
//		https://github.com/whistyun/Markdown.Avalonia/blob/8aec868dafe7798e0a71667bb4d587cd05760b7a/Markdown.Avalonia.Tight/MarkdownStyle.cs#L26-L30
//		https://github.com/whistyun/Markdown.Avalonia/blob/v11.0.0-b1/Markdown.Avalonia.SyntaxHigh/StyleSetup.cs
//
// And this may be removed if Markdown.Avalonia updates.

public class SyntaxSetup
{
	public static Func<string, string, Control>? CreateCodeBlock;

	public IEnumerable<KeyValuePair<string, Func<Match, Control>>> GetOverrideConverters()
	{
		yield return new KeyValuePair<string, Func<Match, Control>>(
			 "CodeBlocksWithLangEvaluator",
			 CodeBlocksEvaluator);
	}

	private Border CodeBlocksEvaluator(Match match)
	{
		var lang = match.Groups[2].Value;
		var code = match.Groups[3].Value;

		if(string.IsNullOrEmpty(lang)) {
			var ctxt = new TextBlock() {
				Text = code,
				TextWrapping = TextWrapping.NoWrap
			};
			ctxt.Classes.Add(Markdown.CodeBlockClass);

			var scrl = new ScrollViewer();
			scrl.Classes.Add(Markdown.CodeBlockClass);
			scrl.Content = ctxt;
			scrl.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;

			var result = new Border();
			result.Classes.Add(Markdown.CodeBlockClass);
			result.Child = scrl;

			return result;
		} else {
			// check wheither style is set
			//if(!ThemeDetector.IsAvalonEditSetup) {
			//	SetupStyle();
			//}

			//var txtEdit = new TextEditor();
			//txtEdit.Tag = lang;

			//txtEdit.Text = code;
			//txtEdit.HorizontalAlignment = HorizontalAlignment.Stretch;
			//txtEdit.IsReadOnly = true;

			var result = new Border();
			result.Classes.Add(Markdown.CodeBlockClass);
			if(CreateCodeBlock != null)
				result.Child = CreateCodeBlock(lang, code);

			return result;
		}
	}

}