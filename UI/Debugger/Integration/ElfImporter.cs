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
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Debugger.Integration;

public abstract class ElfImporter
{
	public static void Import(string path, bool showResult, CpuType cpuType)
	{
		switch(cpuType) {
			case CpuType.Ws: new ElfImporterWs().PrivateImport(path, showResult, cpuType); break;
			case CpuType.Gba: new ElfImporterGba().PrivateImport(path, showResult, cpuType); break;
		}
	}

	protected abstract bool TryGetSymbolInfo(SymbolEntry<uint> symbol, int romSize, [MaybeNullWhen(false)] out ElfSymbolInfo symbolInfo);

	private void PrivateImport(string path, bool showResult, CpuType cpuType)
	{
		try {
			MemoryType memType = cpuType.ToMemoryType();

			Dictionary<AddressInfo, CodeLabel> labels = new();
			HashSet<string> usedLabels = new();
			IELF elf = ELFReader.Load(path);
			int romSize = DebugApi.GetMemorySize(cpuType.GetPrgRomMemoryType());

			if(elf.TryGetSection(".symtab", out ISection section)) {
				if(section is ISymbolTable symbols) {
					foreach(SymbolEntry<uint> symbol in symbols.Entries) {
						if(!string.IsNullOrWhiteSpace(symbol.Name) && symbol.Type != SymbolType.File && symbol.Type != SymbolType.Section && symbol.PointedSection != null) {
							if(!TryGetSymbolInfo(symbol, romSize, out ElfSymbolInfo? symbolInfo)) {
								continue;
							}

							string name = symbolInfo.Name;
							AddressInfo labelAddr = symbolInfo.Address;

							if(!ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(labelAddr.Type)) {
								continue;
							}

							if(symbol.Name.StartsWith("$") && labels.ContainsKey(labelAddr)) {
								continue;
							}

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

			while(true) {
				while(i < name.Length && (name[i] < '0' || name[i] > '9')) {
					i++;
				}

				bool hasLen = false;
				int start = i;
				while(i < name.Length && name[i] >= '0' && name[i] <= '9') {
					i++;
					hasLen = true;
				}

				if(hasLen) {
					int val = int.Parse(name.AsSpan(start, i - start));

					if(i + val <= name.Length) {
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

public class ElfSymbolInfo
{
	public AddressInfo Address;
	public string Name = "";
}

public class ElfImporterWs : ElfImporter
{
	protected override bool TryGetSymbolInfo(SymbolEntry<uint> symbol, int romSize, [MaybeNullWhen(false)] out ElfSymbolInfo symbolInfo)
	{
		symbolInfo = null;

		if(symbol.Name.EndsWith("$") || symbol.Name.EndsWith("&") || symbol.Name.EndsWith("!")) {
			return false;
		}

		AddressInfo absAddr = new();
		uint addr = symbol.PointedSection.LoadAddress + symbol.Value;
		if((addr & 0x80000000) != 0) {
			absAddr.Address = (int)((addr & 0xFFFF) | ((addr & 0x7FF00000) >> 4)) & (romSize - 1);
			absAddr.Type = MemoryType.WsPrgRom;
		} else if(addr <= 0xFFFF) {
			absAddr.Address = (int)addr;
			absAddr.Type = MemoryType.WsWorkRam;
		} else if((addr & 0xF0000) == 0x10000) {
			absAddr.Address = (int)((addr & 0xFFFF) | ((addr & 0xF00000) >> 4));
			absAddr.Type = MemoryType.WsCartRam;
		}
		
		symbolInfo = new() {
			Address = absAddr,
			Name = symbol.Name
		};

		return true;
	}
}

public class ElfImporterGba : ElfImporter
{
	protected override bool TryGetSymbolInfo(SymbolEntry<uint> symbol, int romSize, [MaybeNullWhen(false)] out ElfSymbolInfo symbolInfo)
	{
		symbolInfo = null;

		uint value = symbol.Value & ~(uint)0x01;
		AddressInfo relAddr = new AddressInfo() { Address = (int)value, Type = MemoryType.GbaMemory };
		AddressInfo absAddr = DebugApi.GetAbsoluteAddress(relAddr);

		if(absAddr.Type == MemoryType.None) {
			return false;
		}

		AddressInfo labelAddr = absAddr;

		string name = symbol.Name switch {
			"$a" => "arm_" + relAddr.Address.ToString("X7"),
			"$d" => "data_" + relAddr.Address.ToString("X7"),
			"$t" => "thumb_" + relAddr.Address.ToString("X7"),
			_ => symbol.Name
		};

		symbolInfo = new() {
			Name = name,
			Address = absAddr
		};

		return true;
	}
}