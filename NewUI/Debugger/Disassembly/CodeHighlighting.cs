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
		private static Regex _space = new Regex("^[ \t]+", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _opCode = new Regex("^[a-z0-9]{2,5}", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _syntax = new Regex("^[]([)!+,.|:<>]{1}", RegexOptions.Compiled);
		private static Regex _operand = new Regex("^(([$][0-9a-f]*([.]\\d){0,1})|(#[@$:_0-9a-z]*)|([@_a-z]([@_a-z0-9])*))", RegexOptions.IgnoreCase | RegexOptions.Compiled);

		public static List<CodeColor> GetCpuHighlights(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			string codeString = lineData.Text;
			Color defaultColor = Color.FromRgb(60, 60, 60);

			if(codeString.Length > 0 && highlightCode && !lineData.Flags.HasFlag(LineFlags.Label)) {
				List<CodeColor> colors = new List<CodeColor>();
				bool foundOpCode = false;
				while(codeString.Length > 0) {
					Match m;
					if(foundOpCode && (m = _operand.Match(codeString)).Success) {
						string operand = m.Value;
						Color operandColor = Colors.Black;
						CodeSegmentType type = CodeSegmentType.None;
						if(operand.Length > 0) {
							switch(operand[0]) {
								case '#':
									operandColor = cfg.CodeImmediateColor;
									type = CodeSegmentType.ImmediateValue;
									break;
								case '$':
									operandColor = cfg.CodeAddressColor;
									type = CodeSegmentType.Address;
									break;
								default:
									operandColor = cfg.CodeLabelDefinitionColor;
									type = CodeSegmentType.Label;
									break;
							}
						}
						colors.Add(new CodeColor(m.Value, textColor ?? operandColor, type));
					} else if(!foundOpCode && (m = _opCode.Match(codeString)).Success) {
						foundOpCode = true;
						colors.Add(new CodeColor(m.Value, textColor ?? cfg.CodeOpcodeColor, CodeSegmentType.OpCode));
					} else if((m = _syntax.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, textColor ?? defaultColor, CodeSegmentType.Syntax));
					} else if((m = _space.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, textColor ?? defaultColor, CodeSegmentType.None));
					}

					if(m.Success) {
						codeString = codeString.Substring(m.Value.Length);
					} else {
						break;
					}
				}

				//Display the rest of the line (used by trace logger)
				colors.Add(new CodeColor(codeString, defaultColor, CodeSegmentType.None));

				if(lineData.EffectiveAddress >= 0) {
					string effAddress = lineData.GetEffectiveAddressString(addressFormat, out CodeSegmentType type);
					colors.Add(new CodeColor(" " + effAddress, cfg.CodeEffectiveAddressColor, type));
				}

				if(showMemoryValues && lineData.ValueSize > 0) {
					colors.Add(new CodeColor(lineData.GetValueString(), defaultColor, CodeSegmentType.MemoryValue));
				}

				return colors;
			} else {
				if(codeString.EndsWith(":")) {
					return new List<CodeColor>() { new CodeColor(codeString, cfg.CodeLabelDefinitionColor, CodeSegmentType.Label) };
				} else {
					return new List<CodeColor>() { new CodeColor(codeString, textColor ?? defaultColor, CodeSegmentType.None) };
				}
			}
		}
	}
}
