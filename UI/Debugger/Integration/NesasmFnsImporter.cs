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
using System.Threading.Tasks;

namespace Mesen.Debugger.Integration;

public class NesasmFnsImporter
{
	public static void Import(string path, bool showResult)
	{
		//This only works reliably for NROM games with 32kb PRG
		int errorCount = 0;
		
		bool hasLargePrg = DebugApi.GetMemorySize(MemoryType.NesPrgRom) != 0x8000;
		Dictionary<UInt32, CodeLabel> labels = new Dictionary<uint, CodeLabel>();

		char[] separator = new char[1] { '=' };
		foreach(string row in File.ReadAllLines(path, Encoding.UTF8)) {
			string[] rowData = row.Split(separator);
			if(rowData.Length < 2) {
				//Invalid row
				continue;
			}

			uint address;
			if(UInt32.TryParse(rowData[1].Trim().Substring(1), NumberStyles.HexNumber, CultureInfo.InvariantCulture, out address)) {
				string labelName = LabelManager.InvalidLabelRegex.Replace(rowData[0].Trim(), "_");
				if(!LabelManager.LabelRegex.IsMatch(labelName)) {
					//Reject labels that don't respect the label naming restrictions
					errorCount++;
					continue;
				}

				CodeLabel? codeLabel;
				if(!labels.TryGetValue(address, out codeLabel)) {
					codeLabel = new CodeLabel();
					codeLabel.Address = hasLargePrg ? address : (address - 0x8000);
					codeLabel.MemoryType = hasLargePrg ? MemoryType.NesMemory : MemoryType.NesPrgRom;
					codeLabel.Label = "";
					codeLabel.Comment = "";
					if(ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(codeLabel.MemoryType)) {
						labels[address] = codeLabel;
					}
				}

				codeLabel.Label = labelName;
			} else {
				errorCount++;
			}
		}

		LabelManager.SetLabels(labels.Values);

		if(showResult) {
			if(errorCount > 0) {
				MesenMsgBox.Show(null, "ImportLabelsWithErrors", MessageBoxButtons.OK, MessageBoxIcon.Warning, labels.Count.ToString(), errorCount.ToString());
			} else {
				MesenMsgBox.Show(null, "ImportLabels", MessageBoxButtons.OK, MessageBoxIcon.Info, labels.Count.ToString());
			}
		}
	}
}
