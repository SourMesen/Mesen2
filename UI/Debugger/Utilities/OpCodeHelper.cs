using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.Interop;
using Mesen.Localization;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Mesen.Debugger.Utilities;

public static class OpCodeHelper
{
	private static Dictionary<CpuType, CpuDocumentationData> _data = new();

	static OpCodeHelper()
	{
		InitNesDocumentation();
		InitSnesDocumentation();
		InitPceDocumentation();
		InitGbDocumentation();
		InitSmsDocumentation();
		InitGbaDocumentation();
		InitWsDocumentation();
	}

	public static DynamicTooltip? GetTooltip(CodeSegmentInfo seg)
	{
		if(!_data.TryGetValue(seg.Data.CpuType, out CpuDocumentationData? doc)) {
			return null;
		}
		
		OpCodeDesc? desc = null;
		string opname = seg.Text.ToLower();
		if(doc.OpDescGetter != null) {
			desc = doc.OpDescGetter(doc.OpDesc, opname, seg);
		} else {
			doc.OpDesc.TryGetValue(opname, out desc);
		}

		if(desc == null) {
			return null;
		}

		if(seg.Data.ByteCode.Length <= 0) {
			return null;
		}

		byte opcode = seg.Data.ByteCode[0];

		StackPanel panel = new StackPanel() { MaxWidth = 250, HorizontalAlignment = HorizontalAlignment.Left };
		panel.Children.Add(new TextBlock() { Text = desc.Name, FontSize = 14, Margin = new(0, 2), TextWrapping = TextWrapping.Wrap, FontWeight = FontWeight.Bold });
		panel.Children.Add(new TextBlock() { Text = desc.Description, Margin = new(0, 3, 0, 5),  TextWrapping = TextWrapping.Wrap });

		TooltipEntries items = new();
		items.AddCustomEntry("OP", panel);
		if(!string.IsNullOrEmpty(seg.Data.ByteCodeStr)) {
			items.AddEntry("Byte Code", seg.Data.ByteCodeStr);
		}
		if(doc.OpMode != null) {
			items.AddEntry("Mode", doc.OpMode[opcode]);
		}
		if(doc.OpCycleCount != null) {
			items.AddEntry("Cycle Count", doc.OpCycleCount[opcode]);
		}
		if(desc.Flags != null) {
			items.AddEntry("Affected Flags", string.Join(", ", desc.Flags));
		}

		return new DynamicTooltip() { Items = items };
	}

	private static void InitDocumentation(CpuType cpuType, DocFileFormat? doc, Dictionary<string, OpCodeDesc>? baseDesc = null)
	{
		if(doc == null) {
			return;
		}

		Dictionary<string, OpCodeDesc> desc = baseDesc ?? new();
		foreach(OpCodeDesc op in doc.Instructions) {
			desc[op.Op] = op;
		}

		Dictionary<int, string> mode = new();
		Dictionary<int, string> cycleCount = new();
		for(int i = 0; i < 256; i++) {
			if(doc.AddressingModes != null) {
				mode[i] = ResourceHelper.GetEnumText(doc.AddressingModes[i]);
			}

			if(doc.MinCycles != null) {
				string cycles = doc.MinCycles[i].ToString();
				if(doc.MinCycles[i] < doc.MaxCycles?[i]) {
					cycles += "-" + doc.MaxCycles[i];
				}
				cycleCount[i] = cycles;
			}
		}

		_data[cpuType] = new CpuDocumentationData(desc, doc.AddressingModes != null ? mode : null, doc.MinCycles != null ? cycleCount : null);
	}

	private static void InitNesDocumentation()
	{
		InitDocumentation(CpuType.Nes, ReadDocumentationFile("NesDocumentation.json"));
	}

	private static void InitSnesDocumentation()
	{
		Dictionary<string, OpCodeDesc> baseDesc = new(_data[CpuType.Nes].OpDesc);
		InitDocumentation(CpuType.Snes, ReadDocumentationFile("SnesDocumentation.json"), baseDesc);
		InitDocumentation(CpuType.Spc, ReadDocumentationFile("SpcDocumentation.json"));
	}

	private static void InitPceDocumentation()
	{
		Dictionary<string, OpCodeDesc> baseDesc = new(_data[CpuType.Nes].OpDesc);
		InitDocumentation(CpuType.Pce, ReadDocumentationFile("PceDocumentation.json"), baseDesc);
		_data[CpuType.Pce].OpDescGetter = (dict, opName, _) => {
			OpCodeDesc? desc = null;
			int index = opName.IndexOf("."); //ignore anything after a dot (e.g lda.h, etc.)
			opName = index >= 0 ? opName.Substring(0, index) : opName;
			dict.TryGetValue(opName.Substring(0, Math.Min(opName.Length, 3)), out desc);
			return desc;
		};
	}

	private static void InitGbDocumentation()
	{
		InitDocumentation(CpuType.Gameboy, ReadDocumentationFile("GbDocumentation.json"));
	}

	private static void InitSmsDocumentation()
	{
		//TODOSMS add missing descriptions, etc.
		InitDocumentation(CpuType.Sms, ReadDocumentationFile("SmsDocumentation.json"));
	}

	private static void InitWsDocumentation()
	{
		//TODOWS add missing descriptions, etc.
		InitDocumentation(CpuType.Ws, ReadDocumentationFile("WsDocumentation.json"));
	}

	private static void InitGbaDocumentation()
	{
		//TODOGBA add missing descriptions, etc.
		InitDocumentation(CpuType.Gba, ReadDocumentationFile("GbaDocumentation.json"));

		Func<Dictionary<string, OpCodeDesc>, string, CodeSegmentInfo, bool, OpCodeDesc?> parseOp = (dict, opName, codeSegment, checkSetFlags) => {
			OpCodeDesc? desc = null;
			bool updateFlags = false;
			bool byteSize = false;
			bool halfWordSize = false;
			bool signed = false;
			bool incBefore = false;
			bool incAfter = false;
			bool decBefore = false;
			bool decAfter = false;
			string condHint = "";

			Func<bool> tryParseOp = () => {
				dict.TryGetValue(opName, out desc);
				if(desc != null) {
					desc = desc.Clone();

					if(byteSize) {
						desc.Name += " (byte)";
					}

					if(halfWordSize) {
						desc.Name += " (half-word)";
					}

					if(signed) {
						desc.Name += " (signed)";
					}

					if(incAfter) {
						desc.Name += " (post-increment)";
					}
					if(incBefore) {
						desc.Name += " (pre-increment)";
					}
					if(decAfter) {
						desc.Name += " (post-decrement)";
					}
					if(decBefore) {
						desc.Name += " (pre-increment)";
					}

					if(condHint != "") {
						if(!string.IsNullOrEmpty(desc.Description)) {
							desc.Description += Environment.NewLine;
						}
						desc.Description += " -When: " + condHint;
					}

					if(updateFlags) {
						if(!string.IsNullOrEmpty(desc.Description)) {
							desc.Description += Environment.NewLine;
						}
						desc.Description += " -Update flags";
					}
					return true;
				}
				return false;
			};

			if(tryParseOp()) {
				return desc;
			}

			if(checkSetFlags && opName.EndsWith("s")) {
				updateFlags = true;
				opName = opName.Substring(0, opName.Length - 1);
			}

			if(tryParseOp()) {
				return desc;
			}

			if(opName.EndsWith("ib")) {
				incBefore = true;
				opName = opName.Substring(0, opName.Length - 2);
			}

			if(opName.EndsWith("ia")) {
				incAfter = true;
				opName = opName.Substring(0, opName.Length - 2);
			}

			if(opName.EndsWith("db")) {
				decBefore = true;
				opName = opName.Substring(0, opName.Length - 2);
			}

			if(opName.EndsWith("da")) {
				decAfter = true;
				opName = opName.Substring(0, opName.Length - 2);
			}

			if(tryParseOp()) {
				return desc;
			}

			if(opName.EndsWith("b")) {
				byteSize = true;
				opName = opName.Substring(0, opName.Length - 1);
			}

			if(opName.EndsWith("h")) {
				halfWordSize = true;
				opName = opName.Substring(0, opName.Length - 1);
			}

			if(tryParseOp()) {
				return desc;
			}

			if(opName != "mrs" && opName.EndsWith("s")) {
				signed = true;
				opName = opName.Substring(0, opName.Length - 1);
			}

			string[] conditions = { "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "al" };
			string? condition = conditions.FirstOrDefault(opName.EndsWith);
			if(condition != null) {
				condHint = condition switch {
					"eq" => "Equal (Zero Set)",
					"ne" => "Not Equal (Zero Clear)",
					"cs" => "Greater or Equal (Unsigned) (Carry Set)",
					"cc" => "Lower (Unsigned) (Carry Clear)",
					"mi" => "Minus (Negative Set)",
					"pl" => "Plus (Negative Clear)",
					"vs" => "Overflow Set",
					"vc" => "Overflow Clear",
					"hi" => "Greater (Unsigned) (Carry Set, Zero Clear)",
					"ls" => "Lower or Equal (Unsigned) (Carry Clear, Zero Set)",
					"ge" => "Greater or Equal (Negative == Overflow)",
					"lt" => "Lower or Equal (Negative != Overflow)",
					"gt" => "Greater (Zero Clear AND Negative == Overflow)",
					"le" => "Less or Equal (Zero Set OR Negative != Overflow)",
					"al" => "Always",
					_ => ""
				};

				opName = opName.Substring(0, opName.Length - 2);
			}

			if(tryParseOp()) {
				return desc;
			}

			return desc;
		};

		_data[CpuType.Gba].OpDescGetter = (dict, opName, codeSegment) => {
			OpCodeDesc? desc = parseOp(dict, opName, codeSegment, true);
			if(desc == null) {
				desc = parseOp(dict, opName, codeSegment, false);
			}
			return desc;
		};

		//Use same tooltips for ST018 ARMv3 CPU
		_data[CpuType.St018] = _data[CpuType.Gba];
	}

	private static DocFileFormat? ReadDocumentationFile(string filename)
	{
		using StreamReader reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.Documentation." + filename)!);
		return (DocFileFormat?)JsonSerializer.Deserialize(reader.ReadToEnd(), typeof(DocFileFormat), MesenCamelCaseSerializerContext.Default);
	}

	private class CpuDocumentationData
	{
		public Dictionary<string, OpCodeDesc> OpDesc { get; }
		public Dictionary<int, string>? OpMode { get; }
		public Dictionary<int, string>? OpCycleCount { get; }
		public Func<Dictionary<string, OpCodeDesc>, string, CodeSegmentInfo, OpCodeDesc?>? OpDescGetter { get; set; }

		public CpuDocumentationData(Dictionary<string, OpCodeDesc> opDesc, Dictionary<int, string>? opMode, Dictionary<int, string>? opCycleCount)
		{
			OpDesc = opDesc;
			OpMode = opMode;
			OpCycleCount = opCycleCount;
		}
	}
}

public enum AddrMode
{
	//6502
	None, Acc, Imp, Imm, Rel,
	Zero, Abs, ZeroX, ZeroY,
	Ind, IndX, IndY, AbsX, AbsY,

	//65816
	AbsLng, AbsIdxXInd, AbsIdxX,
	AbsIdxY, AbsLngIdxX, AbsIndLng,
	AbsInd,
	BlkMov,
	Dir, DirIdxX, DirInd,
	DirIndLng, DirIndIdxY, DirIdxIndX,
	DirIdxY, DirIndLngIdxY,
	Imm8, Imm16, ImmX, ImmM,
	RelLng,
	Stk, StkRel, StkRelIndIdxY,

	//PCE
	AbsXInd,
	Block,
	ImZero, ImZeroX, ImAbs, ImAbsX,
	ZInd, ZeroRel,
}

public enum CpuFlag
{
	Carry,
	Decimal,
	Interrupt,
	Negative,
	Overflow,
	Zero,
	Memory,
	Index,
	Emulation,
	HalfCarry,
	DirectPage,
	Sign,
	OverflowParity,
	F3,
	F5
}

public class DocFileFormat
{
	public OpCodeDesc[] Instructions { get; set; } = Array.Empty<OpCodeDesc>();
	public AddrMode[]? AddressingModes { get; set; } = null;
	public int[]? MinCycles { get; set; }
	public int[]? MaxCycles { get; set; }
}

public class OpCodeDesc
{
	public string Op { get; set; } = "";
	public string Name { get; set; } = "";
	public string Description { get; set; } = "";
	public CpuFlag[]? Flags { get; set; }

	public OpCodeDesc Clone()
	{
		return (OpCodeDesc)this.MemberwiseClone();
	}
}
