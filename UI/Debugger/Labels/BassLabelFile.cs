using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Labels;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public class BassLabelFile
	{
		public static void Import(string path, bool silent = false)
		{
			List<CodeLabel> labels = new List<CodeLabel>(1000);

			int errorCount = 0;
			foreach(string row in File.ReadAllLines(path, Encoding.UTF8)) {
				string lineData = row.Trim();
				int splitIndex = lineData.IndexOf(' ');
				UInt32 address;
				
				if(!UInt32.TryParse(lineData.Substring(0, splitIndex), NumberStyles.HexNumber, null, out address)) {
					errorCount++;
				}

				AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)address, Type = SnesMemoryType.CpuMemory });

				if(absAddress.Address >= 0) {
					CodeLabel label = new CodeLabel();
					label.Address = (UInt32)absAddress.Address;
					label.MemoryType = absAddress.Type;
					label.Comment = "";
					string labelName = lineData.Substring(splitIndex + 1).Replace('.', '_');
					if(string.IsNullOrEmpty(labelName) || !LabelManager.LabelRegex.IsMatch(labelName)) {
						errorCount++;
					} else {
						label.Label = labelName;
						labels.Add(label);
					}
				}
			}

			LabelManager.SetLabels(labels);

			if(!silent) {
				string message = $"Import completed with {labels.Count} labels imported";
				if(errorCount > 0) {
					message += $" and {errorCount} error(s)";
				}
				MessageBox.Show(message, "Mesen-S", MessageBoxButtons.OK, MessageBoxIcon.Information);
			}
		}
	}
}
