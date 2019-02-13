using System.Collections.Generic;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	public static class OpenFileDialogExtensions 
	{
		private static string ToCaseInsensitiveFilter(string filter)
		{
			if(Program.IsMono) {
				string[] filterData = filter.Split('|');
				for(int i = 0; i < filterData.Length; i+=2) {
					List<string> fileTypes = new List<string>(filterData[i+1].Split(';'));
					for(int j = 0, len = fileTypes.Count; j < len; j++) {
						fileTypes[j] = fileTypes[j].ToUpper();
						fileTypes.Add(fileTypes[j].ToLower());
					}
					filterData[i+1] = string.Join(";", fileTypes.ToArray());
				}
				return string.Join("|", filterData);
			} else {
				return filter;
			}
		} 

		public static void SetFilter(this OpenFileDialog ofd, string filter)
		{
			ofd.Filter = ToCaseInsensitiveFilter(filter);
		}

		public static void SetFilter(this SaveFileDialog ofd, string filter)
		{
			ofd.Filter = ToCaseInsensitiveFilter(filter);
		}		
	} 
	
}