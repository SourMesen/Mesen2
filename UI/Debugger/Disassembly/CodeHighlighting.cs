using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.Config;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Debugger.Disassembly
{
	class CodeHighlighting
	{
		private static Regex _labelDef = new Regex("^([^\\s]+)\\s*[:=]", RegexOptions.IgnoreCase | RegexOptions.Compiled);

		private static Regex _space = new Regex("^[ \t]+", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _comment = new Regex("^;.*", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _directive = new Regex("^([.][a-z0-9]+)([\\s]+|$)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		
		//(.[a-z]) is allowed after the ocode to support assemblers (for source view) that use
		//e.g .b/.w/.l suffixes to select between 8/16bit operations, etc.
		private static Regex _opCode = new Regex("^([a-z0-9_]+([.][a-z]){0,1})([\\s]+|$)", RegexOptions.IgnoreCase | RegexOptions.Compiled);

		private static Regex _syntax = new Regex("^[]([)!+,.|:<>-]{1}", RegexOptions.Compiled);
		private static Regex _operand = new Regex("^((([$]|0x)[0-9a-f]*([.]\\d){0,1})|(#([$]|0x|[0-9])[0-9a-f]*)|#|([@_a-z]([@_a-z0-9])*))", RegexOptions.IgnoreCase | RegexOptions.Compiled);

		public static List<CodeColor> GetCpuHighlights(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			string codeString = lineData.Text.TrimEnd();
			Color defaultColor = Color.FromRgb(60, 60, 60);

			List<CodeColor> colors = new List<CodeColor>();
			if(codeString.Length > 0 && highlightCode && !lineData.Flags.HasFlag(LineFlags.Label)) {
				int pos = 0;
				bool foundOpCode = false;
				bool foundDirective = false;
				while(codeString.Length > 0) {
					Match m;
					if((m = _comment.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, Color.FromUInt32(cfg.CodeCommentColor), CodeSegmentType.Comment, pos));
					} else if(colors.Count == 0 && (m = _labelDef.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Groups[1].Value, Color.FromUInt32(cfg.CodeLabelDefinitionColor), CodeSegmentType.LabelDefinition, pos));
					} else if(!foundOpCode && (m = _directive.Match(codeString)).Success) {
						foundDirective = true;
						colors.Add(new CodeColor(m.Groups[1].Value, Color.FromUInt32(cfg.CodeOpcodeColor), CodeSegmentType.Directive, pos));
					} else if(!foundOpCode && (m = _opCode.Match(codeString)).Success) {
						foundOpCode = true;
						colors.Add(new CodeColor(m.Groups[1].Value, textColor ?? Color.FromUInt32(cfg.CodeOpcodeColor), CodeSegmentType.OpCode, pos));
					} else if((foundOpCode || foundDirective) && (m = _operand.Match(codeString)).Success) {
						string operand = m.Value;
						Color operandColor = Colors.Black;
						CodeSegmentType type = CodeSegmentType.None;
						if(operand.Length > 0) {
							switch(operand[0]) {
								case '#':
									operandColor = Color.FromUInt32(cfg.CodeImmediateColor);
									type = CodeSegmentType.ImmediateValue;
									break;
								case '$':
									operandColor = Color.FromUInt32(cfg.CodeAddressColor);
									type = CodeSegmentType.Address;
									break;
								default:
									operandColor = Color.FromUInt32(cfg.CodeLabelDefinitionColor);
									type = CodeSegmentType.Label;
									break;
							}
						}
						colors.Add(new CodeColor(m.Value, textColor ?? operandColor, type, pos));
					} else if((m = _syntax.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, !foundOpCode ? defaultColor : (textColor ?? defaultColor), CodeSegmentType.Syntax, pos));
					} else if((m = _space.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, textColor ?? defaultColor, CodeSegmentType.None, pos));
					}

					if(m.Success) {
						if(m.Groups.Count > 1) {
							pos += m.Groups[1].Value.Length;
							codeString = codeString.Substring(m.Groups[1].Value.Length);
						} else {
							pos += m.Value.Length;
							codeString = codeString.Substring(m.Value.Length);
						}
					} else {
						break;
					}
				}

				//Display the rest of the line (used by trace logger)
				if(codeString.Length > 0) {
					colors.Add(new CodeColor(codeString, textColor ?? defaultColor, CodeSegmentType.None, pos));
				}

				if(lineData.ShowEffectiveAddress && lineData.EffectiveAddress >= 0) {
					string effAddress = lineData.GetEffectiveAddressString(addressFormat, out CodeSegmentType type);
					colors.Add(new CodeColor(" " + effAddress, Color.FromUInt32(cfg.CodeEffectiveAddressColor), type));
				}

				if(showMemoryValues && lineData.ValueSize > 0) {
					colors.Add(new CodeColor(lineData.GetValueString(), defaultColor, CodeSegmentType.MemoryValue));
				}

				if(!string.IsNullOrWhiteSpace(lineData.Comment)) {
					colors.Add(new CodeColor(" " + lineData.Comment, Color.FromUInt32(cfg.CodeCommentColor), CodeSegmentType.Comment));
				}
			} else {
				if(lineData.Flags.HasFlag(LineFlags.Comment)) {
					colors.Add(new CodeColor(lineData.Comment, Color.FromUInt32(cfg.CodeCommentColor), CodeSegmentType.Comment));
				} else if(codeString.EndsWith(":")) {
					colors.Add(new CodeColor(codeString, Color.FromUInt32(cfg.CodeLabelDefinitionColor), CodeSegmentType.LabelDefinition));
				} else {
					colors.Add(new CodeColor(codeString, textColor ?? defaultColor, CodeSegmentType.None));
				}
			}

			return colors;
		}
	}
}
