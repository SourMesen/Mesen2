using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Utilities
{
	public static class FirmwareHelper
	{
		private static string GetFileHash(string filename)
		{
			using(SHA256 sha256Hash = SHA256.Create()) {
				// ComputeHash - returns byte array  
				byte[] bytes = sha256Hash.ComputeHash(File.ReadAllBytes(filename));

				// Convert byte array to a string   
				StringBuilder builder = new StringBuilder();
				for(int i = 0; i < bytes.Length; i++) {
					builder.Append(bytes[i].ToString("X2"));
				}
				return builder.ToString();
			}
		}

		private static string GetExpectedHash(CoprocessorType type)
		{
			switch(type) {
				case CoprocessorType.CX4: return "AE8D4D1961B93421FF00B3CAA1D0F0CE7783E749772A3369C36B3DBF0D37EF18";
				case CoprocessorType.DSP1: return "91E87D11E1C30D172556BED2211CCE2EFA94BA595F58C5D264809EF4D363A97B";
				case CoprocessorType.DSP1B: return "D789CB3C36B05C0B23B6C6F23BE7AA37C6E78B6EE9CEAC8D2D2AA9D8C4D35FA9";
				case CoprocessorType.DSP2: return "03EF4EF26C9F701346708CB5D07847B5203CF1B0818BF2930ACD34510FFDD717";
				case CoprocessorType.DSP3: return "0971B08F396C32E61989D1067DDDF8E4B14649D548B2188F7C541B03D7C69E4E";
				case CoprocessorType.DSP4: return "752D03B2D74441E430B7F713001FA241F8BBCFC1A0D890ED4143F174DBE031DA";
				case CoprocessorType.ST010: return "FA9BCED838FEDEA11C6F6ACE33D1878024BDD0D02CC9485899D0BDD4015EC24C";
				case CoprocessorType.ST011: return "8B2B3F3F3E6E29F4D21D8BC736B400BC988B7D2214EBEE15643F01C1FEE2F364";
				case CoprocessorType.ST018: return "6DF209AB5D2524D1839C038BE400AE5EB20DAFC14A3771A3239CD9E8ACD53806";
			}
			throw new Exception("Unexpected coprocessor type");
		}

		public static void RequestFirmwareFile(MissingFirmwareMessage msg)
		{
			string filename = Utf8Marshaler.GetStringFromIntPtr(msg.Filename);
			if(MesenMsgBox.Show("FirmwareNotFound", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, msg.FirmwareType.ToString(), filename, msg.Size.ToString()) == DialogResult.OK) {
				using(OpenFileDialog ofd = new OpenFileDialog()) {
					ofd.SetFilter(ResourceHelper.GetMessage("FilterAll"));
					if(ofd.ShowDialog(frmMain.Instance) == DialogResult.OK) {
						if(GetFileHash(ofd.FileName) != GetExpectedHash(msg.FirmwareType)) {
							if(MesenMsgBox.Show("FirmwareMismatch", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, msg.FirmwareType.ToString(), GetFileHash(ofd.FileName), GetExpectedHash(msg.FirmwareType)) != DialogResult.OK) {
								//Files don't match and user cancelled the action
								return;
							}
						}
						File.Copy(ofd.FileName, Path.Combine(ConfigManager.FirmwareFolder, filename));
					}
				}
			}
		}
	}
}
