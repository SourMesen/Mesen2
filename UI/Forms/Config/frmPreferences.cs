using Mesen.GUI.Config;
using Mesen.GUI.Utilities;
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
	public partial class frmPreferences : BaseConfigForm
	{
		public bool NeedRestart { get; private set; }

		public frmPreferences()
		{
			InitializeComponent();
			if(DesignMode) {
				return;
			}

			Entity = ConfigManager.Config.Preferences.Clone();

			ctrlEmulatorShortcuts.InitializeGrid((PreferencesConfig)Entity);

			AddBinding(nameof(PreferencesConfig.DisplayLanguage), cboDisplayLanguage);

			AddBinding(nameof(PreferencesConfig.AutomaticallyCheckForUpdates), chkAutomaticallyCheckForUpdates);
			AddBinding(nameof(PreferencesConfig.SingleInstance), chkSingleInstance);
			AddBinding(nameof(PreferencesConfig.AutoLoadPatches), chkAutoLoadPatches);

			AddBinding(nameof(PreferencesConfig.PauseWhenInBackground), chkPauseWhenInBackground);
			AddBinding(nameof(PreferencesConfig.PauseWhenInMenusAndConfig), chkPauseInMenuAndConfig);
			AddBinding(nameof(PreferencesConfig.PauseWhenInDebuggingTools), chkPauseInDebugger);
			AddBinding(nameof(PreferencesConfig.AllowBackgroundInput), chkAllowBackgroundInput);
			AddBinding(nameof(PreferencesConfig.PauseOnMovieEnd), chkPauseOnMovieEnd);

			AddBinding(nameof(PreferencesConfig.AssociateRomFiles), chkRomFormat);
			AddBinding(nameof(PreferencesConfig.AssociateMsmFiles), chkMsmFormat);
			AddBinding(nameof(PreferencesConfig.AssociateMssFiles), chkMssFormat);
			AddBinding(nameof(PreferencesConfig.AssociateSpcFiles), chkSpcFormat);
			AddBinding(nameof(PreferencesConfig.AssociateBsFiles), chkBsFormat);

			AddBinding(nameof(PreferencesConfig.AlwaysOnTop), chkAlwaysOnTop);
			AddBinding(nameof(PreferencesConfig.AutoHideMenu), chkAutoHideMenu);

			AddBinding(nameof(PreferencesConfig.ShowTitleBarInfo), chkDisplayTitleBarInfo);
			AddBinding(nameof(PreferencesConfig.DisableOsd), chkDisableOsd);
			AddBinding(nameof(PreferencesConfig.DisableGameSelectionScreen), chkDisableGameSelectionScreen);
			AddBinding(nameof(PreferencesConfig.ShowFps), chkShowFps);
			AddBinding(nameof(PreferencesConfig.ShowFrameCounter), chkShowFrameCounter);
			AddBinding(nameof(PreferencesConfig.ShowGameTimer), chkShowGameTimer);
			AddBinding(nameof(PreferencesConfig.ShowDebugInfo), chkShowDebugInfo);
			AddBinding(nameof(PreferencesConfig.RewindBufferSize), nudRewindBufferSize);

			AddBinding(nameof(PreferencesConfig.GameFolder), psGame);
			AddBinding(nameof(PreferencesConfig.AviFolder), psAvi);
			AddBinding(nameof(PreferencesConfig.MovieFolder), psMovies);
			AddBinding(nameof(PreferencesConfig.SaveDataFolder), psSaveData);
			AddBinding(nameof(PreferencesConfig.SaveStateFolder), psSaveStates);
			AddBinding(nameof(PreferencesConfig.ScreenshotFolder), psScreenshots);
			AddBinding(nameof(PreferencesConfig.WaveFolder), psWave);

			AddBinding(nameof(PreferencesConfig.OverrideGameFolder), chkGameOverride);
			AddBinding(nameof(PreferencesConfig.OverrideAviFolder), chkAviOverride);
			AddBinding(nameof(PreferencesConfig.OverrideMovieFolder), chkMoviesOverride);
			AddBinding(nameof(PreferencesConfig.OverrideSaveDataFolder), chkSaveDataOverride);
			AddBinding(nameof(PreferencesConfig.OverrideSaveStateFolder), chkSaveStatesOverride);
			AddBinding(nameof(PreferencesConfig.OverrideScreenshotFolder), chkScreenshotsOverride);
			AddBinding(nameof(PreferencesConfig.OverrideWaveFolder), chkWaveOverride);

			radStorageDocuments.Checked = ConfigManager.HomeFolder == ConfigManager.DefaultDocumentsFolder;
			radStoragePortable.Checked = !radStorageDocuments.Checked;

			UpdateLocationText();
			UpdateFolderOverrideUi();
		}

		protected override void OnApply()
		{
			ctrlEmulatorShortcuts.UpdateConfig();

			ConfigManager.Config.Preferences = (PreferencesConfig)this.Entity;
			ConfigManager.ApplyChanges();
		}

		private void btnOpenMesenFolder_Click(object sender, EventArgs e)
		{
			System.Diagnostics.Process.Start(ConfigManager.HomeFolder);
		}

		private void btnResetSettings_Click(object sender, EventArgs e)
		{
			if(MesenMsgBox.Show("ResetSettingsConfirmation", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning) == DialogResult.OK) {
				ConfigManager.ResetSettings();
				this.Close();
			}
		}

		private void UpdateFolderOverrideUi()
		{
			psGame.Enabled = chkGameOverride.Checked;
			psAvi.Enabled = chkAviOverride.Checked;
			psMovies.Enabled = chkMoviesOverride.Checked;
			psSaveData.Enabled = chkSaveDataOverride.Checked;
			psSaveStates.Enabled = chkSaveStatesOverride.Checked;
			psScreenshots.Enabled = chkScreenshotsOverride.Checked;
			psWave.Enabled = chkWaveOverride.Checked;

			psGame.DisabledText = ResourceHelper.GetMessage("LastFolderUsed");
			psAvi.DisabledText = ConfigManager.DefaultAviFolder;
			psMovies.DisabledText = ConfigManager.DefaultMovieFolder;
			psSaveData.DisabledText = ConfigManager.DefaultSaveDataFolder;
			psSaveStates.DisabledText = ConfigManager.DefaultSaveStateFolder;
			psScreenshots.DisabledText = ConfigManager.DefaultScreenshotFolder;
			psWave.DisabledText = ConfigManager.DefaultWaveFolder;
		}

		private void UpdateLocationText()
		{
			lblLocation.Text = radStorageDocuments.Checked ? ConfigManager.DefaultDocumentsFolder : ConfigManager.DefaultPortableFolder;
		}

		private void chkOverride_CheckedChanged(object sender, EventArgs e)
		{
			UpdateFolderOverrideUi();
		}

		private void radStorageDocuments_CheckedChanged(object sender, EventArgs e)
		{
			UpdateLocationText();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			if(this.DialogResult == DialogResult.OK) {
				if(!ValidateFolderSettings()) {
					e.Cancel = true;
					return;
				}

				if(radStorageDocuments.Checked != (ConfigManager.HomeFolder == ConfigManager.DefaultDocumentsFolder)) {
					//Need to copy files and display confirmation
					string targetFolder = radStorageDocuments.Checked ? ConfigManager.DefaultDocumentsFolder : ConfigManager.DefaultPortableFolder;
					if(MesenMsgBox.Show("CopyMesenDataPrompt", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, ConfigManager.HomeFolder, targetFolder) == DialogResult.OK) {
						try {
							//Update and save the config before migrating the data
							UpdateObject();
							OnApply();
							ConfigManager.ApplyChanges();
							ConfigManager.SaveConfig();

							if(FolderHelper.MigrateData(ConfigManager.HomeFolder, targetFolder, this)) {
								NeedRestart = true;
							}
						} catch(Exception ex) {
							MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
							e.Cancel = true;
						}
					} else {
						e.Cancel = true;
						return;
					}
				}
			} else {
				base.OnFormClosing(e);
			}
		}

		private bool ValidateFolderSettings()
		{
			bool result = true;
			List<string> invalidFolders = new List<string>();
			try {
				if(chkGameOverride.Checked && !FolderHelper.CheckFolderPermissions(psGame.Text, false)) {
					invalidFolders.Add(chkGameOverride.Text.Replace(":", "").Trim());
				}
				if(chkAviOverride.Checked && !FolderHelper.CheckFolderPermissions(psAvi.Text)) {
					invalidFolders.Add(chkAviOverride.Text.Replace(":", "").Trim());
				}
				if(chkMoviesOverride.Checked && !FolderHelper.CheckFolderPermissions(psMovies.Text)) {
					invalidFolders.Add(chkMoviesOverride.Text.Replace(":", "").Trim());
				}
				if(chkSaveDataOverride.Checked && !FolderHelper.CheckFolderPermissions(psSaveData.Text)) {
					invalidFolders.Add(chkSaveDataOverride.Text.Replace(":", "").Trim());
				}
				if(chkSaveStatesOverride.Checked && !FolderHelper.CheckFolderPermissions(psSaveStates.Text)) {
					invalidFolders.Add(chkSaveStatesOverride.Text.Replace(":", "").Trim());
				}
				if(chkScreenshotsOverride.Checked && !FolderHelper.CheckFolderPermissions(psScreenshots.Text)) {
					invalidFolders.Add(chkScreenshotsOverride.Text.Replace(":", "").Trim());
				}
				if(chkWaveOverride.Checked && !FolderHelper.CheckFolderPermissions(psWave.Text)) {
					invalidFolders.Add(chkWaveOverride.Text.Replace(":", "").Trim());
				}

				result = invalidFolders.Count == 0;
			} catch {
				result = false;
			}
			if(!result) {
				MesenMsgBox.Show("InvalidPaths", MessageBoxButtons.OK, MessageBoxIcon.Error, string.Join(Environment.NewLine, invalidFolders));
			}
			return result;
		}
	}
}
