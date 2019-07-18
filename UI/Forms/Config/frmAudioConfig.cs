using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmAudioConfig : BaseConfigForm
	{
		public frmAudioConfig()
		{
			InitializeComponent();
			if(this.DesignMode) {
				return;
			}

			cboAudioDevice.Items.AddRange(ConfigApi.GetAudioDevices().ToArray());

			Entity = ConfigManager.Config.Audio.Clone();

			AddBinding(nameof(AudioConfig.EnableAudio), chkEnableAudio);
			AddBinding(nameof(AudioConfig.MasterVolume), trkMaster);
			AddBinding(nameof(AudioConfig.AudioLatency), nudLatency);
			AddBinding(nameof(AudioConfig.SampleRate), cboSampleRate);
			AddBinding(nameof(AudioConfig.AudioDevice), cboAudioDevice);
			AddBinding(nameof(AudioConfig.DisableDynamicSampleRate), chkDisableDynamicSampleRate);

			AddBinding(nameof(AudioConfig.EnableEqualizer), chkEnableEqualizer);
			AddBinding(nameof(AudioConfig.Band1Gain), trkBand1Gain);
			AddBinding(nameof(AudioConfig.Band2Gain), trkBand2Gain);
			AddBinding(nameof(AudioConfig.Band3Gain), trkBand3Gain);
			AddBinding(nameof(AudioConfig.Band4Gain), trkBand4Gain);
			AddBinding(nameof(AudioConfig.Band5Gain), trkBand5Gain);
			AddBinding(nameof(AudioConfig.Band6Gain), trkBand6Gain);
			AddBinding(nameof(AudioConfig.Band7Gain), trkBand7Gain);
			AddBinding(nameof(AudioConfig.Band8Gain), trkBand8Gain);
			AddBinding(nameof(AudioConfig.Band9Gain), trkBand9Gain);
			AddBinding(nameof(AudioConfig.Band10Gain), trkBand10Gain);
			AddBinding(nameof(AudioConfig.Band11Gain), trkBand11Gain);
			AddBinding(nameof(AudioConfig.Band12Gain), trkBand12Gain);
			AddBinding(nameof(AudioConfig.Band13Gain), trkBand13Gain);
			AddBinding(nameof(AudioConfig.Band14Gain), trkBand14Gain);
			AddBinding(nameof(AudioConfig.Band15Gain), trkBand15Gain);
			AddBinding(nameof(AudioConfig.Band16Gain), trkBand16Gain);
			AddBinding(nameof(AudioConfig.Band17Gain), trkBand17Gain);
			AddBinding(nameof(AudioConfig.Band18Gain), trkBand18Gain);
			AddBinding(nameof(AudioConfig.Band19Gain), trkBand19Gain);
			AddBinding(nameof(AudioConfig.Band20Gain), trkBand20Gain);

			AddBinding(nameof(AudioConfig.ReduceSoundInBackground), chkReduceSoundInBackground);
			AddBinding(nameof(AudioConfig.MuteSoundInBackground), chkMuteSoundInBackground);
			AddBinding(nameof(AudioConfig.ReduceSoundInFastForward), chkReduceSoundInFastForward);
			AddBinding(nameof(AudioConfig.VolumeReduction), trkVolumeReduction);			
		}

		protected override bool ValidateInput()
		{
			UpdateLatencyWarning();
			UpdateObject();
			((AudioConfig)this.Entity).ApplyConfig();
			return true;
		}

		private void UpdateLatencyWarning()
		{
			picLatencyWarning.Visible = nudLatency.Value <= 55;
			lblLatencyWarning.Visible = nudLatency.Value <= 55;
		}

		protected override void OnApply()
		{
			ConfigManager.Config.Audio = (AudioConfig)this.Entity;
			ConfigManager.ApplyChanges();
		}

		private void chkEnableEqualizer_CheckedChanged(object sender, EventArgs e)
		{
			tlpEqualizer.Enabled = chkEnableEqualizer.Checked;
		}

		private void chkMuteWhenInBackground_CheckedChanged(object sender, EventArgs e)
		{
			UpdateVolumeOptions();
		}

		private void chkReduceVolume_CheckedChanged(object sender, EventArgs e)
		{
			UpdateVolumeOptions();
		}

		private void UpdateVolumeOptions()
		{
			chkReduceSoundInBackground.Enabled = !chkMuteSoundInBackground.Checked;
			trkVolumeReduction.Enabled = chkReduceSoundInFastForward.Checked || (chkReduceSoundInBackground.Checked && chkReduceSoundInBackground.Enabled);
		}
	}
}
