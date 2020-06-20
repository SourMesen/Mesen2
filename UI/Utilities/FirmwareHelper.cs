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

		private static List<string> GetExpectedHashes(FirmwareType type)
		{
			switch(type) {
				case FirmwareType.CX4: return new List<string>() { "AE8D4D1961B93421FF00B3CAA1D0F0CE7783E749772A3369C36B3DBF0D37EF18" };
				case FirmwareType.DSP1: return new List<string>() { "91E87D11E1C30D172556BED2211CCE2EFA94BA595F58C5D264809EF4D363A97B" };
				case FirmwareType.DSP1B: return new List<string>() { "D789CB3C36B05C0B23B6C6F23BE7AA37C6E78B6EE9CEAC8D2D2AA9D8C4D35FA9" };
				case FirmwareType.DSP2: return new List<string>() { "03EF4EF26C9F701346708CB5D07847B5203CF1B0818BF2930ACD34510FFDD717" };
				case FirmwareType.DSP3: return new List<string>() { "0971B08F396C32E61989D1067DDDF8E4B14649D548B2188F7C541B03D7C69E4E" };
				case FirmwareType.DSP4: return new List<string>() { "752D03B2D74441E430B7F713001FA241F8BBCFC1A0D890ED4143F174DBE031DA" };
				case FirmwareType.ST010: return new List<string>() { "FA9BCED838FEDEA11C6F6ACE33D1878024BDD0D02CC9485899D0BDD4015EC24C" };
				case FirmwareType.ST011: return new List<string>() { "8B2B3F3F3E6E29F4D21D8BC736B400BC988B7D2214EBEE15643F01C1FEE2F364" };
				case FirmwareType.ST018: return new List<string>() { "6DF209AB5D2524D1839C038BE400AE5EB20DAFC14A3771A3239CD9E8ACD53806" };
				case FirmwareType.Satellaview: return new List<string>() {
					"27CFDB99F7E4252BF3740D420147B63C4C88616883BC5E7FE43F2F30BF8C8CBB", //Japan, no DRM
					"A49827B45FF9AC9CF5B4658190E1428E59251BC82D8A63D8E9E0F71E439F008F", //English, no DRM
					"3CE321496EDC5D77038DE2034EB3FB354D7724AFD0BC7FD0319F3EB5D57B984D", //Japan, original
					"77D94D64D745014BF8B51280A4204056CDEB9D41EA30EAE80DBC006675BEBEF8", //English, DRM
				};
				case FirmwareType.Gameboy: return new List<string>() { "CF053ECCB4CCAFFF9E67339D4E78E98DCE7D1ED59BE819D2A1BA2232C6FCE1C7", "26E71CF01E301E5DC40E987CD2ECBF6D0276245890AC829DB2A25323DA86818E" };
				case FirmwareType.GameboyColor: return new List<string>() { "B4F2E416A35EEF52CBA161B159C7C8523A92594FACB924B3EDE0D722867C50C7", "3A307A41689BEE99A9A32EA021BF45136906C86B2E4F06C806738398E4F92E45" };
				case FirmwareType.Sgb1GameboyCpu: return new List<string>() { "0E4DDFF32FC9D1EEAAE812A157DD246459B00C9E14F2F61751F661F32361E360" };
				case FirmwareType.Sgb2GameboyCpu: return new List<string>() { "FD243C4FB27008986316CE3DF29E9CFBCDC0CD52704970555A8BB76EDBEC3988" };

				case FirmwareType.SGB1: return new List<string>() {
					"BBA9C269273BEDB9B38BD5EB23BFAA6E509B8DECC7CB80BB5513905AF04F4CEB", //Rev 0 (Japan)
					"C6C4DAAB5C899B69900C460787DE6089EDABE94B760F96D9F583D30CC0A5BB30", //Rev 1 (Japan+USA)
					"A75160F7B89B1F0E20FD2F6441BB86285C7378DB5035EF6885485EAFF6059376", //Rev 2 (World)
				};
				case FirmwareType.SGB2:
					return new List<string>() {
					"C172498A23D1176672931BAB33B629C7D28F914A43DCA9E540B8AF1B37CCF2C6", //SGB2 (Japan)
				};
			}
			throw new Exception("Unexpected firmware type");
		}

		public static void RequestFirmwareFile(MissingFirmwareMessage msg)
		{
			string filename = Utf8Marshaler.GetStringFromIntPtr(msg.Filename);
			if(MesenMsgBox.Show("FirmwareNotFound", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, msg.Firmware.ToString(), filename, msg.Size.ToString()) == DialogResult.OK) {
				using(OpenFileDialog ofd = new OpenFileDialog()) {
					ofd.SetFilter(ResourceHelper.GetMessage("FilterAll"));
					if(ofd.ShowDialog(frmMain.Instance) == DialogResult.OK) {
						List<string> expectedHashes = GetExpectedHashes(msg.Firmware);
						if(!expectedHashes.Contains(GetFileHash(ofd.FileName))) {
							if(MesenMsgBox.Show("FirmwareMismatch", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, msg.Firmware.ToString(), GetFileHash(ofd.FileName), expectedHashes[0]) != DialogResult.OK) {
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
