using System;
using System.Collections.Generic;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Text.RegularExpressions;
using System.Xml;

namespace FastColoredTextBoxNS
{
    public class SyntaxHighlighter : IDisposable
    {
        //styles
        protected static readonly Platform platformType = PlatformType.GetOperationSystemPlatform();
        public readonly Style BlueBoldStyle = new TextStyle(Brushes.Blue, null, FontStyle.Bold);
        public readonly Style BlueStyle = new TextStyle(Brushes.Blue, null, FontStyle.Regular);
        public readonly Style BoldStyle = new TextStyle(null, null, FontStyle.Bold | FontStyle.Underline);
        public readonly Style BrownStyle = new TextStyle(Brushes.Brown, null, FontStyle.Italic);
        public readonly Style GrayStyle = new TextStyle(Brushes.Gray, null, FontStyle.Regular);
        public readonly Style GreenStyle = new TextStyle(Brushes.Green, null, FontStyle.Italic);
        public readonly Style MagentaStyle = new TextStyle(Brushes.Magenta, null, FontStyle.Regular);
        public readonly Style MaroonStyle = new TextStyle(Brushes.Maroon, null, FontStyle.Regular);
        public readonly Style RedStyle = new TextStyle(Brushes.Red, null, FontStyle.Regular);
        public readonly Style BlackStyle = new TextStyle(Brushes.Black, null, FontStyle.Regular);
        //
        protected readonly Dictionary<string, SyntaxDescriptor> descByXMLfileNames =
            new Dictionary<string, SyntaxDescriptor>();

        protected Regex CSharpAttributeRegex,
                      CSharpClassNameRegex;

        protected Regex CSharpCommentRegex1,
                      CSharpCommentRegex2,
                      CSharpCommentRegex3;

        protected Regex CSharpKeywordRegex;
        protected Regex CSharpNumberRegex;
        protected Regex CSharpStringRegex;

        protected Regex HTMLAttrRegex,
                      HTMLAttrValRegex,
                      HTMLCommentRegex1,
                      HTMLCommentRegex2;

        protected Regex HTMLEndTagRegex;

        protected Regex HTMLEntityRegex,
                      HTMLTagContentRegex;

        protected Regex HTMLTagNameRegex;
        protected Regex HTMLTagRegex;

        protected Regex XMLAttrRegex,
                      XMLAttrValRegex,
                      XMLCommentRegex1,
                      XMLCommentRegex2;

        protected Regex XMLEndTagRegex;

        protected Regex XMLEntityRegex,
                      XMLTagContentRegex;

        protected Regex XMLTagNameRegex;
        protected Regex XMLTagRegex;
        protected Regex XMLCDataRegex;
        protected Regex XMLFoldingRegex;

        protected Regex JScriptCommentRegex1,
                      JScriptCommentRegex2,
                      JScriptCommentRegex3;

        protected Regex JScriptKeywordRegex;
        protected Regex JScriptNumberRegex;
        protected Regex JScriptStringRegex;

        protected Regex LuaCommentRegex1,
                      LuaCommentRegex2,
                      LuaCommentRegex3;

        protected Regex LuaKeywordRegex;
        protected Regex LuaNumberRegex;
        protected Regex LuaStringRegex;
        protected Regex LuaFunctionsRegex;

        protected Regex PHPCommentRegex1,
                      PHPCommentRegex2,
                      PHPCommentRegex3;

        protected Regex PHPKeywordRegex1,
                      PHPKeywordRegex2,
                      PHPKeywordRegex3;

        protected Regex PHPNumberRegex;
        protected Regex PHPStringRegex;
        protected Regex PHPVarRegex;

        protected Regex SQLCommentRegex1,
                      SQLCommentRegex2,
                      SQLCommentRegex3;

        protected Regex SQLFunctionsRegex;
        protected Regex SQLKeywordsRegex;
        protected Regex SQLNumberRegex;
        protected Regex SQLStatementsRegex;
        protected Regex SQLStringRegex;
        protected Regex SQLTypesRegex;
        protected Regex SQLVarRegex;
        protected Regex VBClassNameRegex;
        protected Regex VBCommentRegex;
        protected Regex VBKeywordRegex;
        protected Regex VBNumberRegex;
        protected Regex VBStringRegex;

        public static RegexOptions RegexCompiledOption
        {
            get
            {
                return RegexOptions.Compiled;
            }
        }

        #region IDisposable Members

        public void Dispose()
        {
            foreach (SyntaxDescriptor desc in descByXMLfileNames.Values)
                desc.Dispose();
        }

        #endregion

        /// <summary>
        /// Highlights syntax for given language
        /// </summary>
        public virtual void HighlightSyntax(Language language, Range range)
        {
            switch (language)
            {
                case Language.CSharp:
                    CSharpSyntaxHighlight(range);
                    break;
                case Language.VB:
                    VBSyntaxHighlight(range);
                    break;
                case Language.HTML:
                    HTMLSyntaxHighlight(range);
                    break;
                case Language.XML:
                    XMLSyntaxHighlight(range);
                    break;
                case Language.SQL:
                    SQLSyntaxHighlight(range);
                    break;
                case Language.PHP:
                    PHPSyntaxHighlight(range);
                    break;
                case Language.JS:
                    JScriptSyntaxHighlight(range);
                    break;
                case Language.Lua:
                    LuaSyntaxHighlight(range);
                    break;
                default:
                    break;
            }
        }

        public virtual void AutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            var tb = (sender as FastColoredTextBox);
            Language language = tb.Language;
            switch (language)
            {
                case Language.CSharp:
                    CSharpAutoIndentNeeded(sender, args);
                    break;
                case Language.VB:
                    VBAutoIndentNeeded(sender, args);
                    break;
                case Language.HTML:
                    HTMLAutoIndentNeeded(sender, args);
                    break;
                case Language.XML:
                    XMLAutoIndentNeeded(sender, args);
                    break;
                case Language.SQL:
                    SQLAutoIndentNeeded(sender, args);
                    break;
                case Language.PHP:
                    PHPAutoIndentNeeded(sender, args);
                    break;
                case Language.JS:
                    CSharpAutoIndentNeeded(sender, args);
                    break; //JS like C#
                case Language.Lua:
                    LuaAutoIndentNeeded(sender, args);
                    break;
                default:
                    break;
            }
        }

        protected void PHPAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            /*
            FastColoredTextBox tb = sender as FastColoredTextBox;
            tb.CalcAutoIndentShiftByCodeFolding(sender, args);*/
            //block {}
            if (Regex.IsMatch(args.LineText, @"^[^""']*\{.*\}[^""']*$"))
                return;
            //start of block {}
            if (Regex.IsMatch(args.LineText, @"^[^""']*\{"))
            {
                args.ShiftNextLines = args.TabLength;
                return;
            }
            //end of block {}
            if (Regex.IsMatch(args.LineText, @"}[^""']*$"))
            {
                args.Shift = -args.TabLength;
                args.ShiftNextLines = -args.TabLength;
                return;
            }
            //is unclosed operator in previous line ?
            if (Regex.IsMatch(args.PrevLineText, @"^\s*(if|for|foreach|while|[\}\s]*else)\b[^{]*$"))
                if (!Regex.IsMatch(args.PrevLineText, @"(;\s*$)|(;\s*//)")) //operator is unclosed
                {
                    args.Shift = args.TabLength;
                    return;
                }
        }

        protected void SQLAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            var tb = sender as FastColoredTextBox;
            tb.CalcAutoIndentShiftByCodeFolding(sender, args);
        }

        protected void HTMLAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            var tb = sender as FastColoredTextBox;
            tb.CalcAutoIndentShiftByCodeFolding(sender, args);
        }

        protected void XMLAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            var tb = sender as FastColoredTextBox;
            tb.CalcAutoIndentShiftByCodeFolding(sender, args);
        }

        protected void VBAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            //end of block
            if (Regex.IsMatch(args.LineText, @"^\s*(End|EndIf|Next|Loop)\b", RegexOptions.IgnoreCase))
            {
                args.Shift = -args.TabLength;
                args.ShiftNextLines = -args.TabLength;
                return;
            }
            //start of declaration
            if (Regex.IsMatch(args.LineText,
                              @"\b(Class|Property|Enum|Structure|Sub|Function|Namespace|Interface|Get)\b|(Set\s*\()",
                              RegexOptions.IgnoreCase))
            {
                args.ShiftNextLines = args.TabLength;
                return;
            }
            // then ...
            if (Regex.IsMatch(args.LineText, @"\b(Then)\s*\S+", RegexOptions.IgnoreCase))
                return;
            //start of operator block
            if (Regex.IsMatch(args.LineText, @"^\s*(If|While|For|Do|Try|With|Using|Select)\b", RegexOptions.IgnoreCase))
            {
                args.ShiftNextLines = args.TabLength;
                return;
            }

            //Statements else, elseif, case etc
            if (Regex.IsMatch(args.LineText, @"^\s*(Else|ElseIf|Case|Catch|Finally)\b", RegexOptions.IgnoreCase))
            {
                args.Shift = -args.TabLength;
                return;
            }

            //Char _
            if (args.PrevLineText.TrimEnd().EndsWith("_"))
            {
                args.Shift = args.TabLength;
                return;
            }
        }

        protected void CSharpAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            //block {}
            if (Regex.IsMatch(args.LineText, @"^[^""']*\{.*\}[^""']*$"))
                return;
            //start of block {}
            if (Regex.IsMatch(args.LineText, @"^[^""']*\{"))
            {
                args.ShiftNextLines = args.TabLength;
                return;
            }
            //end of block {}
            if (Regex.IsMatch(args.LineText, @"}[^""']*$"))
            {
                args.Shift = -args.TabLength;
                args.ShiftNextLines = -args.TabLength;
                return;
            }
            //label
            if (Regex.IsMatch(args.LineText, @"^\s*\w+\s*:\s*($|//)") &&
                !Regex.IsMatch(args.LineText, @"^\s*default\s*:"))
            {
                args.Shift = -args.TabLength;
                return;
            }
            //some statements: case, default
            if (Regex.IsMatch(args.LineText, @"^\s*(case|default)\b.*:\s*($|//)"))
            {
                args.Shift = -args.TabLength / 2;
                return;
            }
            //is unclosed operator in previous line ?
            if (Regex.IsMatch(args.PrevLineText, @"^\s*(if|for|foreach|while|[\}\s]*else)\b[^{]*$"))
                if (!Regex.IsMatch(args.PrevLineText, @"(;\s*$)|(;\s*//)")) //operator is unclosed
                {
                    args.Shift = args.TabLength;
                    return;
                }
        }

        public static SyntaxDescriptor ParseXmlDescription(XmlDocument doc)
        {
            var desc = new SyntaxDescriptor();
            XmlNode brackets = doc.SelectSingleNode("doc/brackets");
            if (brackets != null)
            {
                if (brackets.Attributes["left"] == null || brackets.Attributes["right"] == null ||
                    brackets.Attributes["left"].Value == "" || brackets.Attributes["right"].Value == "")
                {
                    desc.leftBracket = '\x0';
                    desc.rightBracket = '\x0';
                }
                else
                {
                    desc.leftBracket = brackets.Attributes["left"].Value[0];
                    desc.rightBracket = brackets.Attributes["right"].Value[0];
                }

                if (brackets.Attributes["left2"] == null || brackets.Attributes["right2"] == null ||
                    brackets.Attributes["left2"].Value == "" || brackets.Attributes["right2"].Value == "")
                {
                    desc.leftBracket2 = '\x0';
                    desc.rightBracket2 = '\x0';
                }
                else
                {
                    desc.leftBracket2 = brackets.Attributes["left2"].Value[0];
                    desc.rightBracket2 = brackets.Attributes["right2"].Value[0];
                }

                if (brackets.Attributes["strategy"] == null || brackets.Attributes["strategy"].Value == "")
                    desc.bracketsHighlightStrategy = BracketsHighlightStrategy.Strategy2;
                else
                    desc.bracketsHighlightStrategy = (BracketsHighlightStrategy)Enum.Parse(typeof(BracketsHighlightStrategy), brackets.Attributes["strategy"].Value);
            }

            var styleByName = new Dictionary<string, Style>();

            foreach (XmlNode style in doc.SelectNodes("doc/style"))
            {
                Style s = ParseStyle(style);
                styleByName[style.Attributes["name"].Value] = s;
                desc.styles.Add(s);
            }
            foreach (XmlNode rule in doc.SelectNodes("doc/rule"))
                desc.rules.Add(ParseRule(rule, styleByName));
            foreach (XmlNode folding in doc.SelectNodes("doc/folding"))
                desc.foldings.Add(ParseFolding(folding));

            return desc;
        }

        protected static FoldingDesc ParseFolding(XmlNode foldingNode)
        {
            var folding = new FoldingDesc();
            //regex
            folding.startMarkerRegex = foldingNode.Attributes["start"].Value;
            folding.finishMarkerRegex = foldingNode.Attributes["finish"].Value;
            //options
            XmlAttribute optionsA = foldingNode.Attributes["options"];
            if (optionsA != null)
                folding.options = (RegexOptions)Enum.Parse(typeof(RegexOptions), optionsA.Value);

            return folding;
        }

        protected static RuleDesc ParseRule(XmlNode ruleNode, Dictionary<string, Style> styles)
        {
            var rule = new RuleDesc();
            rule.pattern = ruleNode.InnerText;
            //
            XmlAttribute styleA = ruleNode.Attributes["style"];
            XmlAttribute optionsA = ruleNode.Attributes["options"];
            //Style
            if (styleA == null)
                throw new Exception("Rule must contain style name.");
            if (!styles.ContainsKey(styleA.Value))
                throw new Exception("Style '" + styleA.Value + "' is not found.");
            rule.style = styles[styleA.Value];
            //options
            if (optionsA != null)
                rule.options = (RegexOptions)Enum.Parse(typeof(RegexOptions), optionsA.Value);

            return rule;
        }

        protected static Style ParseStyle(XmlNode styleNode)
        {
            XmlAttribute typeA = styleNode.Attributes["type"];
            XmlAttribute colorA = styleNode.Attributes["color"];
            XmlAttribute backColorA = styleNode.Attributes["backColor"];
            XmlAttribute fontStyleA = styleNode.Attributes["fontStyle"];
            XmlAttribute nameA = styleNode.Attributes["name"];
            //colors
            SolidBrush foreBrush = null;
            if (colorA != null)
                foreBrush = new SolidBrush(ParseColor(colorA.Value));
            SolidBrush backBrush = null;
            if (backColorA != null)
                backBrush = new SolidBrush(ParseColor(backColorA.Value));
            //fontStyle
            FontStyle fontStyle = FontStyle.Regular;
            if (fontStyleA != null)
                fontStyle = (FontStyle)Enum.Parse(typeof(FontStyle), fontStyleA.Value);

            return new TextStyle(foreBrush, backBrush, fontStyle);
        }

        protected static Color ParseColor(string s)
        {
            if (s.StartsWith("#"))
            {
                if (s.Length <= 7)
                    return Color.FromArgb(255,
                                          Color.FromArgb(Int32.Parse(s.Substring(1), NumberStyles.AllowHexSpecifier)));
                else
                    return Color.FromArgb(Int32.Parse(s.Substring(1), NumberStyles.AllowHexSpecifier));
            }
            else
                return Color.FromName(s);
        }

        public void HighlightSyntax(SyntaxDescriptor desc, Range range)
        {
            //set style order
            range.tb.ClearStylesBuffer();
            for (int i = 0; i < desc.styles.Count; i++)
                range.tb.Styles[i] = desc.styles[i];
            //brackets
            char[] oldBrackets = RememberBrackets(range.tb);
            range.tb.LeftBracket = desc.leftBracket;
            range.tb.RightBracket = desc.rightBracket;
            range.tb.LeftBracket2 = desc.leftBracket2;
            range.tb.RightBracket2 = desc.rightBracket2;
            //clear styles of range
            range.ClearStyle(desc.styles.ToArray());
            //highlight syntax
            foreach (RuleDesc rule in desc.rules)
                range.SetStyle(rule.style, rule.Regex);
            //clear folding
            range.ClearFoldingMarkers();
            //folding markers
            foreach (FoldingDesc folding in desc.foldings)
                range.SetFoldingMarkers(folding.startMarkerRegex, folding.finishMarkerRegex, folding.options);

            //
            RestoreBrackets(range.tb, oldBrackets);
        }

        protected void RestoreBrackets(FastColoredTextBox tb, char[] oldBrackets)
        {
            tb.LeftBracket = oldBrackets[0];
            tb.RightBracket = oldBrackets[1];
            tb.LeftBracket2 = oldBrackets[2];
            tb.RightBracket2 = oldBrackets[3];
        }

        protected char[] RememberBrackets(FastColoredTextBox tb)
        {
            return new[] { tb.LeftBracket, tb.RightBracket, tb.LeftBracket2, tb.RightBracket2 };
        }

        protected void InitCShaprRegex()
        {
            //CSharpStringRegex = new Regex( @"""""|@""""|''|@"".*?""|(?<!@)(?<range>"".*?[^\\]"")|'.*?[^\\]'", RegexCompiledOption);

            CSharpStringRegex =
                new Regex(
                    @"
                            # Character definitions:
                            '
                            (?> # disable backtracking
                              (?:
                                \\[^\r\n]|    # escaped meta char
                                [^'\r\n]      # any character except '
                              )*
                            )
                            '?
                            |
                            # Normal string & verbatim strings definitions:
                            (?<verbatimIdentifier>@)?         # this group matches if it is an verbatim string
                            ""
                            (?> # disable backtracking
                              (?:
                                # match and consume an escaped character including escaped double quote ("") char
                                (?(verbatimIdentifier)        # if it is a verbatim string ...
                                  """"|                         #   then: only match an escaped double quote ("") char
                                  \\.                         #   else: match an escaped sequence
                                )
                                | # OR
            
                                # match any char except double quote char ("")
                                [^""]
                              )*
                            )
                            ""
                        ",
                    RegexOptions.ExplicitCapture | RegexOptions.Singleline | RegexOptions.IgnorePatternWhitespace |
                    RegexCompiledOption
                    ); //thanks to rittergig for this regex

            CSharpCommentRegex1 = new Regex(@"//.*$", RegexOptions.Multiline | RegexCompiledOption);
            CSharpCommentRegex2 = new Regex(@"(/\*.*?\*/)|(/\*.*)", RegexOptions.Singleline | RegexCompiledOption);
            CSharpCommentRegex3 = new Regex(@"(/\*.*?\*/)|(.*\*/)",
                                            RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            CSharpNumberRegex = new Regex(@"\b\d+[\.]?\d*([eE]\-?\d+)?[lLdDfF]?\b|\b0x[a-fA-F\d]+\b",
                                          RegexCompiledOption);
            CSharpAttributeRegex = new Regex(@"^\s*(?<range>\[.+?\])\s*$", RegexOptions.Multiline | RegexCompiledOption);
            CSharpClassNameRegex = new Regex(@"\b(class|struct|enum|interface)\s+(?<range>\w+?)\b", RegexCompiledOption);
            CSharpKeywordRegex =
                new Regex(
                    @"\b(abstract|as|base|bool|break|byte|case|catch|char|checked|class|const|continue|decimal|default|delegate|do|double|else|enum|event|explicit|extern|false|finally|fixed|float|for|foreach|goto|if|implicit|in|int|interface|internal|is|lock|long|namespace|new|null|object|operator|out|override|params|private|protected|public|readonly|ref|return|sbyte|sealed|short|sizeof|stackalloc|static|string|struct|switch|this|throw|true|try|typeof|uint|ulong|unchecked|unsafe|ushort|using|virtual|void|volatile|while|add|alias|ascending|descending|dynamic|from|get|global|group|into|join|let|orderby|partial|remove|select|set|value|var|where|yield)\b|#region\b|#endregion\b",
                    RegexCompiledOption);
        }

        public void InitStyleSchema(Language lang)
        {
            switch (lang)
            {
                case Language.CSharp:
                    StringStyle = BrownStyle;
                    CommentStyle = GreenStyle;
                    NumberStyle = MagentaStyle;
                    AttributeStyle = GreenStyle;
                    ClassNameStyle = BoldStyle;
                    KeywordStyle = BlueStyle;
                    CommentTagStyle = GrayStyle;
                    break;
                case Language.VB:
                    StringStyle = BrownStyle;
                    CommentStyle = GreenStyle;
                    NumberStyle = MagentaStyle;
                    ClassNameStyle = BoldStyle;
                    KeywordStyle = BlueStyle;
                    break;
                case Language.HTML:
                    CommentStyle = GreenStyle;
                    TagBracketStyle = BlueStyle;
                    TagNameStyle = MaroonStyle;
                    AttributeStyle = RedStyle;
                    AttributeValueStyle = BlueStyle;
                    HtmlEntityStyle = RedStyle;
                    break;
                case Language.XML:
                    CommentStyle = GreenStyle;
                    XmlTagBracketStyle = BlueStyle;
                    XmlTagNameStyle = MaroonStyle;
                    XmlAttributeStyle = RedStyle;
                    XmlAttributeValueStyle = BlueStyle;
                    XmlEntityStyle = RedStyle;
                    XmlCDataStyle = BlackStyle;
                    break;
                case Language.JS:
                    StringStyle = BrownStyle;
                    CommentStyle = GreenStyle;
                    NumberStyle = MagentaStyle;
                    KeywordStyle = BlueStyle;
                    break;
                case Language.Lua:
                    StringStyle = BrownStyle;
                    CommentStyle = GreenStyle;
                    NumberStyle = MagentaStyle;
                    KeywordStyle = BlueBoldStyle;
                    FunctionsStyle = MaroonStyle;
                    break;
                case Language.PHP:
                    StringStyle = RedStyle;
                    CommentStyle = GreenStyle;
                    NumberStyle = RedStyle;
                    VariableStyle = MaroonStyle;
                    KeywordStyle = MagentaStyle;
                    KeywordStyle2 = BlueStyle;
                    KeywordStyle3 = GrayStyle;
                    break;
                case Language.SQL:
                    StringStyle = RedStyle;
                    CommentStyle = GreenStyle;
                    NumberStyle = MagentaStyle;
                    KeywordStyle = BlueBoldStyle;
                    StatementsStyle = BlueBoldStyle;
                    FunctionsStyle = MaroonStyle;
                    VariableStyle = MaroonStyle;
                    TypesStyle = BrownStyle;
                    break;
            }
        }

        /// <summary>
        /// Highlights C# code
        /// </summary>
        /// <param name="range"></param>
        public virtual void CSharpSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = "//";
            range.tb.LeftBracket = '(';
            range.tb.RightBracket = ')';
            range.tb.LeftBracket2 = '{';
            range.tb.RightBracket2 = '}';
            range.tb.BracketsHighlightStrategy = BracketsHighlightStrategy.Strategy2;

            range.tb.AutoIndentCharsPatterns
                = @"
^\s*[\w\.]+(\s\w+)?\s*(?<range>=)\s*(?<range>[^;]+);
^\s*(case|default)\s*[^:]*(?<range>:)\s*(?<range>[^;]+);
";
            //clear style of changed range
            range.ClearStyle(StringStyle, CommentStyle, NumberStyle, AttributeStyle, ClassNameStyle, KeywordStyle);
            //
            if (CSharpStringRegex == null)
                InitCShaprRegex();
            //string highlighting
            range.SetStyle(StringStyle, CSharpStringRegex);
            //comment highlighting
            range.SetStyle(CommentStyle, CSharpCommentRegex1);
            range.SetStyle(CommentStyle, CSharpCommentRegex2);
            range.SetStyle(CommentStyle, CSharpCommentRegex3);
            //number highlighting
            range.SetStyle(NumberStyle, CSharpNumberRegex);
            //attribute highlighting
            range.SetStyle(AttributeStyle, CSharpAttributeRegex);
            //class name highlighting
            range.SetStyle(ClassNameStyle, CSharpClassNameRegex);
            //keyword highlighting
            range.SetStyle(KeywordStyle, CSharpKeywordRegex);

            //find document comments
            foreach (Range r in range.GetRanges(@"^\s*///.*$", RegexOptions.Multiline))
            {
                //remove C# highlighting from this fragment
                r.ClearStyle(StyleIndex.All);
                //do XML highlighting
                if (HTMLTagRegex == null)
                    InitHTMLRegex();
                //
                r.SetStyle(CommentStyle);
                //tags
                foreach (Range rr in r.GetRanges(HTMLTagContentRegex))
                {
                    rr.ClearStyle(StyleIndex.All);
                    rr.SetStyle(CommentTagStyle);
                }
                //prefix '///'
                foreach (Range rr in r.GetRanges(@"^\s*///", RegexOptions.Multiline))
                {
                    rr.ClearStyle(StyleIndex.All);
                    rr.SetStyle(CommentTagStyle);
                }
            }

            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers("{", "}"); //allow to collapse brackets block
            range.SetFoldingMarkers(@"#region\b", @"#endregion\b"); //allow to collapse #region blocks
            range.SetFoldingMarkers(@"/\*", @"\*/"); //allow to collapse comment block
        }

        protected void InitVBRegex()
        {
            VBStringRegex = new Regex(@"""""|"".*?[^\\]""", RegexCompiledOption);
            VBCommentRegex = new Regex(@"'.*$", RegexOptions.Multiline | RegexCompiledOption);
            VBNumberRegex = new Regex(@"\b\d+[\.]?\d*([eE]\-?\d+)?\b", RegexCompiledOption);
            VBClassNameRegex = new Regex(@"\b(Class|Structure|Enum|Interface)[ ]+(?<range>\w+?)\b",
                                         RegexOptions.IgnoreCase | RegexCompiledOption);
            VBKeywordRegex =
                new Regex(
                    @"\b(AddHandler|AddressOf|Alias|And|AndAlso|As|Boolean|ByRef|Byte|ByVal|Call|Case|Catch|CBool|CByte|CChar|CDate|CDbl|CDec|Char|CInt|Class|CLng|CObj|Const|Continue|CSByte|CShort|CSng|CStr|CType|CUInt|CULng|CUShort|Date|Decimal|Declare|Default|Delegate|Dim|DirectCast|Do|Double|Each|Else|ElseIf|End|EndIf|Enum|Erase|Error|Event|Exit|False|Finally|For|Friend|Function|Get|GetType|GetXMLNamespace|Global|GoSub|GoTo|Handles|If|Implements|Imports|In|Inherits|Integer|Interface|Is|IsNot|Let|Lib|Like|Long|Loop|Me|Mod|Module|MustInherit|MustOverride|MyBase|MyClass|Namespace|Narrowing|New|Next|Not|Nothing|NotInheritable|NotOverridable|Object|Of|On|Operator|Option|Optional|Or|OrElse|Overloads|Overridable|Overrides|ParamArray|Partial|Private|Property|Protected|Public|RaiseEvent|ReadOnly|ReDim|REM|RemoveHandler|Resume|Return|SByte|Select|Set|Shadows|Shared|Short|Single|Static|Step|Stop|String|Structure|Sub|SyncLock|Then|Throw|To|True|Try|TryCast|TypeOf|UInteger|ULong|UShort|Using|Variant|Wend|When|While|Widening|With|WithEvents|WriteOnly|Xor|Region)\b|(#Const|#Else|#ElseIf|#End|#If|#Region)\b",
                    RegexOptions.IgnoreCase | RegexCompiledOption);
        }

        /// <summary>
        /// Highlights VB code
        /// </summary>
        /// <param name="range"></param>
        public virtual void VBSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = "'";
            range.tb.LeftBracket = '(';
            range.tb.RightBracket = ')';
            range.tb.LeftBracket2 = '\x0';
            range.tb.RightBracket2 = '\x0';

            range.tb.AutoIndentCharsPatterns
                = @"
^\s*[\w\.\(\)]+\s*(?<range>=)\s*(?<range>.+)
";
            //clear style of changed range
            range.ClearStyle(StringStyle, CommentStyle, NumberStyle, ClassNameStyle, KeywordStyle);
            //
            if (VBStringRegex == null)
                InitVBRegex();
            //string highlighting
            range.SetStyle(StringStyle, VBStringRegex);
            //comment highlighting
            range.SetStyle(CommentStyle, VBCommentRegex);
            //number highlighting
            range.SetStyle(NumberStyle, VBNumberRegex);
            //class name highlighting
            range.SetStyle(ClassNameStyle, VBClassNameRegex);
            //keyword highlighting
            range.SetStyle(KeywordStyle, VBKeywordRegex);

            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers(@"#Region\b", @"#End\s+Region\b", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers(@"\b(Class|Property|Enum|Structure|Interface)[ \t]+\S+",
                                    @"\bEnd (Class|Property|Enum|Structure|Interface)\b", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers(@"^\s*(?<range>While)[ \t]+\S+", @"^\s*(?<range>End While)\b",
                                    RegexOptions.Multiline | RegexOptions.IgnoreCase);
            range.SetFoldingMarkers(@"\b(Sub|Function)[ \t]+[^\s']+", @"\bEnd (Sub|Function)\b", RegexOptions.IgnoreCase);
            //this declared separately because Sub and Function can be unclosed
            range.SetFoldingMarkers(@"(\r|\n|^)[ \t]*(?<range>Get|Set)[ \t]*(\r|\n|$)", @"\bEnd (Get|Set)\b",
                                    RegexOptions.IgnoreCase);
            range.SetFoldingMarkers(@"^\s*(?<range>For|For\s+Each)\b", @"^\s*(?<range>Next)\b",
                                    RegexOptions.Multiline | RegexOptions.IgnoreCase);
            range.SetFoldingMarkers(@"^\s*(?<range>Do)\b", @"^\s*(?<range>Loop)\b",
                                    RegexOptions.Multiline | RegexOptions.IgnoreCase);
        }

        protected void InitHTMLRegex()
        {
            HTMLCommentRegex1 = new Regex(@"(<!--.*?-->)|(<!--.*)", RegexOptions.Singleline | RegexCompiledOption);
            HTMLCommentRegex2 = new Regex(@"(<!--.*?-->)|(.*-->)",
                                          RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            HTMLTagRegex = new Regex(@"<|/>|</|>", RegexCompiledOption);
            HTMLTagNameRegex = new Regex(@"<(?<range>[!\w:]+)", RegexCompiledOption);
            HTMLEndTagRegex = new Regex(@"</(?<range>[\w:]+)>", RegexCompiledOption);
            HTMLTagContentRegex = new Regex(@"<[^>]+>", RegexCompiledOption);
            HTMLAttrRegex =
                new Regex(
                    @"(?<range>[\w\d\-]{1,20}?)='[^']*'|(?<range>[\w\d\-]{1,20})=""[^""]*""|(?<range>[\w\d\-]{1,20})=[\w\d\-]{1,20}",
                    RegexCompiledOption);
            HTMLAttrValRegex =
                new Regex(
                    @"[\w\d\-]{1,20}?=(?<range>'[^']*')|[\w\d\-]{1,20}=(?<range>""[^""]*"")|[\w\d\-]{1,20}=(?<range>[\w\d\-]{1,20})",
                    RegexCompiledOption);
            HTMLEntityRegex = new Regex(@"\&(amp|gt|lt|nbsp|quot|apos|copy|reg|#[0-9]{1,8}|#x[0-9a-f]{1,8});",
                                        RegexCompiledOption | RegexOptions.IgnoreCase);
        }

        /// <summary>
        /// Highlights HTML code
        /// </summary>
        /// <param name="range"></param>
        public virtual void HTMLSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = null;
            range.tb.LeftBracket = '<';
            range.tb.RightBracket = '>';
            range.tb.LeftBracket2 = '(';
            range.tb.RightBracket2 = ')';
            range.tb.AutoIndentCharsPatterns = @"";
            //clear style of changed range
            range.ClearStyle(CommentStyle, TagBracketStyle, TagNameStyle, AttributeStyle, AttributeValueStyle,
                             HtmlEntityStyle);
            //
            if (HTMLTagRegex == null)
                InitHTMLRegex();
            //comment highlighting
            range.SetStyle(CommentStyle, HTMLCommentRegex1);
            range.SetStyle(CommentStyle, HTMLCommentRegex2);
            //tag brackets highlighting
            range.SetStyle(TagBracketStyle, HTMLTagRegex);
            //tag name
            range.SetStyle(TagNameStyle, HTMLTagNameRegex);
            //end of tag
            range.SetStyle(TagNameStyle, HTMLEndTagRegex);
            //attributes
            range.SetStyle(AttributeStyle, HTMLAttrRegex);
            //attribute values
            range.SetStyle(AttributeValueStyle, HTMLAttrValRegex);
            //html entity
            range.SetStyle(HtmlEntityStyle, HTMLEntityRegex);

            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers("<head", "</head>", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers("<body", "</body>", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers("<table", "</table>", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers("<form", "</form>", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers("<div", "</div>", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers("<script", "</script>", RegexOptions.IgnoreCase);
            range.SetFoldingMarkers("<tr", "</tr>", RegexOptions.IgnoreCase);
        }

        protected void InitXMLRegex()
        {
            XMLCommentRegex1 = new Regex(@"(<!--.*?-->)|(<!--.*)", RegexOptions.Singleline | RegexCompiledOption);
            XMLCommentRegex2 = new Regex(@"(<!--.*?-->)|(.*-->)",
                                          RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            XMLTagRegex = new Regex(@"<\?|<|/>|</|>|\?>", RegexCompiledOption);
            XMLTagNameRegex = new Regex(@"<[?](?<range1>[x][m][l]{1})|<(?<range>[!\w:]+)", RegexCompiledOption);
            XMLEndTagRegex = new Regex(@"</(?<range>[\w:]+)>", RegexCompiledOption);
            XMLTagContentRegex = new Regex(@"<[^>]+>", RegexCompiledOption);
            XMLAttrRegex =
                new Regex(
                    @"(?<range>[\w\d\-\:]+)[ ]*=[ ]*'[^']*'|(?<range>[\w\d\-\:]+)[ ]*=[ ]*""[^""]*""|(?<range>[\w\d\-\:]+)[ ]*=[ ]*[\w\d\-\:]+",
                    RegexCompiledOption);
            XMLAttrValRegex =
                new Regex(
                    @"[\w\d\-]+?=(?<range>'[^']*')|[\w\d\-]+[ ]*=[ ]*(?<range>""[^""]*"")|[\w\d\-]+[ ]*=[ ]*(?<range>[\w\d\-]+)",
                    RegexCompiledOption);
            XMLEntityRegex = new Regex(@"\&(amp|gt|lt|nbsp|quot|apos|copy|reg|#[0-9]{1,8}|#x[0-9a-f]{1,8});",
                                        RegexCompiledOption | RegexOptions.IgnoreCase);
            XMLCDataRegex = new Regex(@"<!\s*\[CDATA\s*\[(?<text>(?>[^]]+|](?!]>))*)]]>", RegexCompiledOption | RegexOptions.IgnoreCase); // http://stackoverflow.com/questions/21681861/i-need-a-regex-that-matches-cdata-elements-in-html
            XMLFoldingRegex = new Regex(@"<(?<range>/?\w+)\s[^>]*?[^/]>|<(?<range>/?\w+)\s*>", RegexOptions.Singleline | RegexCompiledOption);
        }

        /// <summary>
        /// Highlights XML code
        /// </summary>
        /// <param name="range"></param>
        public virtual void XMLSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = null;
            range.tb.LeftBracket = '<';
            range.tb.RightBracket = '>';
            range.tb.LeftBracket2 = '(';
            range.tb.RightBracket2 = ')';
            range.tb.AutoIndentCharsPatterns = @"";
            //clear style of changed range
            range.ClearStyle(CommentStyle, XmlTagBracketStyle, XmlTagNameStyle, XmlAttributeStyle, XmlAttributeValueStyle,
                             XmlEntityStyle, XmlCDataStyle);

            //
            if (XMLTagRegex == null)
            {
                InitXMLRegex();
            }

            //xml CData
            range.SetStyle(XmlCDataStyle, XMLCDataRegex);

            //comment highlighting
            range.SetStyle(CommentStyle, XMLCommentRegex1);
            range.SetStyle(CommentStyle, XMLCommentRegex2);

            //tag brackets highlighting
            range.SetStyle(XmlTagBracketStyle, XMLTagRegex);

            //tag name
            range.SetStyle(XmlTagNameStyle, XMLTagNameRegex);

            //end of tag
            range.SetStyle(XmlTagNameStyle, XMLEndTagRegex);

            //attributes
            range.SetStyle(XmlAttributeStyle, XMLAttrRegex);

            //attribute values
            range.SetStyle(XmlAttributeValueStyle, XMLAttrValRegex);

            //xml entity
            range.SetStyle(XmlEntityStyle, XMLEntityRegex);

            //clear folding markers
            range.ClearFoldingMarkers();

            //set folding markers
            XmlFolding(range);
        }

        private void XmlFolding(Range range)
        {
            var stack = new Stack<XmlFoldingTag>();
            var id = 0;
            var fctb = range.tb;
            //extract opening and closing tags (exclude open-close tags: <TAG/>)
            foreach (var r in range.GetRanges(XMLFoldingRegex))
            {
                var tagName = r.Text;
                var iLine = r.Start.iLine;
                //if it is opening tag...
                if (tagName[0] != '/')
                {
                    // ...push into stack
                    var tag = new XmlFoldingTag {Name = tagName, id = id++, startLine = r.Start.iLine};
                    stack.Push(tag);
                    // if this line has no markers - set marker
                    if (string.IsNullOrEmpty(fctb[iLine].FoldingStartMarker))
                        fctb[iLine].FoldingStartMarker = tag.Marker;
                }
                else
                {
                    //if it is closing tag - pop from stack
                    if (stack.Count > 0)
                    {
                        var tag = stack.Pop();
                        //compare line number
                        if (iLine == tag.startLine)
                        {
                            //remove marker, because same line can not be folding
                            if (fctb[iLine].FoldingStartMarker == tag.Marker) //was it our marker?
                                fctb[iLine].FoldingStartMarker = null;
                        }
                        else
                        {
                            //set end folding marker
                            if (string.IsNullOrEmpty(fctb[iLine].FoldingEndMarker))
                                fctb[iLine].FoldingEndMarker = tag.Marker;
                        }
                    }
                }
            }
        }

        class XmlFoldingTag
		{
			public string Name;
			public int id;
			public int startLine;
			public string Marker { get { return Name + id; } }
		}

        protected void InitSQLRegex()
        {
            SQLStringRegex = new Regex(@"""""|''|"".*?[^\\]""|'.*?[^\\]'", RegexCompiledOption);
            SQLNumberRegex = new Regex(@"\b\d+[\.]?\d*([eE]\-?\d+)?\b", RegexCompiledOption);
            SQLCommentRegex1 = new Regex(@"--.*$", RegexOptions.Multiline | RegexCompiledOption);
            SQLCommentRegex2 = new Regex(@"(/\*.*?\*/)|(/\*.*)", RegexOptions.Singleline | RegexCompiledOption);
            SQLCommentRegex3 = new Regex(@"(/\*.*?\*/)|(.*\*/)",
                                         RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            SQLVarRegex = new Regex(@"@[a-zA-Z_\d]*\b", RegexCompiledOption);
            SQLStatementsRegex = new Regex(@"\b(ALTER APPLICATION ROLE|ALTER ASSEMBLY|ALTER ASYMMETRIC KEY|ALTER AUTHORIZATION|ALTER BROKER PRIORITY|ALTER CERTIFICATE|ALTER CREDENTIAL|ALTER CRYPTOGRAPHIC PROVIDER|ALTER DATABASE|ALTER DATABASE AUDIT SPECIFICATION|ALTER DATABASE ENCRYPTION KEY|ALTER ENDPOINT|ALTER EVENT SESSION|ALTER FULLTEXT CATALOG|ALTER FULLTEXT INDEX|ALTER FULLTEXT STOPLIST|ALTER FUNCTION|ALTER INDEX|ALTER LOGIN|ALTER MASTER KEY|ALTER MESSAGE TYPE|ALTER PARTITION FUNCTION|ALTER PARTITION SCHEME|ALTER PROCEDURE|ALTER QUEUE|ALTER REMOTE SERVICE BINDING|ALTER RESOURCE GOVERNOR|ALTER RESOURCE POOL|ALTER ROLE|ALTER ROUTE|ALTER SCHEMA|ALTER SERVER AUDIT|ALTER SERVER AUDIT SPECIFICATION|ALTER SERVICE|ALTER SERVICE MASTER KEY|ALTER SYMMETRIC KEY|ALTER TABLE|ALTER TRIGGER|ALTER USER|ALTER VIEW|ALTER WORKLOAD GROUP|ALTER XML SCHEMA COLLECTION|BULK INSERT|CREATE AGGREGATE|CREATE APPLICATION ROLE|CREATE ASSEMBLY|CREATE ASYMMETRIC KEY|CREATE BROKER PRIORITY|CREATE CERTIFICATE|CREATE CONTRACT|CREATE CREDENTIAL|CREATE CRYPTOGRAPHIC PROVIDER|CREATE DATABASE|CREATE DATABASE AUDIT SPECIFICATION|CREATE DATABASE ENCRYPTION KEY|CREATE DEFAULT|CREATE ENDPOINT|CREATE EVENT NOTIFICATION|CREATE EVENT SESSION|CREATE FULLTEXT CATALOG|CREATE FULLTEXT INDEX|CREATE FULLTEXT STOPLIST|CREATE FUNCTION|CREATE INDEX|CREATE LOGIN|CREATE MASTER KEY|CREATE MESSAGE TYPE|CREATE PARTITION FUNCTION|CREATE PARTITION SCHEME|CREATE PROCEDURE|CREATE QUEUE|CREATE REMOTE SERVICE BINDING|CREATE RESOURCE POOL|CREATE ROLE|CREATE ROUTE|CREATE RULE|CREATE SCHEMA|CREATE SERVER AUDIT|CREATE SERVER AUDIT SPECIFICATION|CREATE SERVICE|CREATE SPATIAL INDEX|CREATE STATISTICS|CREATE SYMMETRIC KEY|CREATE SYNONYM|CREATE TABLE|CREATE TRIGGER|CREATE TYPE|CREATE USER|CREATE VIEW|CREATE WORKLOAD GROUP|CREATE XML INDEX|CREATE XML SCHEMA COLLECTION|DELETE|DISABLE TRIGGER|DROP AGGREGATE|DROP APPLICATION ROLE|DROP ASSEMBLY|DROP ASYMMETRIC KEY|DROP BROKER PRIORITY|DROP CERTIFICATE|DROP CONTRACT|DROP CREDENTIAL|DROP CRYPTOGRAPHIC PROVIDER|DROP DATABASE|DROP DATABASE AUDIT SPECIFICATION|DROP DATABASE ENCRYPTION KEY|DROP DEFAULT|DROP ENDPOINT|DROP EVENT NOTIFICATION|DROP EVENT SESSION|DROP FULLTEXT CATALOG|DROP FULLTEXT INDEX|DROP FULLTEXT STOPLIST|DROP FUNCTION|DROP INDEX|DROP LOGIN|DROP MASTER KEY|DROP MESSAGE TYPE|DROP PARTITION FUNCTION|DROP PARTITION SCHEME|DROP PROCEDURE|DROP QUEUE|DROP REMOTE SERVICE BINDING|DROP RESOURCE POOL|DROP ROLE|DROP ROUTE|DROP RULE|DROP SCHEMA|DROP SERVER AUDIT|DROP SERVER AUDIT SPECIFICATION|DROP SERVICE|DROP SIGNATURE|DROP STATISTICS|DROP SYMMETRIC KEY|DROP SYNONYM|DROP TABLE|DROP TRIGGER|DROP TYPE|DROP USER|DROP VIEW|DROP WORKLOAD GROUP|DROP XML SCHEMA COLLECTION|ENABLE TRIGGER|EXEC|EXECUTE|REPLACE|FROM|INSERT|MERGE|OPTION|OUTPUT|SELECT|TOP|TRUNCATE TABLE|UPDATE|UPDATE STATISTICS|WHERE|WITH|INTO|IN|SET)\b", RegexOptions.IgnoreCase | RegexCompiledOption);
            SQLKeywordsRegex = new Regex(@"\b(ADD|ALL|AND|ANY|AS|ASC|AUTHORIZATION|BACKUP|BEGIN|BETWEEN|BREAK|BROWSE|BY|CASCADE|CHECK|CHECKPOINT|CLOSE|CLUSTERED|COLLATE|COLUMN|COMMIT|COMPUTE|CONSTRAINT|CONTAINS|CONTINUE|CROSS|CURRENT|CURRENT_DATE|CURRENT_TIME|CURSOR|DATABASE|DBCC|DEALLOCATE|DECLARE|DEFAULT|DENY|DESC|DISK|DISTINCT|DISTRIBUTED|DOUBLE|DUMP|ELSE|END|ERRLVL|ESCAPE|EXCEPT|EXISTS|EXIT|EXTERNAL|FETCH|FILE|FILLFACTOR|FOR|FOREIGN|FREETEXT|FULL|FUNCTION|GOTO|GRANT|GROUP|HAVING|HOLDLOCK|IDENTITY|IDENTITY_INSERT|IDENTITYCOL|IF|INDEX|INNER|INTERSECT|IS|JOIN|KEY|KILL|LIKE|LINENO|LOAD|NATIONAL|NOCHECK|NONCLUSTERED|NOT|NULL|OF|OFF|OFFSETS|ON|OPEN|OR|ORDER|OUTER|OVER|PERCENT|PIVOT|PLAN|PRECISION|PRIMARY|PRINT|PROC|PROCEDURE|PUBLIC|RAISERROR|READ|READTEXT|RECONFIGURE|REFERENCES|REPLICATION|RESTORE|RESTRICT|RETURN|REVERT|REVOKE|ROLLBACK|ROWCOUNT|ROWGUIDCOL|RULE|SAVE|SCHEMA|SECURITYAUDIT|SHUTDOWN|SOME|STATISTICS|TABLE|TABLESAMPLE|TEXTSIZE|THEN|TO|TRAN|TRANSACTION|TRIGGER|TSEQUAL|UNION|UNIQUE|UNPIVOT|UPDATETEXT|USE|USER|VALUES|VARYING|VIEW|WAITFOR|WHEN|WHILE|WRITETEXT)\b", RegexOptions.IgnoreCase | RegexCompiledOption);
            SQLFunctionsRegex = new Regex(@"(@@CONNECTIONS|@@CPU_BUSY|@@CURSOR_ROWS|@@DATEFIRST|@@DATEFIRST|@@DBTS|@@ERROR|@@FETCH_STATUS|@@IDENTITY|@@IDLE|@@IO_BUSY|@@LANGID|@@LANGUAGE|@@LOCK_TIMEOUT|@@MAX_CONNECTIONS|@@MAX_PRECISION|@@NESTLEVEL|@@OPTIONS|@@PACKET_ERRORS|@@PROCID|@@REMSERVER|@@ROWCOUNT|@@SERVERNAME|@@SERVICENAME|@@SPID|@@TEXTSIZE|@@TRANCOUNT|@@VERSION)\b|\b(ABS|ACOS|APP_NAME|ASCII|ASIN|ASSEMBLYPROPERTY|AsymKey_ID|ASYMKEY_ID|asymkeyproperty|ASYMKEYPROPERTY|ATAN|ATN2|AVG|CASE|CAST|CEILING|Cert_ID|Cert_ID|CertProperty|CHAR|CHARINDEX|CHECKSUM_AGG|COALESCE|COL_LENGTH|COL_NAME|COLLATIONPROPERTY|COLLATIONPROPERTY|COLUMNPROPERTY|COLUMNS_UPDATED|COLUMNS_UPDATED|CONTAINSTABLE|CONVERT|COS|COT|COUNT|COUNT_BIG|CRYPT_GEN_RANDOM|CURRENT_TIMESTAMP|CURRENT_TIMESTAMP|CURRENT_USER|CURRENT_USER|CURSOR_STATUS|DATABASE_PRINCIPAL_ID|DATABASE_PRINCIPAL_ID|DATABASEPROPERTY|DATABASEPROPERTYEX|DATALENGTH|DATALENGTH|DATEADD|DATEDIFF|DATENAME|DATEPART|DAY|DB_ID|DB_NAME|DECRYPTBYASYMKEY|DECRYPTBYCERT|DECRYPTBYKEY|DECRYPTBYKEYAUTOASYMKEY|DECRYPTBYKEYAUTOCERT|DECRYPTBYPASSPHRASE|DEGREES|DENSE_RANK|DIFFERENCE|ENCRYPTBYASYMKEY|ENCRYPTBYCERT|ENCRYPTBYKEY|ENCRYPTBYPASSPHRASE|ERROR_LINE|ERROR_MESSAGE|ERROR_NUMBER|ERROR_PROCEDURE|ERROR_SEVERITY|ERROR_STATE|EVENTDATA|EXP|FILE_ID|FILE_IDEX|FILE_NAME|FILEGROUP_ID|FILEGROUP_NAME|FILEGROUPPROPERTY|FILEPROPERTY|FLOOR|fn_helpcollations|fn_listextendedproperty|fn_servershareddrives|fn_virtualfilestats|fn_virtualfilestats|FORMATMESSAGE|FREETEXTTABLE|FULLTEXTCATALOGPROPERTY|FULLTEXTSERVICEPROPERTY|GETANSINULL|GETDATE|GETUTCDATE|GROUPING|HAS_PERMS_BY_NAME|HOST_ID|HOST_NAME|IDENT_CURRENT|IDENT_CURRENT|IDENT_INCR|IDENT_INCR|IDENT_SEED|IDENTITY\(|INDEX_COL|INDEXKEY_PROPERTY|INDEXPROPERTY|IS_MEMBER|IS_OBJECTSIGNED|IS_SRVROLEMEMBER|ISDATE|ISDATE|ISNULL|ISNUMERIC|Key_GUID|Key_GUID|Key_ID|Key_ID|KEY_NAME|KEY_NAME|LEFT|LEN|LOG|LOG10|LOWER|LTRIM|MAX|MIN|MONTH|NCHAR|NEWID|NTILE|NULLIF|OBJECT_DEFINITION|OBJECT_ID|OBJECT_NAME|OBJECT_SCHEMA_NAME|OBJECTPROPERTY|OBJECTPROPERTYEX|OPENDATASOURCE|OPENQUERY|OPENROWSET|OPENXML|ORIGINAL_LOGIN|ORIGINAL_LOGIN|PARSENAME|PATINDEX|PATINDEX|PERMISSIONS|PI|POWER|PUBLISHINGSERVERNAME|PWDCOMPARE|PWDENCRYPT|QUOTENAME|RADIANS|RAND|RANK|REPLICATE|REVERSE|RIGHT|ROUND|ROW_NUMBER|ROWCOUNT_BIG|RTRIM|SCHEMA_ID|SCHEMA_ID|SCHEMA_NAME|SCHEMA_NAME|SCOPE_IDENTITY|SERVERPROPERTY|SESSION_USER|SESSION_USER|SESSIONPROPERTY|SETUSER|SIGN|SignByAsymKey|SignByCert|SIN|SOUNDEX|SPACE|SQL_VARIANT_PROPERTY|SQRT|SQUARE|STATS_DATE|STDEV|STDEVP|STR|STUFF|SUBSTRING|SUM|SUSER_ID|SUSER_NAME|SUSER_SID|SUSER_SNAME|SWITCHOFFSET|SYMKEYPROPERTY|symkeyproperty|sys\.dm_db_index_physical_stats|sys\.fn_builtin_permissions|sys\.fn_my_permissions|SYSDATETIME|SYSDATETIMEOFFSET|SYSTEM_USER|SYSTEM_USER|SYSUTCDATETIME|TAN|TERTIARY_WEIGHTS|TEXTPTR|TODATETIMEOFFSET|TRIGGER_NESTLEVEL|TYPE_ID|TYPE_NAME|TYPEPROPERTY|UNICODE|UPDATE\(|UPPER|USER_ID|USER_NAME|USER_NAME|VAR|VARP|VerifySignedByAsymKey|VerifySignedByCert|XACT_STATE|YEAR)\b", RegexOptions.IgnoreCase | RegexCompiledOption);
            SQLTypesRegex =
                new Regex(
                    @"\b(BIGINT|NUMERIC|BIT|SMALLINT|DECIMAL|SMALLMONEY|INT|TINYINT|MONEY|FLOAT|REAL|DATE|DATETIMEOFFSET|DATETIME2|SMALLDATETIME|DATETIME|TIME|CHAR|VARCHAR|TEXT|NCHAR|NVARCHAR|NTEXT|BINARY|VARBINARY|IMAGE|TIMESTAMP|HIERARCHYID|TABLE|UNIQUEIDENTIFIER|SQL_VARIANT|XML)\b",
                    RegexOptions.IgnoreCase | RegexCompiledOption);
        }

        /// <summary>
        /// Highlights SQL code
        /// </summary>
        /// <param name="range"></param>
        public virtual void SQLSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = "--";
            range.tb.LeftBracket = '(';
            range.tb.RightBracket = ')';
            range.tb.LeftBracket2 = '\x0';
            range.tb.RightBracket2 = '\x0';

            range.tb.AutoIndentCharsPatterns = @"";
            //clear style of changed range
            range.ClearStyle(CommentStyle, StringStyle, NumberStyle, VariableStyle, StatementsStyle, KeywordStyle,
                             FunctionsStyle, TypesStyle);
            //
            if (SQLStringRegex == null)
                InitSQLRegex();
            //comment highlighting
            range.SetStyle(CommentStyle, SQLCommentRegex1);
            range.SetStyle(CommentStyle, SQLCommentRegex2);
            range.SetStyle(CommentStyle, SQLCommentRegex3);
            //string highlighting
            range.SetStyle(StringStyle, SQLStringRegex);
            //number highlighting
            range.SetStyle(NumberStyle, SQLNumberRegex);
            //types highlighting
            range.SetStyle(TypesStyle, SQLTypesRegex);
            //var highlighting
            range.SetStyle(VariableStyle, SQLVarRegex);
            //statements
            range.SetStyle(StatementsStyle, SQLStatementsRegex);
            //keywords
            range.SetStyle(KeywordStyle, SQLKeywordsRegex);
            //functions
            range.SetStyle(FunctionsStyle, SQLFunctionsRegex);

            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers(@"\bBEGIN\b", @"\bEND\b", RegexOptions.IgnoreCase);
            //allow to collapse BEGIN..END blocks
            range.SetFoldingMarkers(@"/\*", @"\*/"); //allow to collapse comment block
        }

        protected void InitPHPRegex()
        {
            PHPStringRegex = new Regex(@"""""|''|"".*?[^\\]""|'.*?[^\\]'", RegexCompiledOption);
            PHPNumberRegex = new Regex(@"\b\d+[\.]?\d*\b", RegexCompiledOption);
            PHPCommentRegex1 = new Regex(@"(//|#).*$", RegexOptions.Multiline | RegexCompiledOption);
            PHPCommentRegex2 = new Regex(@"(/\*.*?\*/)|(/\*.*)", RegexOptions.Singleline | RegexCompiledOption);
            PHPCommentRegex3 = new Regex(@"(/\*.*?\*/)|(.*\*/)",
                                         RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            PHPVarRegex = new Regex(@"\$[a-zA-Z_\d]*\b", RegexCompiledOption);
            PHPKeywordRegex1 =
                new Regex(
                    @"\b(die|echo|empty|exit|eval|include|include_once|isset|list|require|require_once|return|print|unset)\b",
                    RegexCompiledOption);
            PHPKeywordRegex2 =
                new Regex(
                    @"\b(abstract|and|array|as|break|case|catch|cfunction|class|clone|const|continue|declare|default|do|else|elseif|enddeclare|endfor|endforeach|endif|endswitch|endwhile|extends|final|for|foreach|function|global|goto|if|implements|instanceof|interface|namespace|new|or|private|protected|public|static|switch|throw|try|use|var|while|xor)\b",
                    RegexCompiledOption);
            PHPKeywordRegex3 = new Regex(@"__CLASS__|__DIR__|__FILE__|__LINE__|__FUNCTION__|__METHOD__|__NAMESPACE__",
                                         RegexCompiledOption);
        }

        /// <summary>
        /// Highlights PHP code
        /// </summary>
        /// <param name="range"></param>
        public virtual void PHPSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = "//";
            range.tb.LeftBracket = '(';
            range.tb.RightBracket = ')';
            range.tb.LeftBracket2 = '{';
            range.tb.RightBracket2 = '}';
            range.tb.BracketsHighlightStrategy = BracketsHighlightStrategy.Strategy2;
            //clear style of changed range
            range.ClearStyle(StringStyle, CommentStyle, NumberStyle, VariableStyle, KeywordStyle, KeywordStyle2,
                             KeywordStyle3);

            range.tb.AutoIndentCharsPatterns
                = @"
^\s*\$[\w\.\[\]\'\""]+\s*(?<range>=)\s*(?<range>[^;]+);
";

            //
            if (PHPStringRegex == null)
                InitPHPRegex();
            //string highlighting
            range.SetStyle(StringStyle, PHPStringRegex);
            //comment highlighting
            range.SetStyle(CommentStyle, PHPCommentRegex1);
            range.SetStyle(CommentStyle, PHPCommentRegex2);
            range.SetStyle(CommentStyle, PHPCommentRegex3);
            //number highlighting
            range.SetStyle(NumberStyle, PHPNumberRegex);
            //var highlighting
            range.SetStyle(VariableStyle, PHPVarRegex);
            //keyword highlighting
            range.SetStyle(KeywordStyle, PHPKeywordRegex1);
            range.SetStyle(KeywordStyle2, PHPKeywordRegex2);
            range.SetStyle(KeywordStyle3, PHPKeywordRegex3);

            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers("{", "}"); //allow to collapse brackets block
            range.SetFoldingMarkers(@"/\*", @"\*/"); //allow to collapse comment block
        }

        protected void InitJScriptRegex()
        {
            JScriptStringRegex = new Regex(@"""""|''|"".*?[^\\]""|'.*?[^\\]'", RegexCompiledOption);
            JScriptCommentRegex1 = new Regex(@"//.*$", RegexOptions.Multiline | RegexCompiledOption);
            JScriptCommentRegex2 = new Regex(@"(/\*.*?\*/)|(/\*.*)", RegexOptions.Singleline | RegexCompiledOption);
            JScriptCommentRegex3 = new Regex(@"(/\*.*?\*/)|(.*\*/)",
                                             RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            JScriptNumberRegex = new Regex(@"\b\d+[\.]?\d*([eE]\-?\d+)?[lLdDfF]?\b|\b0x[a-fA-F\d]+\b",
                                           RegexCompiledOption);
            JScriptKeywordRegex =
                new Regex(
                    @"\b(true|false|break|case|catch|const|continue|default|delete|do|else|export|for|function|if|in|instanceof|new|null|return|switch|this|throw|try|var|void|while|with|typeof)\b",
                    RegexCompiledOption);
        }

        /// <summary>
        /// Highlights JavaScript code
        /// </summary>
        /// <param name="range"></param>
        public virtual void JScriptSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = "//";
            range.tb.LeftBracket = '(';
            range.tb.RightBracket = ')';
            range.tb.LeftBracket2 = '{';
            range.tb.RightBracket2 = '}';
            range.tb.BracketsHighlightStrategy = BracketsHighlightStrategy.Strategy2;

            range.tb.AutoIndentCharsPatterns
                = @"
^\s*[\w\.]+(\s\w+)?\s*(?<range>=)\s*(?<range>[^;]+);
";

            //clear style of changed range
            range.ClearStyle(StringStyle, CommentStyle, NumberStyle, KeywordStyle);
            //
            if (JScriptStringRegex == null)
                InitJScriptRegex();
            //string highlighting
            range.SetStyle(StringStyle, JScriptStringRegex);
            //comment highlighting
            range.SetStyle(CommentStyle, JScriptCommentRegex1);
            range.SetStyle(CommentStyle, JScriptCommentRegex2);
            range.SetStyle(CommentStyle, JScriptCommentRegex3);
            //number highlighting
            range.SetStyle(NumberStyle, JScriptNumberRegex);
            //keyword highlighting
            range.SetStyle(KeywordStyle, JScriptKeywordRegex);
            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers("{", "}"); //allow to collapse brackets block
            range.SetFoldingMarkers(@"/\*", @"\*/"); //allow to collapse comment block
        }

        protected void InitLuaRegex()
        {
            LuaStringRegex = new Regex(@"""""|''|"".*?[^\\]""|'.*?[^\\]'", RegexCompiledOption);
            LuaCommentRegex1 = new Regex(@"--.*$", RegexOptions.Multiline | RegexCompiledOption);
            LuaCommentRegex2 = new Regex(@"(--\[\[.*?\]\])|(--\[\[.*)", RegexOptions.Singleline | RegexCompiledOption);
            LuaCommentRegex3 = new Regex(@"(--\[\[.*?\]\])|(.*\]\])",
                                             RegexOptions.Singleline | RegexOptions.RightToLeft | RegexCompiledOption);
            LuaNumberRegex = new Regex(@"\b\d+[\.]?\d*([eE]\-?\d+)?[lLdDfF]?\b|\b0x[a-fA-F\d]+\b",
                                           RegexCompiledOption);
            LuaKeywordRegex =
                new Regex(
                    @"\b(and|break|do|else|elseif|end|false|for|function|if|in|local|nil|not|or|repeat|return|then|true|until|while)\b",
                    RegexCompiledOption);

            LuaFunctionsRegex =
                new Regex(
                    @"\b(assert|collectgarbage|dofile|error|getfenv|getmetatable|ipairs|load|loadfile|loadstring|module|next|pairs|pcall|print|rawequal|rawget|rawset|require|select|setfenv|setmetatable|tonumber|tostring|type|unpack|xpcall)\b",
                    RegexCompiledOption);
        }

        /// <summary>
        /// Highlights Lua code
        /// </summary>
        /// <param name="range"></param>
        public virtual void LuaSyntaxHighlight(Range range)
        {
            range.tb.CommentPrefix = "--";
            range.tb.LeftBracket = '(';
            range.tb.RightBracket = ')';
            range.tb.LeftBracket2 = '{';
            range.tb.RightBracket2 = '}';
            range.tb.BracketsHighlightStrategy = BracketsHighlightStrategy.Strategy2;

            range.tb.AutoIndentCharsPatterns
                = @"
^\s*[\w\.]+(\s\w+)?\s*(?<range>=)\s*(?<range>.+)
";

            //clear style of changed range
            range.ClearStyle(StringStyle, CommentStyle, NumberStyle, KeywordStyle, FunctionsStyle);
            //
            if (LuaStringRegex == null)
                InitLuaRegex();
            //string highlighting
            range.SetStyle(StringStyle, LuaStringRegex);
            //comment highlighting
            range.SetStyle(CommentStyle, LuaCommentRegex1);
            range.SetStyle(CommentStyle, LuaCommentRegex2);
            range.SetStyle(CommentStyle, LuaCommentRegex3);
            //number highlighting
            range.SetStyle(NumberStyle, LuaNumberRegex);
            //keyword highlighting
            range.SetStyle(KeywordStyle, LuaKeywordRegex);
            //functions highlighting
            range.SetStyle(FunctionsStyle, LuaFunctionsRegex);
            //clear folding markers
            range.ClearFoldingMarkers();
            //set folding markers
            range.SetFoldingMarkers("{", "}"); //allow to collapse brackets block
            range.SetFoldingMarkers(@"--\[\[", @"\]\]"); //allow to collapse comment block
        }

        protected void LuaAutoIndentNeeded(object sender, AutoIndentEventArgs args)
        {
            //end of block
            if (Regex.IsMatch(args.LineText, @"^\s*(end|until)\b"))
            {
                args.Shift = -args.TabLength;
                args.ShiftNextLines = -args.TabLength;
                return;
            }
            // then ...
            if (Regex.IsMatch(args.LineText, @"\b(then)\s*\S+"))
                return;
            //start of operator block
            if (Regex.IsMatch(args.LineText, @"^\s*(function|do|for|while|repeat|if)\b"))
            {
                args.ShiftNextLines = args.TabLength;
                return;
            }

            //Statements else, elseif, case etc
            if (Regex.IsMatch(args.LineText, @"^\s*(else|elseif)\b", RegexOptions.IgnoreCase))
            {
                args.Shift = -args.TabLength;
                return;
            }
        }

        #region Styles

        /// <summary>
        /// String style
        /// </summary>
        public Style StringStyle { get; set; }

        /// <summary>
        /// Comment style
        /// </summary>
        public Style CommentStyle { get; set; }

        /// <summary>
        /// Number style
        /// </summary>
        public Style NumberStyle { get; set; }

        /// <summary>
        /// C# attribute style
        /// </summary>
        public Style AttributeStyle { get; set; }

        /// <summary>
        /// Class name style
        /// </summary>
        public Style ClassNameStyle { get; set; }

        /// <summary>
        /// Keyword style
        /// </summary>
        public Style KeywordStyle { get; set; }

        /// <summary>
        /// Style of tags in comments of C#
        /// </summary>
        public Style CommentTagStyle { get; set; }

        /// <summary>
        /// HTML attribute value style
        /// </summary>
        public Style AttributeValueStyle { get; set; }

        /// <summary>
        /// HTML tag brackets style
        /// </summary>
        public Style TagBracketStyle { get; set; }

        /// <summary>
        /// HTML tag name style
        /// </summary>
        public Style TagNameStyle { get; set; }

        /// <summary>
        /// HTML Entity style
        /// </summary>
        public Style HtmlEntityStyle { get; set; }

        /// <summary>
        /// XML attribute style
        /// </summary>
        public Style XmlAttributeStyle { get; set; }

        /// <summary>
        /// XML attribute value style
        /// </summary>
        public Style XmlAttributeValueStyle { get; set; }

        /// <summary>
        /// XML tag brackets style
        /// </summary>
        public Style XmlTagBracketStyle { get; set; }

        /// <summary>
        /// XML tag name style
        /// </summary>
        public Style XmlTagNameStyle { get; set; }

        /// <summary>
        /// XML Entity style
        /// </summary>
        public Style XmlEntityStyle { get; set; }

        /// <summary>
        /// XML CData style
        /// </summary>
        public Style XmlCDataStyle { get; set; }

        /// <summary>
        /// Variable style
        /// </summary>
        public Style VariableStyle { get; set; }

        /// <summary>
        /// Specific PHP keyword style
        /// </summary>
        public Style KeywordStyle2 { get; set; }

        /// <summary>
        /// Specific PHP keyword style
        /// </summary>
        public Style KeywordStyle3 { get; set; }

        /// <summary>
        /// SQL Statements style
        /// </summary>
        public Style StatementsStyle { get; set; }

        /// <summary>
        /// SQL Functions style
        /// </summary>
        public Style FunctionsStyle { get; set; }

        /// <summary>
        /// SQL Types style
        /// </summary>
        public Style TypesStyle { get; set; }

        #endregion
    }

    /// <summary>
    /// Language
    /// </summary>
    public enum Language
    {
        Custom,
        CSharp,
        VB,
        HTML,
        XML,
        SQL,
        PHP,
        JS,
        Lua
    }
}