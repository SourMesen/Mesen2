namespace Mesen.GUI.Forms
{
	partial class frmMain
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.ctrlRenderer = new Mesen.GUI.Controls.ctrlRenderer();
			this.mnuMain = new Mesen.GUI.Controls.ctrlMesenMenuStrip();
			this.mnuFile = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuOpen = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuReloadRom = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSaveState = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuLoadState = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem10 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRecentFiles = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuExit = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGame = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPause = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuReset = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPowerCycle = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem24 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuPowerOff = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuOptions = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmulationSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmuSpeedNormal = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuIncreaseSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDecreaseSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmuSpeedMaximumSpeed = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem9 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuEmuSpeedTriple = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmuSpeedDouble = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmuSpeedHalf = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmuSpeedQuarter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem14 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuShowFPS = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuVideoScale = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale1x = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale2x = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale3x = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale4x = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale5x = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale6x = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem13 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuFullscreen = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuVideoFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNoneFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem21 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuNtscFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem15 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuXBRZ2xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuXBRZ3xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuXBRZ4xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuXBRZ5xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuXBRZ6xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem16 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuHQ2xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHQ3xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHQ4xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem17 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuScale2xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale3xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScale4xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem23 = new System.Windows.Forms.ToolStripSeparator();
			this.mnu2xSaiFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSuper2xSaiFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSuperEagleFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem18 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuPrescale2xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPrescale3xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPrescale4xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPrescale6xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPrescale8xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPrescale10xFilter = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem19 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuBilinearInterpolation = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuBlendHighResolutionModes = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRegion = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRegionAuto = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRegionNtsc = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRegionPal = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAudioConfig = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuInputConfig = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuVideoConfig = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEmulationConfig = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuPreferences = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTools = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuCheats = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMovies = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPlayMovie = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRecordMovie = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStopMovie = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlay = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuStartServer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuConnect = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlaySelectController = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlayPlayer1 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlayPlayer2 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlayPlayer3 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlayPlayer4 = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNetPlayPlayer5 = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuNetPlaySpectator = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuProfile = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem25 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSoundRecorder = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuWaveRecord = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuWaveStop = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuVideoRecorder = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAviRecord = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuAviStop = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem11 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuTests = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTestRun = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTestRecord = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTestStop = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRunAllTests = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuLogWindow = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuRandomGame = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTakeScreenshot = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDebug = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuDebugger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuEventViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuMemoryTools = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuRegisterViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTraceLogger = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem26 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAssembler = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuProfiler = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuScriptWindow = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem12 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuTilemapViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuTileViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSpriteViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuPaletteViewer = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem22 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuSpcDebugger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuSa1Debugger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuGsuDebugger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuNecDspDebugger = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuHelp = new System.Windows.Forms.ToolStripMenuItem();
			this.mnuCheckForUpdates = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem20 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuReportBug = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
			this.mnuAbout = new System.Windows.Forms.ToolStripMenuItem();
			this.pnlRenderer = new System.Windows.Forms.Panel();
			this.ctrlRecentGames = new Mesen.GUI.Controls.ctrlRecentGames();
			this.mnuMain.SuspendLayout();
			this.pnlRenderer.SuspendLayout();
			this.SuspendLayout();
			// 
			// ctrlRenderer
			// 
			this.ctrlRenderer.Location = new System.Drawing.Point(0, 0);
			this.ctrlRenderer.Name = "ctrlRenderer";
			this.ctrlRenderer.Size = new System.Drawing.Size(512, 478);
			this.ctrlRenderer.TabIndex = 0;
			// 
			// mnuMain
			// 
			this.mnuMain.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuFile,
            this.mnuGame,
            this.mnuOptions,
            this.mnuTools,
            this.mnuDebug,
            this.mnuHelp});
			this.mnuMain.Location = new System.Drawing.Point(0, 0);
			this.mnuMain.Name = "mnuMain";
			this.mnuMain.Size = new System.Drawing.Size(512, 24);
			this.mnuMain.TabIndex = 1;
			this.mnuMain.Text = "ctrlMesenMenuStrip1";
			// 
			// mnuFile
			// 
			this.mnuFile.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuOpen,
            this.mnuReloadRom,
            this.toolStripMenuItem2,
            this.mnuSaveState,
            this.mnuLoadState,
            this.toolStripMenuItem10,
            this.mnuRecentFiles,
            this.toolStripMenuItem6,
            this.mnuExit});
			this.mnuFile.Name = "mnuFile";
			this.mnuFile.Size = new System.Drawing.Size(37, 20);
			this.mnuFile.Text = "File";
			this.mnuFile.DropDownClosed += new System.EventHandler(this.mnu_DropDownClosed);
			this.mnuFile.DropDownOpening += new System.EventHandler(this.mnuFile_DropDownOpening);
			this.mnuFile.DropDownOpened += new System.EventHandler(this.mnu_DropDownOpened);
			// 
			// mnuOpen
			// 
			this.mnuOpen.Image = global::Mesen.GUI.Properties.Resources.Folder;
			this.mnuOpen.Name = "mnuOpen";
			this.mnuOpen.Size = new System.Drawing.Size(140, 22);
			this.mnuOpen.Text = "Open";
			// 
			// mnuReloadRom
			// 
			this.mnuReloadRom.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuReloadRom.Name = "mnuReloadRom";
			this.mnuReloadRom.Size = new System.Drawing.Size(140, 22);
			this.mnuReloadRom.Text = "Reload ROM";
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(137, 6);
			// 
			// mnuSaveState
			// 
			this.mnuSaveState.Name = "mnuSaveState";
			this.mnuSaveState.Size = new System.Drawing.Size(140, 22);
			this.mnuSaveState.Text = "Save State";
			this.mnuSaveState.DropDownOpening += new System.EventHandler(this.mnuSaveState_DropDownOpening);
			// 
			// mnuLoadState
			// 
			this.mnuLoadState.Name = "mnuLoadState";
			this.mnuLoadState.Size = new System.Drawing.Size(140, 22);
			this.mnuLoadState.Text = "Load State";
			this.mnuLoadState.DropDownOpening += new System.EventHandler(this.mnuLoadState_DropDownOpening);
			// 
			// toolStripMenuItem10
			// 
			this.toolStripMenuItem10.Name = "toolStripMenuItem10";
			this.toolStripMenuItem10.Size = new System.Drawing.Size(137, 6);
			// 
			// mnuRecentFiles
			// 
			this.mnuRecentFiles.Name = "mnuRecentFiles";
			this.mnuRecentFiles.Size = new System.Drawing.Size(140, 22);
			this.mnuRecentFiles.Text = "Recent Files";
			// 
			// toolStripMenuItem6
			// 
			this.toolStripMenuItem6.Name = "toolStripMenuItem6";
			this.toolStripMenuItem6.Size = new System.Drawing.Size(137, 6);
			// 
			// mnuExit
			// 
			this.mnuExit.Image = global::Mesen.GUI.Properties.Resources.Exit;
			this.mnuExit.Name = "mnuExit";
			this.mnuExit.Size = new System.Drawing.Size(140, 22);
			this.mnuExit.Text = "Exit";
			// 
			// mnuGame
			// 
			this.mnuGame.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuPause,
            this.mnuReset,
            this.mnuPowerCycle,
            this.toolStripMenuItem24,
            this.mnuPowerOff});
			this.mnuGame.Name = "mnuGame";
			this.mnuGame.Size = new System.Drawing.Size(50, 20);
			this.mnuGame.Text = "Game";
			this.mnuGame.DropDownClosed += new System.EventHandler(this.mnu_DropDownClosed);
			this.mnuGame.DropDownOpened += new System.EventHandler(this.mnu_DropDownOpened);
			// 
			// mnuPause
			// 
			this.mnuPause.Enabled = false;
			this.mnuPause.Image = global::Mesen.GUI.Properties.Resources.MediaPause;
			this.mnuPause.Name = "mnuPause";
			this.mnuPause.Size = new System.Drawing.Size(139, 22);
			this.mnuPause.Text = "Pause";
			// 
			// mnuReset
			// 
			this.mnuReset.Enabled = false;
			this.mnuReset.Image = global::Mesen.GUI.Properties.Resources.Refresh;
			this.mnuReset.Name = "mnuReset";
			this.mnuReset.Size = new System.Drawing.Size(139, 22);
			this.mnuReset.Text = "Reset";
			// 
			// mnuPowerCycle
			// 
			this.mnuPowerCycle.Enabled = false;
			this.mnuPowerCycle.Image = global::Mesen.GUI.Properties.Resources.PowerCycle;
			this.mnuPowerCycle.Name = "mnuPowerCycle";
			this.mnuPowerCycle.Size = new System.Drawing.Size(139, 22);
			this.mnuPowerCycle.Text = "Power Cycle";
			// 
			// toolStripMenuItem24
			// 
			this.toolStripMenuItem24.Name = "toolStripMenuItem24";
			this.toolStripMenuItem24.Size = new System.Drawing.Size(136, 6);
			// 
			// mnuPowerOff
			// 
			this.mnuPowerOff.Image = global::Mesen.GUI.Properties.Resources.MediaStop;
			this.mnuPowerOff.Name = "mnuPowerOff";
			this.mnuPowerOff.Size = new System.Drawing.Size(139, 22);
			this.mnuPowerOff.Text = "Power Off";
			// 
			// mnuOptions
			// 
			this.mnuOptions.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuEmulationSpeed,
            this.mnuVideoScale,
            this.mnuVideoFilter,
            this.mnuRegion,
            this.toolStripMenuItem4,
            this.mnuAudioConfig,
            this.mnuInputConfig,
            this.mnuVideoConfig,
            this.mnuEmulationConfig,
            this.toolStripMenuItem3,
            this.mnuPreferences});
			this.mnuOptions.Name = "mnuOptions";
			this.mnuOptions.Size = new System.Drawing.Size(61, 20);
			this.mnuOptions.Text = "Options";
			this.mnuOptions.DropDownClosed += new System.EventHandler(this.mnu_DropDownClosed);
			this.mnuOptions.DropDownOpened += new System.EventHandler(this.mnu_DropDownOpened);
			// 
			// mnuEmulationSpeed
			// 
			this.mnuEmulationSpeed.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuEmuSpeedNormal,
            this.toolStripMenuItem8,
            this.mnuIncreaseSpeed,
            this.mnuDecreaseSpeed,
            this.mnuEmuSpeedMaximumSpeed,
            this.toolStripMenuItem9,
            this.mnuEmuSpeedTriple,
            this.mnuEmuSpeedDouble,
            this.mnuEmuSpeedHalf,
            this.mnuEmuSpeedQuarter,
            this.toolStripMenuItem14,
            this.mnuShowFPS});
			this.mnuEmulationSpeed.Image = global::Mesen.GUI.Properties.Resources.Speed;
			this.mnuEmulationSpeed.Name = "mnuEmulationSpeed";
			this.mnuEmulationSpeed.Size = new System.Drawing.Size(135, 22);
			this.mnuEmulationSpeed.Text = "Speed";
			this.mnuEmulationSpeed.DropDownOpening += new System.EventHandler(this.mnuEmulationSpeed_DropDownOpening);
			// 
			// mnuEmuSpeedNormal
			// 
			this.mnuEmuSpeedNormal.Name = "mnuEmuSpeedNormal";
			this.mnuEmuSpeedNormal.Size = new System.Drawing.Size(163, 22);
			this.mnuEmuSpeedNormal.Text = "Normal (100%)";
			// 
			// toolStripMenuItem8
			// 
			this.toolStripMenuItem8.Name = "toolStripMenuItem8";
			this.toolStripMenuItem8.Size = new System.Drawing.Size(160, 6);
			// 
			// mnuIncreaseSpeed
			// 
			this.mnuIncreaseSpeed.Name = "mnuIncreaseSpeed";
			this.mnuIncreaseSpeed.Size = new System.Drawing.Size(163, 22);
			this.mnuIncreaseSpeed.Text = "Increase Speed";
			// 
			// mnuDecreaseSpeed
			// 
			this.mnuDecreaseSpeed.Name = "mnuDecreaseSpeed";
			this.mnuDecreaseSpeed.Size = new System.Drawing.Size(163, 22);
			this.mnuDecreaseSpeed.Text = "Decrease Speed";
			// 
			// mnuEmuSpeedMaximumSpeed
			// 
			this.mnuEmuSpeedMaximumSpeed.Name = "mnuEmuSpeedMaximumSpeed";
			this.mnuEmuSpeedMaximumSpeed.Size = new System.Drawing.Size(163, 22);
			this.mnuEmuSpeedMaximumSpeed.Text = "Maximum Speed";
			// 
			// toolStripMenuItem9
			// 
			this.toolStripMenuItem9.Name = "toolStripMenuItem9";
			this.toolStripMenuItem9.Size = new System.Drawing.Size(160, 6);
			// 
			// mnuEmuSpeedTriple
			// 
			this.mnuEmuSpeedTriple.Name = "mnuEmuSpeedTriple";
			this.mnuEmuSpeedTriple.Size = new System.Drawing.Size(163, 22);
			this.mnuEmuSpeedTriple.Tag = "";
			this.mnuEmuSpeedTriple.Text = "Triple (300%)";
			// 
			// mnuEmuSpeedDouble
			// 
			this.mnuEmuSpeedDouble.Name = "mnuEmuSpeedDouble";
			this.mnuEmuSpeedDouble.Size = new System.Drawing.Size(163, 22);
			this.mnuEmuSpeedDouble.Text = "Double (200%)";
			// 
			// mnuEmuSpeedHalf
			// 
			this.mnuEmuSpeedHalf.Name = "mnuEmuSpeedHalf";
			this.mnuEmuSpeedHalf.Size = new System.Drawing.Size(163, 22);
			this.mnuEmuSpeedHalf.Text = "Half (50%)";
			// 
			// mnuEmuSpeedQuarter
			// 
			this.mnuEmuSpeedQuarter.Name = "mnuEmuSpeedQuarter";
			this.mnuEmuSpeedQuarter.Size = new System.Drawing.Size(163, 22);
			this.mnuEmuSpeedQuarter.Text = "Quarter (25%)";
			// 
			// toolStripMenuItem14
			// 
			this.toolStripMenuItem14.Name = "toolStripMenuItem14";
			this.toolStripMenuItem14.Size = new System.Drawing.Size(160, 6);
			// 
			// mnuShowFPS
			// 
			this.mnuShowFPS.Name = "mnuShowFPS";
			this.mnuShowFPS.Size = new System.Drawing.Size(163, 22);
			this.mnuShowFPS.Text = "Show FPS";
			// 
			// mnuVideoScale
			// 
			this.mnuVideoScale.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuScale1x,
            this.mnuScale2x,
            this.mnuScale3x,
            this.mnuScale4x,
            this.mnuScale5x,
            this.mnuScale6x,
            this.toolStripMenuItem13,
            this.mnuFullscreen});
			this.mnuVideoScale.Image = global::Mesen.GUI.Properties.Resources.Fullscreen;
			this.mnuVideoScale.Name = "mnuVideoScale";
			this.mnuVideoScale.Size = new System.Drawing.Size(135, 22);
			this.mnuVideoScale.Text = "Video Size";
			this.mnuVideoScale.DropDownOpening += new System.EventHandler(this.mnuVideoScale_DropDownOpening);
			// 
			// mnuScale1x
			// 
			this.mnuScale1x.Name = "mnuScale1x";
			this.mnuScale1x.Size = new System.Drawing.Size(127, 22);
			this.mnuScale1x.Text = "1x";
			// 
			// mnuScale2x
			// 
			this.mnuScale2x.Name = "mnuScale2x";
			this.mnuScale2x.Size = new System.Drawing.Size(127, 22);
			this.mnuScale2x.Text = "2x";
			// 
			// mnuScale3x
			// 
			this.mnuScale3x.Name = "mnuScale3x";
			this.mnuScale3x.Size = new System.Drawing.Size(127, 22);
			this.mnuScale3x.Text = "3x";
			// 
			// mnuScale4x
			// 
			this.mnuScale4x.Name = "mnuScale4x";
			this.mnuScale4x.Size = new System.Drawing.Size(127, 22);
			this.mnuScale4x.Text = "4x";
			// 
			// mnuScale5x
			// 
			this.mnuScale5x.Name = "mnuScale5x";
			this.mnuScale5x.Size = new System.Drawing.Size(127, 22);
			this.mnuScale5x.Text = "5x";
			// 
			// mnuScale6x
			// 
			this.mnuScale6x.Name = "mnuScale6x";
			this.mnuScale6x.Size = new System.Drawing.Size(127, 22);
			this.mnuScale6x.Text = "6x";
			// 
			// toolStripMenuItem13
			// 
			this.toolStripMenuItem13.Name = "toolStripMenuItem13";
			this.toolStripMenuItem13.Size = new System.Drawing.Size(124, 6);
			// 
			// mnuFullscreen
			// 
			this.mnuFullscreen.Name = "mnuFullscreen";
			this.mnuFullscreen.Size = new System.Drawing.Size(127, 22);
			this.mnuFullscreen.Text = "Fullscreen";
			// 
			// mnuVideoFilter
			// 
			this.mnuVideoFilter.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuNoneFilter,
            this.toolStripMenuItem21,
            this.mnuNtscFilter,
            this.toolStripMenuItem15,
            this.mnuXBRZ2xFilter,
            this.mnuXBRZ3xFilter,
            this.mnuXBRZ4xFilter,
            this.mnuXBRZ5xFilter,
            this.mnuXBRZ6xFilter,
            this.toolStripMenuItem16,
            this.mnuHQ2xFilter,
            this.mnuHQ3xFilter,
            this.mnuHQ4xFilter,
            this.toolStripMenuItem17,
            this.mnuScale2xFilter,
            this.mnuScale3xFilter,
            this.mnuScale4xFilter,
            this.toolStripMenuItem23,
            this.mnu2xSaiFilter,
            this.mnuSuper2xSaiFilter,
            this.mnuSuperEagleFilter,
            this.toolStripMenuItem18,
            this.mnuPrescale2xFilter,
            this.mnuPrescale3xFilter,
            this.mnuPrescale4xFilter,
            this.mnuPrescale6xFilter,
            this.mnuPrescale8xFilter,
            this.mnuPrescale10xFilter,
            this.toolStripMenuItem19,
            this.mnuBilinearInterpolation,
            this.mnuBlendHighResolutionModes});
			this.mnuVideoFilter.Image = global::Mesen.GUI.Properties.Resources.VideoFilter;
			this.mnuVideoFilter.Name = "mnuVideoFilter";
			this.mnuVideoFilter.Size = new System.Drawing.Size(135, 22);
			this.mnuVideoFilter.Text = "Video Filter";
			this.mnuVideoFilter.DropDownOpening += new System.EventHandler(this.mnuVideoFilter_DropDownOpening);
			// 
			// mnuNoneFilter
			// 
			this.mnuNoneFilter.Name = "mnuNoneFilter";
			this.mnuNoneFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuNoneFilter.Text = "None";
			// 
			// toolStripMenuItem21
			// 
			this.toolStripMenuItem21.Name = "toolStripMenuItem21";
			this.toolStripMenuItem21.Size = new System.Drawing.Size(228, 6);
			// 
			// mnuNtscFilter
			// 
			this.mnuNtscFilter.Name = "mnuNtscFilter";
			this.mnuNtscFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuNtscFilter.Text = "NTSC";
			// 
			// toolStripMenuItem15
			// 
			this.toolStripMenuItem15.Name = "toolStripMenuItem15";
			this.toolStripMenuItem15.Size = new System.Drawing.Size(228, 6);
			// 
			// mnuXBRZ2xFilter
			// 
			this.mnuXBRZ2xFilter.Name = "mnuXBRZ2xFilter";
			this.mnuXBRZ2xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuXBRZ2xFilter.Text = "xBRZ 2x";
			// 
			// mnuXBRZ3xFilter
			// 
			this.mnuXBRZ3xFilter.Name = "mnuXBRZ3xFilter";
			this.mnuXBRZ3xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuXBRZ3xFilter.Text = "xBRZ 3x";
			// 
			// mnuXBRZ4xFilter
			// 
			this.mnuXBRZ4xFilter.Name = "mnuXBRZ4xFilter";
			this.mnuXBRZ4xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuXBRZ4xFilter.Text = "xBRZ 4x";
			// 
			// mnuXBRZ5xFilter
			// 
			this.mnuXBRZ5xFilter.Name = "mnuXBRZ5xFilter";
			this.mnuXBRZ5xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuXBRZ5xFilter.Text = "xBRZ 5x";
			// 
			// mnuXBRZ6xFilter
			// 
			this.mnuXBRZ6xFilter.Name = "mnuXBRZ6xFilter";
			this.mnuXBRZ6xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuXBRZ6xFilter.Text = "xBRZ 6x";
			// 
			// toolStripMenuItem16
			// 
			this.toolStripMenuItem16.Name = "toolStripMenuItem16";
			this.toolStripMenuItem16.Size = new System.Drawing.Size(228, 6);
			// 
			// mnuHQ2xFilter
			// 
			this.mnuHQ2xFilter.Name = "mnuHQ2xFilter";
			this.mnuHQ2xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuHQ2xFilter.Text = "HQ 2x";
			// 
			// mnuHQ3xFilter
			// 
			this.mnuHQ3xFilter.Name = "mnuHQ3xFilter";
			this.mnuHQ3xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuHQ3xFilter.Text = "HQ 3x";
			// 
			// mnuHQ4xFilter
			// 
			this.mnuHQ4xFilter.Name = "mnuHQ4xFilter";
			this.mnuHQ4xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuHQ4xFilter.Text = "HQ 4x";
			// 
			// toolStripMenuItem17
			// 
			this.toolStripMenuItem17.Name = "toolStripMenuItem17";
			this.toolStripMenuItem17.Size = new System.Drawing.Size(228, 6);
			// 
			// mnuScale2xFilter
			// 
			this.mnuScale2xFilter.Name = "mnuScale2xFilter";
			this.mnuScale2xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuScale2xFilter.Text = "Scale2x";
			// 
			// mnuScale3xFilter
			// 
			this.mnuScale3xFilter.Name = "mnuScale3xFilter";
			this.mnuScale3xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuScale3xFilter.Text = "Scale3x";
			// 
			// mnuScale4xFilter
			// 
			this.mnuScale4xFilter.Name = "mnuScale4xFilter";
			this.mnuScale4xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuScale4xFilter.Text = "Scale4x";
			// 
			// toolStripMenuItem23
			// 
			this.toolStripMenuItem23.Name = "toolStripMenuItem23";
			this.toolStripMenuItem23.Size = new System.Drawing.Size(228, 6);
			// 
			// mnu2xSaiFilter
			// 
			this.mnu2xSaiFilter.Name = "mnu2xSaiFilter";
			this.mnu2xSaiFilter.Size = new System.Drawing.Size(231, 22);
			this.mnu2xSaiFilter.Text = "2xSai";
			// 
			// mnuSuper2xSaiFilter
			// 
			this.mnuSuper2xSaiFilter.Name = "mnuSuper2xSaiFilter";
			this.mnuSuper2xSaiFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuSuper2xSaiFilter.Text = "Super2xSai";
			// 
			// mnuSuperEagleFilter
			// 
			this.mnuSuperEagleFilter.Name = "mnuSuperEagleFilter";
			this.mnuSuperEagleFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuSuperEagleFilter.Text = "SuperEagle";
			// 
			// toolStripMenuItem18
			// 
			this.toolStripMenuItem18.Name = "toolStripMenuItem18";
			this.toolStripMenuItem18.Size = new System.Drawing.Size(228, 6);
			// 
			// mnuPrescale2xFilter
			// 
			this.mnuPrescale2xFilter.Name = "mnuPrescale2xFilter";
			this.mnuPrescale2xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuPrescale2xFilter.Text = "Prescale 2x";
			// 
			// mnuPrescale3xFilter
			// 
			this.mnuPrescale3xFilter.Name = "mnuPrescale3xFilter";
			this.mnuPrescale3xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuPrescale3xFilter.Text = "Prescale 3x";
			// 
			// mnuPrescale4xFilter
			// 
			this.mnuPrescale4xFilter.Name = "mnuPrescale4xFilter";
			this.mnuPrescale4xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuPrescale4xFilter.Text = "Prescale 4x";
			// 
			// mnuPrescale6xFilter
			// 
			this.mnuPrescale6xFilter.Name = "mnuPrescale6xFilter";
			this.mnuPrescale6xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuPrescale6xFilter.Text = "Prescale 6x";
			// 
			// mnuPrescale8xFilter
			// 
			this.mnuPrescale8xFilter.Name = "mnuPrescale8xFilter";
			this.mnuPrescale8xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuPrescale8xFilter.Text = "Prescale 8x";
			// 
			// mnuPrescale10xFilter
			// 
			this.mnuPrescale10xFilter.Name = "mnuPrescale10xFilter";
			this.mnuPrescale10xFilter.Size = new System.Drawing.Size(231, 22);
			this.mnuPrescale10xFilter.Text = "Prescale 10x";
			// 
			// toolStripMenuItem19
			// 
			this.toolStripMenuItem19.Name = "toolStripMenuItem19";
			this.toolStripMenuItem19.Size = new System.Drawing.Size(228, 6);
			// 
			// mnuBilinearInterpolation
			// 
			this.mnuBilinearInterpolation.CheckOnClick = true;
			this.mnuBilinearInterpolation.Name = "mnuBilinearInterpolation";
			this.mnuBilinearInterpolation.Size = new System.Drawing.Size(231, 22);
			this.mnuBilinearInterpolation.Text = "Use Bilinear Interpolation";
			// 
			// mnuBlendHighResolutionModes
			// 
			this.mnuBlendHighResolutionModes.CheckOnClick = true;
			this.mnuBlendHighResolutionModes.Name = "mnuBlendHighResolutionModes";
			this.mnuBlendHighResolutionModes.Size = new System.Drawing.Size(231, 22);
			this.mnuBlendHighResolutionModes.Text = "Blend High Resolution Modes";
			// 
			// mnuRegion
			// 
			this.mnuRegion.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuRegionAuto,
            this.toolStripMenuItem1,
            this.mnuRegionNtsc,
            this.mnuRegionPal});
			this.mnuRegion.Image = global::Mesen.GUI.Properties.Resources.WebBrowser;
			this.mnuRegion.Name = "mnuRegion";
			this.mnuRegion.Size = new System.Drawing.Size(135, 22);
			this.mnuRegion.Text = "Region";
			this.mnuRegion.DropDownOpening += new System.EventHandler(this.mnuRegion_DropDownOpening);
			// 
			// mnuRegionAuto
			// 
			this.mnuRegionAuto.Name = "mnuRegionAuto";
			this.mnuRegionAuto.Size = new System.Drawing.Size(104, 22);
			this.mnuRegionAuto.Text = "Auto";
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(101, 6);
			// 
			// mnuRegionNtsc
			// 
			this.mnuRegionNtsc.Name = "mnuRegionNtsc";
			this.mnuRegionNtsc.Size = new System.Drawing.Size(104, 22);
			this.mnuRegionNtsc.Text = "NTSC";
			// 
			// mnuRegionPal
			// 
			this.mnuRegionPal.Name = "mnuRegionPal";
			this.mnuRegionPal.Size = new System.Drawing.Size(104, 22);
			this.mnuRegionPal.Text = "PAL";
			// 
			// toolStripMenuItem4
			// 
			this.toolStripMenuItem4.Name = "toolStripMenuItem4";
			this.toolStripMenuItem4.Size = new System.Drawing.Size(132, 6);
			// 
			// mnuAudioConfig
			// 
			this.mnuAudioConfig.Image = global::Mesen.GUI.Properties.Resources.Audio;
			this.mnuAudioConfig.Name = "mnuAudioConfig";
			this.mnuAudioConfig.Size = new System.Drawing.Size(135, 22);
			this.mnuAudioConfig.Text = "Audio";
			this.mnuAudioConfig.Click += new System.EventHandler(this.mnuAudioConfig_Click);
			// 
			// mnuInputConfig
			// 
			this.mnuInputConfig.Image = global::Mesen.GUI.Properties.Resources.Controller;
			this.mnuInputConfig.Name = "mnuInputConfig";
			this.mnuInputConfig.Size = new System.Drawing.Size(135, 22);
			this.mnuInputConfig.Text = "Input";
			this.mnuInputConfig.Click += new System.EventHandler(this.mnuInputConfig_Click);
			// 
			// mnuVideoConfig
			// 
			this.mnuVideoConfig.Image = global::Mesen.GUI.Properties.Resources.VideoOptions;
			this.mnuVideoConfig.Name = "mnuVideoConfig";
			this.mnuVideoConfig.Size = new System.Drawing.Size(135, 22);
			this.mnuVideoConfig.Text = "Video";
			this.mnuVideoConfig.Click += new System.EventHandler(this.mnuVideoConfig_Click);
			// 
			// mnuEmulationConfig
			// 
			this.mnuEmulationConfig.Image = global::Mesen.GUI.Properties.Resources.DipSwitches;
			this.mnuEmulationConfig.Name = "mnuEmulationConfig";
			this.mnuEmulationConfig.Size = new System.Drawing.Size(135, 22);
			this.mnuEmulationConfig.Text = "Emulation";
			this.mnuEmulationConfig.Click += new System.EventHandler(this.mnuEmulationConfig_Click);
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(132, 6);
			// 
			// mnuPreferences
			// 
			this.mnuPreferences.Image = global::Mesen.GUI.Properties.Resources.Settings;
			this.mnuPreferences.Name = "mnuPreferences";
			this.mnuPreferences.Size = new System.Drawing.Size(135, 22);
			this.mnuPreferences.Text = "Preferences";
			this.mnuPreferences.Click += new System.EventHandler(this.mnuPreferences_Click);
			// 
			// mnuTools
			// 
			this.mnuTools.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuCheats,
            this.mnuMovies,
            this.mnuNetPlay,
            this.toolStripMenuItem25,
            this.mnuSoundRecorder,
            this.mnuVideoRecorder,
            this.toolStripMenuItem11,
            this.mnuTests,
            this.mnuLogWindow,
            this.toolStripMenuItem7,
            this.mnuRandomGame,
            this.mnuTakeScreenshot});
			this.mnuTools.Name = "mnuTools";
			this.mnuTools.Size = new System.Drawing.Size(47, 20);
			this.mnuTools.Text = "Tools";
			this.mnuTools.DropDownClosed += new System.EventHandler(this.mnu_DropDownClosed);
			this.mnuTools.DropDownOpening += new System.EventHandler(this.mnuTools_DropDownOpening);
			this.mnuTools.DropDownOpened += new System.EventHandler(this.mnu_DropDownOpened);
			// 
			// mnuCheats
			// 
			this.mnuCheats.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuCheats.Name = "mnuCheats";
			this.mnuCheats.Size = new System.Drawing.Size(182, 22);
			this.mnuCheats.Text = "Cheats";
			// 
			// mnuMovies
			// 
			this.mnuMovies.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuPlayMovie,
            this.mnuRecordMovie,
            this.mnuStopMovie});
			this.mnuMovies.Image = global::Mesen.GUI.Properties.Resources.Movie;
			this.mnuMovies.Name = "mnuMovies";
			this.mnuMovies.Size = new System.Drawing.Size(182, 22);
			this.mnuMovies.Text = "Movies";
			// 
			// mnuPlayMovie
			// 
			this.mnuPlayMovie.Image = global::Mesen.GUI.Properties.Resources.MediaPlay;
			this.mnuPlayMovie.Name = "mnuPlayMovie";
			this.mnuPlayMovie.Size = new System.Drawing.Size(120, 22);
			this.mnuPlayMovie.Text = "Play...";
			this.mnuPlayMovie.Click += new System.EventHandler(this.mnuPlayMovie_Click);
			// 
			// mnuRecordMovie
			// 
			this.mnuRecordMovie.Image = global::Mesen.GUI.Properties.Resources.Record;
			this.mnuRecordMovie.Name = "mnuRecordMovie";
			this.mnuRecordMovie.Size = new System.Drawing.Size(120, 22);
			this.mnuRecordMovie.Text = "Record...";
			this.mnuRecordMovie.Click += new System.EventHandler(this.mnuRecordMovie_Click);
			// 
			// mnuStopMovie
			// 
			this.mnuStopMovie.Image = global::Mesen.GUI.Properties.Resources.MediaStop;
			this.mnuStopMovie.Name = "mnuStopMovie";
			this.mnuStopMovie.Size = new System.Drawing.Size(120, 22);
			this.mnuStopMovie.Text = "Stop";
			this.mnuStopMovie.Click += new System.EventHandler(this.mnuStopMovie_Click);
			// 
			// mnuNetPlay
			// 
			this.mnuNetPlay.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuStartServer,
            this.mnuConnect,
            this.mnuNetPlaySelectController,
            this.toolStripSeparator2,
            this.mnuProfile});
			this.mnuNetPlay.Image = global::Mesen.GUI.Properties.Resources.Network;
			this.mnuNetPlay.Name = "mnuNetPlay";
			this.mnuNetPlay.Size = new System.Drawing.Size(182, 22);
			this.mnuNetPlay.Text = "Net Play";
			// 
			// mnuStartServer
			// 
			this.mnuStartServer.Name = "mnuStartServer";
			this.mnuStartServer.Size = new System.Drawing.Size(168, 22);
			this.mnuStartServer.Text = "Start Server";
			// 
			// mnuConnect
			// 
			this.mnuConnect.Name = "mnuConnect";
			this.mnuConnect.Size = new System.Drawing.Size(168, 22);
			this.mnuConnect.Text = "Connect to Server";
			// 
			// mnuNetPlaySelectController
			// 
			this.mnuNetPlaySelectController.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuNetPlayPlayer1,
            this.mnuNetPlayPlayer2,
            this.mnuNetPlayPlayer3,
            this.mnuNetPlayPlayer4,
            this.mnuNetPlayPlayer5,
            this.toolStripSeparator1,
            this.mnuNetPlaySpectator});
			this.mnuNetPlaySelectController.Name = "mnuNetPlaySelectController";
			this.mnuNetPlaySelectController.Size = new System.Drawing.Size(168, 22);
			this.mnuNetPlaySelectController.Text = "Select Controller";
			// 
			// mnuNetPlayPlayer1
			// 
			this.mnuNetPlayPlayer1.Name = "mnuNetPlayPlayer1";
			this.mnuNetPlayPlayer1.Size = new System.Drawing.Size(124, 22);
			this.mnuNetPlayPlayer1.Text = "Player 1";
			// 
			// mnuNetPlayPlayer2
			// 
			this.mnuNetPlayPlayer2.Name = "mnuNetPlayPlayer2";
			this.mnuNetPlayPlayer2.Size = new System.Drawing.Size(124, 22);
			this.mnuNetPlayPlayer2.Text = "Player 2";
			// 
			// mnuNetPlayPlayer3
			// 
			this.mnuNetPlayPlayer3.Name = "mnuNetPlayPlayer3";
			this.mnuNetPlayPlayer3.Size = new System.Drawing.Size(124, 22);
			this.mnuNetPlayPlayer3.Text = "Player 3";
			// 
			// mnuNetPlayPlayer4
			// 
			this.mnuNetPlayPlayer4.Name = "mnuNetPlayPlayer4";
			this.mnuNetPlayPlayer4.Size = new System.Drawing.Size(124, 22);
			this.mnuNetPlayPlayer4.Text = "Player 4";
			// 
			// mnuNetPlayPlayer5
			// 
			this.mnuNetPlayPlayer5.Name = "mnuNetPlayPlayer5";
			this.mnuNetPlayPlayer5.Size = new System.Drawing.Size(124, 22);
			this.mnuNetPlayPlayer5.Text = "Player 5";
			// 
			// toolStripSeparator1
			// 
			this.toolStripSeparator1.Name = "toolStripSeparator1";
			this.toolStripSeparator1.Size = new System.Drawing.Size(121, 6);
			// 
			// mnuNetPlaySpectator
			// 
			this.mnuNetPlaySpectator.Name = "mnuNetPlaySpectator";
			this.mnuNetPlaySpectator.Size = new System.Drawing.Size(124, 22);
			this.mnuNetPlaySpectator.Text = "Spectator";
			// 
			// toolStripSeparator2
			// 
			this.toolStripSeparator2.Name = "toolStripSeparator2";
			this.toolStripSeparator2.Size = new System.Drawing.Size(165, 6);
			// 
			// mnuProfile
			// 
			this.mnuProfile.Name = "mnuProfile";
			this.mnuProfile.Size = new System.Drawing.Size(168, 22);
			this.mnuProfile.Text = "Configure Profile";
			// 
			// toolStripMenuItem25
			// 
			this.toolStripMenuItem25.Name = "toolStripMenuItem25";
			this.toolStripMenuItem25.Size = new System.Drawing.Size(179, 6);
			// 
			// mnuSoundRecorder
			// 
			this.mnuSoundRecorder.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuWaveRecord,
            this.mnuWaveStop});
			this.mnuSoundRecorder.Image = global::Mesen.GUI.Properties.Resources.Microphone;
			this.mnuSoundRecorder.Name = "mnuSoundRecorder";
			this.mnuSoundRecorder.Size = new System.Drawing.Size(182, 22);
			this.mnuSoundRecorder.Text = "Sound Recorder";
			// 
			// mnuWaveRecord
			// 
			this.mnuWaveRecord.Image = global::Mesen.GUI.Properties.Resources.Record;
			this.mnuWaveRecord.Name = "mnuWaveRecord";
			this.mnuWaveRecord.Size = new System.Drawing.Size(155, 22);
			this.mnuWaveRecord.Text = "Record...";
			this.mnuWaveRecord.Click += new System.EventHandler(this.mnuWaveRecord_Click);
			// 
			// mnuWaveStop
			// 
			this.mnuWaveStop.Image = global::Mesen.GUI.Properties.Resources.MediaStop;
			this.mnuWaveStop.Name = "mnuWaveStop";
			this.mnuWaveStop.Size = new System.Drawing.Size(155, 22);
			this.mnuWaveStop.Text = "Stop Recording";
			this.mnuWaveStop.Click += new System.EventHandler(this.mnuWaveStop_Click);
			// 
			// mnuVideoRecorder
			// 
			this.mnuVideoRecorder.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuAviRecord,
            this.mnuAviStop});
			this.mnuVideoRecorder.Image = global::Mesen.GUI.Properties.Resources.VideoRecorder;
			this.mnuVideoRecorder.Name = "mnuVideoRecorder";
			this.mnuVideoRecorder.Size = new System.Drawing.Size(182, 22);
			this.mnuVideoRecorder.Text = "Video Recorder";
			// 
			// mnuAviRecord
			// 
			this.mnuAviRecord.Image = global::Mesen.GUI.Properties.Resources.Record;
			this.mnuAviRecord.Name = "mnuAviRecord";
			this.mnuAviRecord.Size = new System.Drawing.Size(155, 22);
			this.mnuAviRecord.Text = "Record...";
			this.mnuAviRecord.Click += new System.EventHandler(this.mnuAviRecord_Click);
			// 
			// mnuAviStop
			// 
			this.mnuAviStop.Image = global::Mesen.GUI.Properties.Resources.MediaStop;
			this.mnuAviStop.Name = "mnuAviStop";
			this.mnuAviStop.Size = new System.Drawing.Size(155, 22);
			this.mnuAviStop.Text = "Stop Recording";
			this.mnuAviStop.Click += new System.EventHandler(this.mnuAviStop_Click);
			// 
			// toolStripMenuItem11
			// 
			this.toolStripMenuItem11.Name = "toolStripMenuItem11";
			this.toolStripMenuItem11.Size = new System.Drawing.Size(179, 6);
			// 
			// mnuTests
			// 
			this.mnuTests.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuTestRun,
            this.mnuTestRecord,
            this.mnuTestStop,
            this.mnuRunAllTests});
			this.mnuTests.Name = "mnuTests";
			this.mnuTests.Size = new System.Drawing.Size(182, 22);
			this.mnuTests.Text = "Tests";
			// 
			// mnuTestRun
			// 
			this.mnuTestRun.Name = "mnuTestRun";
			this.mnuTestRun.Size = new System.Drawing.Size(152, 22);
			this.mnuTestRun.Text = "Run...";
			// 
			// mnuTestRecord
			// 
			this.mnuTestRecord.Name = "mnuTestRecord";
			this.mnuTestRecord.Size = new System.Drawing.Size(152, 22);
			this.mnuTestRecord.Text = "Record...";
			// 
			// mnuTestStop
			// 
			this.mnuTestStop.Name = "mnuTestStop";
			this.mnuTestStop.Size = new System.Drawing.Size(152, 22);
			this.mnuTestStop.Text = "Stop recording";
			// 
			// mnuRunAllTests
			// 
			this.mnuRunAllTests.Name = "mnuRunAllTests";
			this.mnuRunAllTests.Size = new System.Drawing.Size(152, 22);
			this.mnuRunAllTests.Text = "Run all tests";
			// 
			// mnuLogWindow
			// 
			this.mnuLogWindow.Image = global::Mesen.GUI.Properties.Resources.LogWindow;
			this.mnuLogWindow.Name = "mnuLogWindow";
			this.mnuLogWindow.Size = new System.Drawing.Size(182, 22);
			this.mnuLogWindow.Text = "Log Window";
			this.mnuLogWindow.Click += new System.EventHandler(this.mnuLogWindow_Click);
			// 
			// toolStripMenuItem7
			// 
			this.toolStripMenuItem7.Name = "toolStripMenuItem7";
			this.toolStripMenuItem7.Size = new System.Drawing.Size(179, 6);
			// 
			// mnuRandomGame
			// 
			this.mnuRandomGame.Image = global::Mesen.GUI.Properties.Resources.Dice;
			this.mnuRandomGame.Name = "mnuRandomGame";
			this.mnuRandomGame.Size = new System.Drawing.Size(182, 22);
			this.mnuRandomGame.Text = "Load Random Game";
			// 
			// mnuTakeScreenshot
			// 
			this.mnuTakeScreenshot.Image = global::Mesen.GUI.Properties.Resources.Camera;
			this.mnuTakeScreenshot.Name = "mnuTakeScreenshot";
			this.mnuTakeScreenshot.Size = new System.Drawing.Size(182, 22);
			this.mnuTakeScreenshot.Text = "Take Screenshot";
			// 
			// mnuDebug
			// 
			this.mnuDebug.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuDebugger,
            this.mnuEventViewer,
            this.mnuMemoryTools,
            this.mnuRegisterViewer,
            this.mnuTraceLogger,
            this.toolStripMenuItem26,
            this.mnuAssembler,
            this.mnuProfiler,
            this.mnuScriptWindow,
            this.toolStripMenuItem12,
            this.mnuTilemapViewer,
            this.mnuTileViewer,
            this.mnuSpriteViewer,
            this.mnuPaletteViewer,
            this.toolStripMenuItem22,
            this.mnuSpcDebugger,
            this.mnuSa1Debugger,
            this.mnuGsuDebugger,
            this.mnuNecDspDebugger});
			this.mnuDebug.Name = "mnuDebug";
			this.mnuDebug.Size = new System.Drawing.Size(54, 20);
			this.mnuDebug.Text = "Debug";
			this.mnuDebug.DropDownClosed += new System.EventHandler(this.mnu_DropDownClosed);
			this.mnuDebug.DropDownOpened += new System.EventHandler(this.mnu_DropDownOpened);
			// 
			// mnuDebugger
			// 
			this.mnuDebugger.Image = global::Mesen.GUI.Properties.Resources.Debugger;
			this.mnuDebugger.Name = "mnuDebugger";
			this.mnuDebugger.Size = new System.Drawing.Size(183, 22);
			this.mnuDebugger.Text = "Debugger";
			// 
			// mnuEventViewer
			// 
			this.mnuEventViewer.Image = global::Mesen.GUI.Properties.Resources.NesEventViewer;
			this.mnuEventViewer.Name = "mnuEventViewer";
			this.mnuEventViewer.Size = new System.Drawing.Size(183, 22);
			this.mnuEventViewer.Text = "Event Viewer";
			// 
			// mnuMemoryTools
			// 
			this.mnuMemoryTools.Image = global::Mesen.GUI.Properties.Resources.CheatCode;
			this.mnuMemoryTools.Name = "mnuMemoryTools";
			this.mnuMemoryTools.Size = new System.Drawing.Size(183, 22);
			this.mnuMemoryTools.Text = "Memory Tools";
			// 
			// mnuRegisterViewer
			// 
			this.mnuRegisterViewer.Image = global::Mesen.GUI.Properties.Resources.RegisterIcon;
			this.mnuRegisterViewer.Name = "mnuRegisterViewer";
			this.mnuRegisterViewer.Size = new System.Drawing.Size(183, 22);
			this.mnuRegisterViewer.Text = "Register Viewer";
			// 
			// mnuTraceLogger
			// 
			this.mnuTraceLogger.Image = global::Mesen.GUI.Properties.Resources.LogWindow;
			this.mnuTraceLogger.Name = "mnuTraceLogger";
			this.mnuTraceLogger.Size = new System.Drawing.Size(183, 22);
			this.mnuTraceLogger.Text = "Trace Logger";
			// 
			// toolStripMenuItem26
			// 
			this.toolStripMenuItem26.Name = "toolStripMenuItem26";
			this.toolStripMenuItem26.Size = new System.Drawing.Size(180, 6);
			// 
			// mnuAssembler
			// 
			this.mnuAssembler.Image = global::Mesen.GUI.Properties.Resources.Chip;
			this.mnuAssembler.Name = "mnuAssembler";
			this.mnuAssembler.Size = new System.Drawing.Size(183, 22);
			this.mnuAssembler.Text = "Assembler";
			// 
			// mnuProfiler
			// 
			this.mnuProfiler.Image = global::Mesen.GUI.Properties.Resources.PerfTracker;
			this.mnuProfiler.Name = "mnuProfiler";
			this.mnuProfiler.Size = new System.Drawing.Size(183, 22);
			this.mnuProfiler.Text = "Performance Profiler";
			// 
			// mnuScriptWindow
			// 
			this.mnuScriptWindow.Image = global::Mesen.GUI.Properties.Resources.Script;
			this.mnuScriptWindow.Name = "mnuScriptWindow";
			this.mnuScriptWindow.Size = new System.Drawing.Size(183, 22);
			this.mnuScriptWindow.Text = "Script Window";
			// 
			// toolStripMenuItem12
			// 
			this.toolStripMenuItem12.Name = "toolStripMenuItem12";
			this.toolStripMenuItem12.Size = new System.Drawing.Size(180, 6);
			// 
			// mnuTilemapViewer
			// 
			this.mnuTilemapViewer.Image = global::Mesen.GUI.Properties.Resources.VideoOptions;
			this.mnuTilemapViewer.Name = "mnuTilemapViewer";
			this.mnuTilemapViewer.Size = new System.Drawing.Size(183, 22);
			this.mnuTilemapViewer.Text = "Tilemap Viewer";
			// 
			// mnuTileViewer
			// 
			this.mnuTileViewer.Image = global::Mesen.GUI.Properties.Resources.VerticalLayout;
			this.mnuTileViewer.Name = "mnuTileViewer";
			this.mnuTileViewer.Size = new System.Drawing.Size(183, 22);
			this.mnuTileViewer.Text = "Tile Viewer";
			// 
			// mnuSpriteViewer
			// 
			this.mnuSpriteViewer.Image = global::Mesen.GUI.Properties.Resources.PerfTracker;
			this.mnuSpriteViewer.Name = "mnuSpriteViewer";
			this.mnuSpriteViewer.Size = new System.Drawing.Size(183, 22);
			this.mnuSpriteViewer.Text = "Sprite Viewer";
			// 
			// mnuPaletteViewer
			// 
			this.mnuPaletteViewer.Image = global::Mesen.GUI.Properties.Resources.VideoFilter;
			this.mnuPaletteViewer.Name = "mnuPaletteViewer";
			this.mnuPaletteViewer.Size = new System.Drawing.Size(183, 22);
			this.mnuPaletteViewer.Text = "Palette Viewer";
			// 
			// toolStripMenuItem22
			// 
			this.toolStripMenuItem22.Name = "toolStripMenuItem22";
			this.toolStripMenuItem22.Size = new System.Drawing.Size(180, 6);
			// 
			// mnuSpcDebugger
			// 
			this.mnuSpcDebugger.Image = global::Mesen.GUI.Properties.Resources.SpcDebugger;
			this.mnuSpcDebugger.Name = "mnuSpcDebugger";
			this.mnuSpcDebugger.Size = new System.Drawing.Size(183, 22);
			this.mnuSpcDebugger.Text = "SPC Debugger";
			// 
			// mnuSa1Debugger
			// 
			this.mnuSa1Debugger.Image = global::Mesen.GUI.Properties.Resources.Sa1Debugger;
			this.mnuSa1Debugger.Name = "mnuSa1Debugger";
			this.mnuSa1Debugger.Size = new System.Drawing.Size(183, 22);
			this.mnuSa1Debugger.Text = "SA-1 Debugger";
			// 
			// mnuGsuDebugger
			// 
			this.mnuGsuDebugger.Image = global::Mesen.GUI.Properties.Resources.GsuDebugger;
			this.mnuGsuDebugger.Name = "mnuGsuDebugger";
			this.mnuGsuDebugger.Size = new System.Drawing.Size(183, 22);
			this.mnuGsuDebugger.Text = "GSU Debugger";
			// 
			// mnuNecDspDebugger
			// 
			this.mnuNecDspDebugger.Image = global::Mesen.GUI.Properties.Resources.NecDspDebugger;
			this.mnuNecDspDebugger.Name = "mnuNecDspDebugger";
			this.mnuNecDspDebugger.Size = new System.Drawing.Size(183, 22);
			this.mnuNecDspDebugger.Text = "DSP Debugger";
			// 
			// mnuHelp
			// 
			this.mnuHelp.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.mnuCheckForUpdates,
            this.toolStripMenuItem20,
            this.mnuReportBug,
            this.toolStripMenuItem5,
            this.mnuAbout});
			this.mnuHelp.Name = "mnuHelp";
			this.mnuHelp.Size = new System.Drawing.Size(44, 20);
			this.mnuHelp.Text = "Help";
			this.mnuHelp.DropDownClosed += new System.EventHandler(this.mnu_DropDownClosed);
			this.mnuHelp.DropDownOpened += new System.EventHandler(this.mnu_DropDownOpened);
			// 
			// mnuCheckForUpdates
			// 
			this.mnuCheckForUpdates.Image = global::Mesen.GUI.Properties.Resources.Update;
			this.mnuCheckForUpdates.Name = "mnuCheckForUpdates";
			this.mnuCheckForUpdates.Size = new System.Drawing.Size(170, 22);
			this.mnuCheckForUpdates.Text = "Check for updates";
			this.mnuCheckForUpdates.Click += new System.EventHandler(this.mnuCheckForUpdates_Click);
			// 
			// toolStripMenuItem20
			// 
			this.toolStripMenuItem20.Name = "toolStripMenuItem20";
			this.toolStripMenuItem20.Size = new System.Drawing.Size(167, 6);
			// 
			// mnuReportBug
			// 
			this.mnuReportBug.Image = global::Mesen.GUI.Properties.Resources.Comment;
			this.mnuReportBug.Name = "mnuReportBug";
			this.mnuReportBug.Size = new System.Drawing.Size(170, 22);
			this.mnuReportBug.Text = "Report a bug";
			this.mnuReportBug.Click += new System.EventHandler(this.mnuReportBug_Click);
			// 
			// toolStripMenuItem5
			// 
			this.toolStripMenuItem5.Name = "toolStripMenuItem5";
			this.toolStripMenuItem5.Size = new System.Drawing.Size(167, 6);
			// 
			// mnuAbout
			// 
			this.mnuAbout.Image = global::Mesen.GUI.Properties.Resources.Exclamation;
			this.mnuAbout.Name = "mnuAbout";
			this.mnuAbout.Size = new System.Drawing.Size(170, 22);
			this.mnuAbout.Text = "About";
			this.mnuAbout.Click += new System.EventHandler(this.mnuAbout_Click);
			// 
			// pnlRenderer
			// 
			this.pnlRenderer.BackColor = System.Drawing.Color.Black;
			this.pnlRenderer.Controls.Add(this.ctrlRenderer);
			this.pnlRenderer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.pnlRenderer.Location = new System.Drawing.Point(0, 24);
			this.pnlRenderer.Name = "pnlRenderer";
			this.pnlRenderer.Size = new System.Drawing.Size(512, 478);
			this.pnlRenderer.TabIndex = 2;
			// 
			// ctrlRecentGames
			// 
			this.ctrlRecentGames.BackColor = System.Drawing.Color.Transparent;
			this.ctrlRecentGames.Dock = System.Windows.Forms.DockStyle.Top;
			this.ctrlRecentGames.Location = new System.Drawing.Point(0, 24);
			this.ctrlRecentGames.Name = "ctrlRecentGames";
			this.ctrlRecentGames.Size = new System.Drawing.Size(512, 265);
			this.ctrlRecentGames.TabIndex = 1;
			this.ctrlRecentGames.Visible = false;
			// 
			// frmMain
			// 
			this.AllowDrop = true;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(512, 502);
			this.Controls.Add(this.ctrlRecentGames);
			this.Controls.Add(this.pnlRenderer);
			this.Controls.Add(this.mnuMain);
			this.MainMenuStrip = this.mnuMain;
			this.Name = "frmMain";
			this.Text = "frmMain";
			this.Controls.SetChildIndex(this.mnuMain, 0);
			this.Controls.SetChildIndex(this.pnlRenderer, 0);
			this.Controls.SetChildIndex(this.ctrlRecentGames, 0);
			this.mnuMain.ResumeLayout(false);
			this.mnuMain.PerformLayout();
			this.pnlRenderer.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private Controls.ctrlRenderer ctrlRenderer;
		private Controls.ctrlMesenMenuStrip mnuMain;
		private System.Windows.Forms.ToolStripMenuItem mnuDebug;
		private System.Windows.Forms.ToolStripMenuItem mnuDebugger;
		private System.Windows.Forms.ToolStripMenuItem mnuTraceLogger;
		private System.Windows.Forms.ToolStripMenuItem mnuFile;
		private System.Windows.Forms.ToolStripMenuItem mnuOpen;
		private System.Windows.Forms.ToolStripMenuItem mnuMemoryTools;
		private System.Windows.Forms.ToolStripMenuItem mnuTilemapViewer;
		private System.Windows.Forms.ToolStripMenuItem mnuEventViewer;
		private System.Windows.Forms.ToolStripMenuItem mnuOptions;
		private System.Windows.Forms.ToolStripMenuItem mnuVideoConfig;
		private System.Windows.Forms.Panel pnlRenderer;
		private System.Windows.Forms.ToolStripMenuItem mnuAudioConfig;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem mnuRecentFiles;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem6;
		private System.Windows.Forms.ToolStripMenuItem mnuExit;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
		private System.Windows.Forms.ToolStripMenuItem mnuPreferences;
		private System.Windows.Forms.ToolStripMenuItem mnuGame;
		private System.Windows.Forms.ToolStripMenuItem mnuPause;
		private System.Windows.Forms.ToolStripMenuItem mnuReset;
		private System.Windows.Forms.ToolStripMenuItem mnuPowerCycle;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem24;
		private System.Windows.Forms.ToolStripMenuItem mnuPowerOff;
		private System.Windows.Forms.ToolStripMenuItem mnuEmulationSpeed;
		private System.Windows.Forms.ToolStripMenuItem mnuEmuSpeedNormal;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem8;
		private System.Windows.Forms.ToolStripMenuItem mnuIncreaseSpeed;
		private System.Windows.Forms.ToolStripMenuItem mnuDecreaseSpeed;
		private System.Windows.Forms.ToolStripMenuItem mnuEmuSpeedMaximumSpeed;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem9;
		private System.Windows.Forms.ToolStripMenuItem mnuEmuSpeedTriple;
		private System.Windows.Forms.ToolStripMenuItem mnuEmuSpeedDouble;
		private System.Windows.Forms.ToolStripMenuItem mnuEmuSpeedHalf;
		private System.Windows.Forms.ToolStripMenuItem mnuEmuSpeedQuarter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem14;
		private System.Windows.Forms.ToolStripMenuItem mnuShowFPS;
		private System.Windows.Forms.ToolStripMenuItem mnuVideoScale;
		private System.Windows.Forms.ToolStripMenuItem mnuScale1x;
		private System.Windows.Forms.ToolStripMenuItem mnuScale2x;
		private System.Windows.Forms.ToolStripMenuItem mnuScale3x;
		private System.Windows.Forms.ToolStripMenuItem mnuScale4x;
		private System.Windows.Forms.ToolStripMenuItem mnuScale5x;
		private System.Windows.Forms.ToolStripMenuItem mnuScale6x;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem13;
		private System.Windows.Forms.ToolStripMenuItem mnuFullscreen;
		private System.Windows.Forms.ToolStripMenuItem mnuVideoFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuNoneFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem21;
		private System.Windows.Forms.ToolStripMenuItem mnuNtscFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem15;
		private System.Windows.Forms.ToolStripMenuItem mnuXBRZ2xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuXBRZ3xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuXBRZ4xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuXBRZ5xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuXBRZ6xFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem16;
		private System.Windows.Forms.ToolStripMenuItem mnuHQ2xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuHQ3xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuHQ4xFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem17;
		private System.Windows.Forms.ToolStripMenuItem mnuScale2xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuScale3xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuScale4xFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem23;
		private System.Windows.Forms.ToolStripMenuItem mnu2xSaiFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuSuper2xSaiFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuSuperEagleFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem18;
		private System.Windows.Forms.ToolStripMenuItem mnuPrescale2xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuPrescale3xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuPrescale4xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuPrescale6xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuPrescale8xFilter;
		private System.Windows.Forms.ToolStripMenuItem mnuPrescale10xFilter;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem19;
		private System.Windows.Forms.ToolStripMenuItem mnuBlendHighResolutionModes;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
		private System.Windows.Forms.ToolStripMenuItem mnuHelp;
		private System.Windows.Forms.ToolStripMenuItem mnuCheckForUpdates;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem20;
		private System.Windows.Forms.ToolStripMenuItem mnuReportBug;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
		private System.Windows.Forms.ToolStripMenuItem mnuAbout;
		private System.Windows.Forms.ToolStripMenuItem mnuTools;
		private System.Windows.Forms.ToolStripMenuItem mnuLogWindow;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem7;
		private System.Windows.Forms.ToolStripMenuItem mnuTakeScreenshot;
		private System.Windows.Forms.ToolStripMenuItem mnuSaveState;
		private System.Windows.Forms.ToolStripMenuItem mnuLoadState;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem10;
		private System.Windows.Forms.ToolStripMenuItem mnuEmulationConfig;
		private System.Windows.Forms.ToolStripMenuItem mnuInputConfig;
		private System.Windows.Forms.ToolStripMenuItem mnuRegion;
		private System.Windows.Forms.ToolStripMenuItem mnuRegionAuto;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem mnuRegionNtsc;
		private System.Windows.Forms.ToolStripMenuItem mnuRegionPal;
		private Controls.ctrlRecentGames ctrlRecentGames;
		private System.Windows.Forms.ToolStripMenuItem mnuVideoRecorder;
		private System.Windows.Forms.ToolStripMenuItem mnuAviRecord;
		private System.Windows.Forms.ToolStripMenuItem mnuAviStop;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem11;
		private System.Windows.Forms.ToolStripMenuItem mnuSoundRecorder;
		private System.Windows.Forms.ToolStripMenuItem mnuWaveRecord;
		private System.Windows.Forms.ToolStripMenuItem mnuWaveStop;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem12;
		private System.Windows.Forms.ToolStripMenuItem mnuPaletteViewer;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem22;
		private System.Windows.Forms.ToolStripMenuItem mnuTileViewer;
		private System.Windows.Forms.ToolStripMenuItem mnuSpcDebugger;
		private System.Windows.Forms.ToolStripMenuItem mnuSpriteViewer;
		private System.Windows.Forms.ToolStripMenuItem mnuScriptWindow;
		private System.Windows.Forms.ToolStripMenuItem mnuSa1Debugger;
		private System.Windows.Forms.ToolStripMenuItem mnuGsuDebugger;
		private System.Windows.Forms.ToolStripMenuItem mnuMovies;
		private System.Windows.Forms.ToolStripMenuItem mnuPlayMovie;
		private System.Windows.Forms.ToolStripMenuItem mnuRecordMovie;
		private System.Windows.Forms.ToolStripMenuItem mnuStopMovie;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem25;
		private System.Windows.Forms.ToolStripMenuItem mnuBilinearInterpolation;
		private System.Windows.Forms.ToolStripMenuItem mnuRegisterViewer;
		private System.Windows.Forms.ToolStripMenuItem mnuRandomGame;
		private System.Windows.Forms.ToolStripMenuItem mnuCheats;
		private System.Windows.Forms.ToolStripMenuItem mnuTests;
		private System.Windows.Forms.ToolStripMenuItem mnuTestRun;
		private System.Windows.Forms.ToolStripMenuItem mnuTestRecord;
		private System.Windows.Forms.ToolStripMenuItem mnuTestStop;
		private System.Windows.Forms.ToolStripMenuItem mnuRunAllTests;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlay;
		private System.Windows.Forms.ToolStripMenuItem mnuStartServer;
		private System.Windows.Forms.ToolStripMenuItem mnuConnect;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlaySelectController;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlayPlayer1;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlayPlayer2;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlayPlayer3;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlayPlayer4;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlayPlayer5;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
		private System.Windows.Forms.ToolStripMenuItem mnuNetPlaySpectator;
		private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
		private System.Windows.Forms.ToolStripMenuItem mnuProfile;
	  private System.Windows.Forms.ToolStripMenuItem mnuProfiler;
	  private System.Windows.Forms.ToolStripSeparator toolStripMenuItem26;
	  private System.Windows.Forms.ToolStripMenuItem mnuAssembler;
	  private System.Windows.Forms.ToolStripMenuItem mnuReloadRom;
	  private System.Windows.Forms.ToolStripMenuItem mnuNecDspDebugger;
   }
}