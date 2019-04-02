using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	partial class frmAbout : BaseForm
	{
		public frmAbout()
		{
			InitializeComponent();

			lblMesenVersion.Text = EmuApi.GetMesenVersion().ToString(3);

			Version ver = Assembly.GetEntryAssembly().GetName().Version;
			DateTime buildTime = new DateTime(2000, 1, 1, 0, 0, 0, DateTimeKind.Utc).AddDays(ver.Build).AddSeconds(ver.Revision * 2);
			lblBuildDate.Text = buildTime.ToShortDateString() + " " + buildTime.ToShortTimeString();

#if AUTOBUILD
			string devVersion = ResourceExtractor.ReadZippedResource("DevBuild.txt");
			if(devVersion != null) {
				lblMesenVersion.Text = devVersion;
			}
#endif
		}

		private void lblLink_Click(object sender, EventArgs e)
		{
			Process.Start("https://www.mesen.ca/snes/");
		}

		private void picDonate_Click(object sender, EventArgs e)
		{
			Process.Start("https://www.mesen.ca/Donate.php?l=" + ResourceHelper.GetLanguageCode());
		}
	}
}
