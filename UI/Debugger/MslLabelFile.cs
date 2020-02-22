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
	public class MslLabelFile
	{
		public static void Import(string path, bool silent = false)
		{
			List<CodeLabel> labels = new List<CodeLabel>(1000);

			int errorCount = 0;
			foreach(string row in File.ReadAllLines(path, Encoding.UTF8)) {
				CodeLabel label = CodeLabel.FromString(row);
				if(label == null) {
					errorCount++;
				} else {
					labels.Add(label);
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

		public static void Export(string path)
		{
			List<CodeLabel> labels = LabelManager.GetAllLabels();

			labels.Sort((CodeLabel a, CodeLabel b) => {
				int result = a.MemoryType.CompareTo(b.MemoryType);
				if(result == 0) {
					return a.Address.CompareTo(b.Address);
				} else {
					return result;
				}
			});

			StringBuilder sb = new StringBuilder();
			foreach(CodeLabel label in labels) {
				sb.Append(label.ToString() + "\n");
			}
			File.WriteAllText(path, sb.ToString(), Encoding.UTF8);
		}
	}
}
