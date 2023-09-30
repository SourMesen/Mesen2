using Mesen.Interop;
using Mesen.Debugger.ViewModels;
using System.Collections.Generic;
using Mesen.Debugger.Controls;
using Avalonia.Media;
using System.Text.RegularExpressions;
using Mesen.Config;

namespace Mesen.Debugger.Disassembly
{
	public class SourceViewStyleProvider : BaseStyleProvider
	{
		private SourceViewViewModel _model;

		public SourceViewStyleProvider(CpuType cpuType, SourceViewViewModel model) : base(cpuType)
		{
			_model = model;
		}

		public override bool IsLineActive(CodeLineData line, int lineIndex)
		{
			return _model.ActiveAddress.HasValue && _model.ActiveAddress.Value == line.Address;
		}

		public override bool IsLineFocused(CodeLineData line, int lineIndex)
		{
			lineIndex += _model.ScrollPosition;
			return _model.SelectedRow == lineIndex;
		}

		public override bool IsLineSelected(CodeLineData line, int lineIndex)
		{
			lineIndex += _model.ScrollPosition;
			return lineIndex >= _model.SelectionStart && lineIndex <= _model.SelectionEnd;
		}

		public override List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			if(_model.SelectedFile?.IsAssembly != false) {
				return base.GetCodeColors(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
			} else {
				return GetCHighlights(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
			}
		}

		private static Regex _space = new Regex("^[ \t]+", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _comment = new Regex("^//.*", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _keywords = new Regex("^(if|else|static|void|int|short|long|char|unsigned|signed|break|return|continue|switch|case|const|while|do|#define|#pragma|#include){1}([^a-z0-9_-]+|$)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _text = new Regex("^([a-z0-9_]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);
		private static Regex _syntax = new Regex("^[]([)!+,.|:<>?&^;{}\"'/*%=#-]{1}", RegexOptions.Compiled);
		private static Regex _number = new Regex("^(0x[0-9a-f]+|0b[01]+|[0-9]+)", RegexOptions.IgnoreCase | RegexOptions.Compiled);

		public static List<CodeColor> GetCHighlights(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			DebuggerConfig cfg = ConfigManager.Config.Debug.Debugger;
			string codeString = lineData.Text.TrimEnd();
			Color defaultColor = Color.FromRgb(60, 60, 60);

			List<CodeColor> colors = new List<CodeColor>();
			if(codeString.Length > 0 && highlightCode && !lineData.Flags.HasFlag(LineFlags.Label)) {
				int pos = 0;
				while(codeString.Length > 0) {
					Match m;
					if((m = _comment.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, textColor ?? cfg.CodeCommentColor, CodeSegmentType.Comment, pos));
					} else if((m = _number.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, textColor ?? cfg.CodeImmediateColor, CodeSegmentType.ImmediateValue, pos));
					} else if((m = _keywords.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Groups[1].Value, textColor ?? cfg.CodeAddressColor, CodeSegmentType.OpCode, pos));
					} else if((m = _text.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Groups[1].Value, textColor ?? defaultColor, CodeSegmentType.OpCode, pos));
					} else if((m = _syntax.Match(codeString)).Success) {
						colors.Add(new CodeColor(m.Value, textColor ?? cfg.CodeEffectiveAddressColor, CodeSegmentType.Syntax, pos));
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

				//Display the rest of the line
				if(codeString.Length > 0) {
					colors.Add(new CodeColor(codeString, textColor ?? defaultColor, CodeSegmentType.None, pos));
				}
			} else {
				colors.Add(new CodeColor(codeString, textColor ?? defaultColor, CodeSegmentType.None));
			}

			return colors;
		}
	}
}
