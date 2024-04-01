using ELFSharp.ELF;
using ELFSharp.ELF.Sections;
using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Debugger.Integration;

public class ElfImporter
{
	public static void Import(string path, bool showResult, CpuType cpuType)
	{
		try {
			MemoryType memType = cpuType.ToMemoryType();

			Dictionary<AddressInfo, CodeLabel> labels = new();
			HashSet<string> usedLabels = new();
			IELF elf = ELFReader.Load(path);
			if(elf.TryGetSection(".symtab", out ISection section)) {
				if(section is ISymbolTable symbols) {
					foreach(SymbolEntry<uint> symbol in symbols.Entries) {
						if(!string.IsNullOrWhiteSpace(symbol.Name) && symbol.Type != SymbolType.File && symbol.Type != SymbolType.Section && symbol.Type != SymbolType.Object && symbol.PointedSection != null) {
							uint value = symbol.Value & ~(uint)0x01;
							AddressInfo relAddr = new AddressInfo() { Address = (int)value, Type = memType };
							AddressInfo absAddr = DebugApi.GetAbsoluteAddress(relAddr);
							AddressInfo labelAddr = absAddr.Type != MemoryType.None ? absAddr : relAddr;

							if(!ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(labelAddr.Type)) {
								continue;
							}

							if(symbol.Name.StartsWith("$") && labels.ContainsKey(labelAddr)) {
								continue;
							}

							string name = symbol.Name switch {
								"$a" => "arm_" + relAddr.Address.ToString("X7"),
								"$d" => "data_" + relAddr.Address.ToString("X7"),
								"$t" => "thumb_" + relAddr.Address.ToString("X7"),
								_ => symbol.Name
							};

							//Demangle and replace any invalid characters with underscores
							name = LabelManager.InvalidLabelRegex.Replace(Demangle(name), "_");

							if(labels.Remove(labelAddr, out CodeLabel? existingLabel)) {
								usedLabels.Remove(existingLabel.Label);
							}

							int j = 0;
							string orgLabel = name;
							while(usedLabels.Contains(name)) {
								j++;
								name = orgLabel + j.ToString();
							}

							usedLabels.Add(name);

							labels[labelAddr] = new CodeLabel() {
								Label = name,
								Address = (uint)labelAddr.Address,
								MemoryType = labelAddr.Type,
								Length = symbol.Type != SymbolType.Function && symbol.Size > 1 ? symbol.Size : 1
							};
						}
					}
				}
			}

			if(labels.Count > 0) {
				LabelManager.SetLabels(labels.Values);
			}

			if(showResult) {
				MesenMsgBox.Show(null, "ImportLabels", MessageBoxButtons.OK, MessageBoxIcon.Info, labels.Count.ToString());
			}
		} catch(Exception ex) {
			if(showResult) {
				MesenMsgBox.ShowException(ex);
			}
		}
	}

	private static string Demangle(string name)
	{
		int index = name.IndexOf("_Z");
		if(index >= 0) {
			List<string> parts = new();
			int i = 0;

			while(i < name.Length && (name[i] < '0' || name[i] > '9')) {
				i++;
			}

			while(true) {
				bool hasLen = false;
				int start = i;
				while(i < name.Length && name[i] >= '0' && name[i] <= '9') {
					i++;
					hasLen = true;
				}

				if(hasLen) {
					int val = int.Parse(name.AsSpan(start, i - start));

					if(start + val <= name.Length) {
						string part = name.Substring(i, val);
						if(!string.IsNullOrWhiteSpace(part) && part != "_GLOBAL__N_1") {
							parts.Add(part);
						}
						i += val;
					} else {
						break;
					}
				} else {
					break;
				}
			}

			if(parts.Count > 0) {
				return string.Join('_', parts);
			}
		}

		return name;
	}
}
