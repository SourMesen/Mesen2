using Mesen.Debugger;
using Mesen.Interop;
using Mesen.ViewModels;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;

namespace Mesen.Debugger.Labels
{
	public class LabelManager
	{
		public static Regex LabelRegex { get; } = new Regex("^[@_a-zA-Z]+[@_a-zA-Z0-9]*$", RegexOptions.Compiled);
		public static Regex InvalidLabelRegex { get; } = new Regex("[^@_a-zA-Z0-9]", RegexOptions.Compiled);
		public static Regex AssertRegex { get; } = new Regex(@"assert\((.*)\)", RegexOptions.Compiled);

		private static Dictionary<UInt64, CodeLabel> _labelsByKey = new Dictionary<UInt64, CodeLabel>();
		private static HashSet<CodeLabel> _labels = new HashSet<CodeLabel>();
		private static Dictionary<string, CodeLabel> _reverseLookup = new Dictionary<string, CodeLabel>();

		public static event EventHandler? OnLabelUpdated;

		private static int _suspendEvents = 0;
		public static void SuspendEvents() { Interlocked.Increment(ref _suspendEvents); }
		public static void ResumeEvents() { Interlocked.Decrement(ref _suspendEvents); ProcessLabelUpdate(); }

		public static void ResetLabels()
		{
			DebugApi.ClearLabels();
			_labels.Clear();
			_labelsByKey.Clear();
			_reverseLookup.Clear();

			ProcessLabelUpdate();
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

		public static bool ContainsLabel(CodeLabel label)
		{
			return _labels.Contains(label);
		}

		public static void SetLabels(IEnumerable<CodeLabel> labels, bool raiseEvents = true)
		{
			Dictionary<MemoryType, bool> isAvailable = new();

			foreach(CodeLabel label in labels) {
				//Check if label memory type is valid before adding it to the list
				if(!isAvailable.TryGetValue(label.MemoryType, out bool available)) {
					available = DebugApi.GetMemorySize(label.MemoryType) > 0;
					isAvailable[label.MemoryType] = available;
				}

				if(available) {
					SetLabel(label, false);
				}
			}
			if(raiseEvents) {
				ProcessLabelUpdate();
			}
		}

		public static List<CodeLabel> GetLabels(CpuType cpu)
		{
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

		public static void DeleteLabels(IEnumerable<CodeLabel> labels)
		{
			foreach(CodeLabel label in labels) {
				DeleteLabel(label, false);
			}
			ProcessLabelUpdate();
		}

		public static void RefreshLabels(bool raiseEvent)
		{
			DebugApi.ClearLabels();
			LabelManager.SetLabels(new List<CodeLabel>(_labels), raiseEvent);
		}

		private static void ProcessLabelUpdate()
		{
			if(_suspendEvents == 0) {
				OnLabelUpdated?.Invoke(null, EventArgs.Empty);
				UpdateAssertBreakpoints();
			}
		}

		private static void UpdateAssertBreakpoints()
		{
			List<Breakpoint> asserts = new List<Breakpoint>();

			Action<CodeLabel, string, CpuType> addAssert = (CodeLabel label, string condition, CpuType cpuType) => {
				asserts.Add(new Breakpoint() {
					BreakOnExec = true,
					MemoryType = label.MemoryType,
					CpuType = cpuType,
					StartAddress = label.Address,
					EndAddress = label.Address,
					Condition = "!(" + condition + ")",
					IsAssert = true
				});
			};

			foreach(CodeLabel label in _labels) {
				foreach(string commentLine in label.Comment.Split('\n')) {
					Match m = LabelManager.AssertRegex.Match(commentLine);
					if(m.Success) {
						foreach(CpuType cpuType in MainWindowViewModel.Instance.RomInfo.CpuTypes) {
							if(cpuType.CanAccessMemoryType(label.MemoryType)) {
								addAssert(label, m.Groups[1].Value, cpuType);
							}
						}
					}
				}
			}

			BreakpointManager.Asserts = asserts;
			BreakpointManager.SetBreakpoints();
		}
	}

	[Flags]
	public enum CodeLabelFlags
	{
		None = 0,
		AutoJumpLabel = 1
	}
}
