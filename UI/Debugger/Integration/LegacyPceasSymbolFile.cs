using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace Mesen.Debugger.Integration;

public class LegacyPceasSymbolFile
{
	public static bool IsValidFile(string content)
	{
		return content.Contains("Bank\tAddr\tLabel") || content.Contains("Label\t\t\t\tAddr\tBank");
	}

	private static bool GetBankAddressLabel(string row, bool isAltFieldOrder, out UInt32 address, out UInt32 bank, out string? labelName)
	{
		address = 0;
		bank = 0;
		labelName = null;

		int bankIndex = 0;
		int addrIndex = 1;
		int labelIndex = 2;
		if(isAltFieldOrder) {
			labelIndex = 0;
			addrIndex = 1;
			bankIndex = 2;
		}

		row = row.Trim();
		if(row.Length == 0) {
			return true;
		}

		string[] rowData = row.Split('\t', StringSplitOptions.RemoveEmptyEntries | StringSplitOptions.TrimEntries);
		if(rowData.Length != 3) {
			return false;
		}

		if(!UInt32.TryParse(rowData[bankIndex], NumberStyles.HexNumber, null, out bank)) {
			return false;
		}

		if(!UInt32.TryParse(rowData[addrIndex], NumberStyles.HexNumber, null, out address)) {
			return false;
		}

		labelName = LabelManager.InvalidLabelRegex.Replace(rowData[labelIndex], "_");
		if(string.IsNullOrEmpty(labelName) || !LabelManager.LabelRegex.IsMatch(labelName)) {
			return false;
		}
		return true;
	}

	public static void Import(string path, bool showResult)
	{
		List<CodeLabel> labels = new List<CodeLabel>(1000);

		Dictionary<MemoryType, int> sizeByMemoryType = new();

		string[] lines = File.ReadAllLines(path, Encoding.UTF8);
		if(lines.Length < 2) {
			return;
		}

		bool isAltFieldOrder = lines[0].Contains("Label\t\t\t\tAddr\tBank");

		int errorCount = 0;
		foreach(string row in lines.Skip(2)) {
			UInt32 address;
			UInt32 bank;
			string? labelName;

			if(!GetBankAddressLabel(row, isAltFieldOrder, out address, out bank, out labelName)) {
				errorCount++;
				continue;
			} else if(labelName == null) {
				//Empty line
				continue;
			} else if(address > 0xFFFF) {
				errorCount++;
				continue;
			}

			AddressInfo absAddress = new();
			if(bank == 0xF8) {
				absAddress.Address = (int)(address & 0x1FFF);
				absAddress.Type = MemoryType.PceWorkRam;
			} else if(bank == 0xF0) {
				//these appear to be constants - ignore them?
				continue;
			} else if(bank < 0x80) {
				absAddress.Address = (int)((bank * 0x2000) + (address & 0x1FFF));
				absAddress.Type = MemoryType.PcePrgRom;
			}

			if(absAddress.Address >= 0) {
				if(!sizeByMemoryType.TryGetValue(absAddress.Type, out int size)) {
					sizeByMemoryType[absAddress.Type] = size = DebugApi.GetMemorySize(absAddress.Type);
				}

				if(absAddress.Address >= size) {
					errorCount++;
					continue;
				}

				CodeLabel label = new CodeLabel();
				label.Address = (UInt32)absAddress.Address;
				label.MemoryType = absAddress.Type;
				label.Comment = "";
				label.Label = labelName;
				if(ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(label.MemoryType)) {
					labels.Add(label);
				}
			} else {
				errorCount++;
			}
		}

		LabelManager.SetLabels(labels);

		if(showResult) {
			if(errorCount > 0) {
				MesenMsgBox.Show(null, "ImportLabelsWithErrors", MessageBoxButtons.OK, MessageBoxIcon.Warning, labels.Count.ToString(), errorCount.ToString());
			} else {
				MesenMsgBox.Show(null, "ImportLabels", MessageBoxButtons.OK, MessageBoxIcon.Info, labels.Count.ToString());
			}
		}
	}
}
