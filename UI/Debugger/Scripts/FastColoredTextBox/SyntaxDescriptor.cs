using System.Collections.Generic;
using System.Text.RegularExpressions;
using System;

namespace FastColoredTextBoxNS
{
    public class SyntaxDescriptor: IDisposable
    {
        public char leftBracket = '(';
        public char rightBracket = ')';
        public char leftBracket2 = '{';
        public char rightBracket2 = '}';
        public BracketsHighlightStrategy bracketsHighlightStrategy = BracketsHighlightStrategy.Strategy2;
        public readonly List<Style> styles = new List<Style>();
        public readonly List<RuleDesc> rules = new List<RuleDesc>();
        public readonly List<FoldingDesc> foldings = new List<FoldingDesc>();

        public void Dispose()
        {
            foreach (var style in styles)
                style.Dispose();
        }
    }

    public class RuleDesc
    {
        Regex regex;
        public string pattern;
        public RegexOptions options = RegexOptions.None;
        public Style style;

        public Regex Regex
        {
            get
            {
                if (regex == null)
                {
                    regex = new Regex(pattern, SyntaxHighlighter.RegexCompiledOption | options);
                }
                return regex;
            }
        }
    }

    public class FoldingDesc
    {
        public string startMarkerRegex;
        public string finishMarkerRegex;
        public RegexOptions options = RegexOptions.None;
    }
}
