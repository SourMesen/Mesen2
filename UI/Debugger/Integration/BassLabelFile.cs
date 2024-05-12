using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Text;

namespace Mesen.Debugger.Integration;

public class BassLabelFile
{
	public static void Import(string path, bool showResult, CpuType cpuType)
	{
		List<CodeLabel> labels = new List<CodeLabel>(1000);

		MemoryType memType = cpuType.ToMemoryType();
		int errorCount = 0;
		foreach(string row in File.ReadAllLines(path, Encoding.UTF8)) {
			string lineData = row.Trim();
			int splitIndex = lineData.IndexOf(' ');
			if(splitIndex < 0) {
				errorCount++;
				continue;
			}

			UInt32 address;
			
			if(!UInt32.TryParse(lineData.Substring(0, splitIndex), NumberStyles.HexNumber, null, out address)) {
				errorCount++;
				continue;
			}

			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)address, Type = memType });

			if(absAddress.Address >= 0) {
				CodeLabel label = new CodeLabel();
				label.Address = (UInt32)absAddress.Address;
				label.MemoryType = absAddress.Type;
				label.Comment = "";

				string labelName = LabelManager.InvalidLabelRegex.Replace(lineData.Substring(splitIndex + 1), "_");
				if(string.IsNullOrEmpty(labelName) || !LabelManager.LabelRegex.IsMatch(labelName)) {
					errorCount++;
				} else {
					label.Label = labelName;
					if(ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(label.MemoryType)) {
						labels.Add(label);
					}
				}
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
