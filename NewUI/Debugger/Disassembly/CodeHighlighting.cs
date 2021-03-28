using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.GUI.Config;
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
						Color operandColor = operand.Length > 0 ? (operand[0] == '#' ? (Color)cfg.CodeImmediateColor : (operand[0] == '$' ? (Color)cfg.CodeAddressColor : (Color)cfg.CodeLabelDefinitionColor)) : Colors.Black;
						colors.Add(new CodeColor() { Text = m.Value, Color = textColor.HasValue ? textColor.Value : operandColor });
					} else if(!foundOpCode && (m = _opCode.Match(codeString)).Success) {
						foundOpCode = true;
						colors.Add(new CodeColor() { Text = m.Value, Color = textColor.HasValue ? textColor.Value : (Color)cfg.CodeOpcodeColor });
					} else if((m = _syntax.Match(codeString)).Success) {
						colors.Add(new CodeColor() { Text = m.Value, Color = textColor.HasValue ? textColor.Value : defaultColor });
					} else if((m = _space.Match(codeString)).Success) {
						colors.Add(new CodeColor() { Text = m.Value, Color = textColor.HasValue ? textColor.Value : defaultColor });
					}

					if(m.Success) {
						codeString = codeString.Substring(m.Value.Length);
					} else {
						break;
					}
				}

				//Display the rest of the line (used by trace logger)
				colors.Add(new CodeColor() { Text = codeString, Color = defaultColor });

				if(lineData.EffectiveAddress >= 0) {
					colors.Add(new CodeColor() { Text = " " + lineData.GetEffectiveAddressString(addressFormat), Color = cfg.CodeEffectiveAddressColor });
				}

				if(showMemoryValues && lineData.ValueSize > 0) {
					colors.Add(new CodeColor() { Text = lineData.GetValueString(), Color = defaultColor });
				}

				return colors;
			} else {
				Color color = codeString.EndsWith(":") ? (Color)cfg.CodeLabelDefinitionColor : (textColor ?? defaultColor);
				return new List<CodeColor>() { new CodeColor() { Text = codeString, Color = color } };
			}
		}
	}
}
