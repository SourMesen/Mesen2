using Avalonia.Styling;

namespace Markdown.Avalonia.SyntaxHigh;

public class StyleSetup
{
	public IEnumerable<KeyValuePair<string, Action<Styles>>> GetOverrideStyles()
	{
		return new List<KeyValuePair<string, Action<Styles>>>();
	}
}
