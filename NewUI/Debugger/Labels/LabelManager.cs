using Mesen.Debugger;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Debugger.Labels
{
	public class LabelManager
	{
		public static Regex LabelRegex { get; } = new Regex("^[@_a-zA-Z]+[@_a-zA-Z0-9]*$", RegexOptions.Compiled);
		public static Regex AssertRegex { get; } = new Regex(@"assert\((.*)\)", RegexOptions.Compiled);

		private static Dictionary<UInt64, CodeLabel> _labelsByKey = new Dictionary<UInt64, CodeLabel>();
		private static HashSet<CodeLabel> _labels = new HashSet<CodeLabel>();
		private static Dictionary<string, CodeLabel> _reverseLookup = new Dictionary<string, CodeLabel>();

		public static event EventHandler? OnLabelUpdated;

		public static void ResetLabels()
		{
			DebugApi.ClearLabels();
			_labels.Clear();
			_labelsByKey.Clear();
			_reverseLookup.Clear();
		}

		public static CodeLabel? GetLabel(UInt32 address, MemoryType type)
		{
			CodeLabel? label;
			_labelsByKey.TryGetValue(GetKey(address, type), out label);
			return label;
		}

		public static CodeLabel? GetLabel(AddressInfo addr)
		{
			CodeLabel? label = GetLabel((UInt32)addr.Address, addr.Type);
			if(label != null) {
				return label;
			}

			if(addr.Type.IsRelativeMemory()) {
				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(addr);
				if(absAddress.Address >= 0) {
					return GetLabel((UInt32)absAddress.Address, absAddress.Type);
				}
			}
			return null;
		}

		public static CodeLabel? GetLabel(string label)
		{
			return _reverseLookup.ContainsKey(label) ? _reverseLookup[label] : null;
		}

		public static void SetLabels(IEnumerable<CodeLabel> labels, bool raiseEvents = true)
		{
			foreach(CodeLabel label in labels) {
				SetLabel(label, false);
			}
			if(raiseEvents) {
				ProcessLabelUpdate();
			}
		}

		public static List<CodeLabel> GetLabels(CpuType cpu)
		{
			if(cpu == CpuType.Sa1 || cpu == CpuType.Gsu) {
				//Share label list between SNES CPU, SA1 and GSU (since the share the same PRG ROM)
				cpu = CpuType.Snes;
			}

			return _labels.Where((lbl) => lbl.Matches(cpu)).ToList<CodeLabel>();
		}

		public static List<CodeLabel> GetAllLabels()
		{
			return _labels.ToList();
		}

		private static UInt64 GetKey(UInt32 address, MemoryType memType)
		{
			return address | ((UInt64)memType << 32);
		}

		public static void SetLabel(uint address, MemoryType memType, string label, string comment)
		{
			LabelManager.SetLabel(new CodeLabel() {
				Address = address,
				MemoryType = memType,
				Label = label,
				Comment = comment
			}, false);
		}

		public static bool SetLabel(CodeLabel label, bool raiseEvent)
		{
			if(_reverseLookup.ContainsKey(label.Label)) {
				//Another identical label exists, we need to remove it
				DeleteLabel(_reverseLookup[label.Label], false);
			}

			string comment = label.Comment;
			for(UInt32 i = label.Address; i < label.Address + label.Length; i++) {
				UInt64 key = GetKey(i, label.MemoryType);
				CodeLabel? existingLabel;
				if(_labelsByKey.TryGetValue(key, out existingLabel)) {
					DeleteLabel(existingLabel, false);
					_reverseLookup.Remove(existingLabel.Label);
				}

				_labelsByKey[key] = label;

				if(label.Length == 1) {
					DebugApi.SetLabel(i, label.MemoryType, label.Label, comment.Replace(Environment.NewLine, "\n"));
				} else {
					DebugApi.SetLabel(i, label.MemoryType, label.Label + "+" + (i - label.Address).ToString(), comment.Replace(Environment.NewLine, "\n"));

					//Only set the comment on the first byte of multi-byte comments
					comment = "";
				}
			}

			_labels.Add(label);
			if(label.Label.Length > 0) {
				_reverseLookup[label.Label] = label;
			}

			if(raiseEvent) {
				ProcessLabelUpdate();
			}

			return true;
		}

		public static void DeleteLabel(CodeLabel label, bool raiseEvent)
		{
			bool needEvent = false;

			_labels.Remove(label);
			for(UInt32 i = label.Address; i < label.Address + label.Length; i++) {
				UInt64 key = GetKey(i, label.MemoryType);
				if(_labelsByKey.ContainsKey(key)) {
					_reverseLookup.Remove(_labelsByKey[key].Label);
				}

				if(_labelsByKey.Remove(key)) {
					DebugApi.SetLabel(i, label.MemoryType, string.Empty, string.Empty);
					if(raiseEvent) {
						needEvent = true;
					}
				}
			}

			if(needEvent) {
				ProcessLabelUpdate();
			}
		}

		/*public static void CreateAutomaticJumpLabels()
		{
			byte[] cdlData = InteropEmu.DebugGetPrgCdlData();
			List<CodeLabel> labelsToAdd = new List<CodeLabel>();
			for(int i = 0; i < cdlData.Length; i++) {
				if((cdlData[i] & (byte)CdlPrgFlags.JumpTarget) != 0 && LabelManager.GetLabel((uint)i, SnesMemoryType.PrgRom) == null) {
					labelsToAdd.Add(new CodeLabel() { Flags = CodeLabelFlags.AutoJumpLabel, Address = (uint)i, SnesMemoryType = SnesMemoryType.PrgRom, Label = "L" + i.ToString("X4"), Comment = "" });
				}
			}
			if(labelsToAdd.Count > 0) {
				LabelManager.SetLabels(labelsToAdd, true);
			}
		}*/

		public static void RefreshLabels()
		{
			DebugApi.ClearLabels();
			LabelManager.SetLabels(new List<CodeLabel>(_labels), true);
		}

		private static void ProcessLabelUpdate()
		{
			OnLabelUpdated?.Invoke(null, EventArgs.Empty);
			UpdateAssertBreakpoints();
		}

		private static void UpdateAssertBreakpoints()
		{
			List<Breakpoint> asserts = new List<Breakpoint>();

			Action<CodeLabel, string, CpuType> addAssert = (CodeLabel label, string condition, CpuType cpuType) => {
				asserts.Add(new Breakpoint() {
					BreakOnExec = true,
					MemoryType = label.MemoryType,
					CpuType = cpuType,
					//TODO Address = label.Address,
					Condition = "!(" + condition + ")",
					//TODO IsAssert = true
				});
			};

			foreach(CodeLabel label in _labels) {
				foreach(string commentLine in label.Comment.Split('\n')) {
					Match m = LabelManager.AssertRegex.Match(commentLine);
					if(m.Success) {
						CpuType cpuType = label.MemoryType.ToCpuType();
						addAssert(label, m.Groups[1].Value, cpuType);
						if(cpuType == CpuType.Snes) {
							addAssert(label, m.Groups[1].Value, CpuType.Sa1);
						}
					}
				}
			}

			//TODO
			/*BreakpointManager.Asserts = asserts;
			BreakpointManager.SetBreakpoints();*/
		}
	}

	[Flags]
	public enum CodeLabelFlags
	{
		None = 0,
		AutoJumpLabel = 1
	}
}
