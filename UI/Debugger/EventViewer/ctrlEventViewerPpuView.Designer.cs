using Mesen.GUI.Controls;

namespace Mesen.GUI.Debugger
{
	partial class ctrlEventViewerPpuView
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

		#region Component Designer generated code

		/// <summary> 
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.tmrOverlay = new System.Windows.Forms.Timer(this.components);
			this.grpOptions = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
			this.picNmi = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowNmi = new System.Windows.Forms.CheckBox();
			this.picIrq = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowIrq = new System.Windows.Forms.CheckBox();
			this.label4 = new System.Windows.Forms.Label();
			this.picWramWrites = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowWorkRamRegisterWrites = new System.Windows.Forms.CheckBox();
			this.picWramReads = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowWorkRamRegisterReads = new System.Windows.Forms.CheckBox();
			this.label3 = new System.Windows.Forms.Label();
			this.picCpuWrites = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowCpuRegisterWrites = new System.Windows.Forms.CheckBox();
			this.picCpuReads = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowCpuRegisterReads = new System.Windows.Forms.CheckBox();
			this.label2 = new System.Windows.Forms.Label();
			this.picApuWrites = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowApuRegisterWrites = new System.Windows.Forms.CheckBox();
			this.picApuReads = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowApuRegisterReads = new System.Windows.Forms.CheckBox();
			this.label1 = new System.Windows.Forms.Label();
			this.picPpuWrites = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowPpuRegisterWrites = new System.Windows.Forms.CheckBox();
			this.chkShowPpuRegisterReads = new System.Windows.Forms.CheckBox();
			this.lblPpuRegisters = new System.Windows.Forms.Label();
			this.picPpuReads = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowMarkedBreakpoints = new System.Windows.Forms.CheckBox();
			this.picMarkedBreakpoints = new Mesen.GUI.Debugger.ctrlColorPicker();
			this.chkShowPreviousFrameEvents = new System.Windows.Forms.CheckBox();
			this.grpDmaFilters = new System.Windows.Forms.GroupBox();
			this.tableLayoutPanel2 = new System.Windows.Forms.TableLayoutPanel();
			this.chkDmaChannel0 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel1 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel2 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel3 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel4 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel5 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel6 = new System.Windows.Forms.CheckBox();
			this.chkDmaChannel7 = new System.Windows.Forms.CheckBox();
			this.picViewer = new Mesen.GUI.Debugger.PpuViewer.ctrlImagePanel();
			this.grpOptions.SuspendLayout();
			this.tableLayoutPanel1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.picNmi)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picIrq)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picWramWrites)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picWramReads)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picCpuWrites)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picCpuReads)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picApuWrites)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picApuReads)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picPpuWrites)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picPpuReads)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.picMarkedBreakpoints)).BeginInit();
			this.grpDmaFilters.SuspendLayout();
			this.tableLayoutPanel2.SuspendLayout();
			this.SuspendLayout();
			// 
			// tmrOverlay
			// 
			this.tmrOverlay.Interval = 50;
			this.tmrOverlay.Tick += new System.EventHandler(this.tmrOverlay_Tick);
			// 
			// grpOptions
			// 
			this.grpOptions.Controls.Add(this.tableLayoutPanel1);
			this.grpOptions.Dock = System.Windows.Forms.DockStyle.Right;
			this.grpOptions.Location = new System.Drawing.Point(686, 0);
			this.grpOptions.Name = "grpOptions";
			this.grpOptions.Size = new System.Drawing.Size(261, 529);
			this.grpOptions.TabIndex = 2;
			this.grpOptions.TabStop = false;
			this.grpOptions.Text = "Show...";
			// 
			// tableLayoutPanel1
			// 
			this.tableLayoutPanel1.ColumnCount = 6;
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 15F));
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle());
			this.tableLayoutPanel1.Controls.Add(this.picNmi, 5, 4);
			this.tableLayoutPanel1.Controls.Add(this.chkShowNmi, 4, 4);
			this.tableLayoutPanel1.Controls.Add(this.picIrq, 2, 4);
			this.tableLayoutPanel1.Controls.Add(this.chkShowIrq, 1, 4);
			this.tableLayoutPanel1.Controls.Add(this.label4, 0, 4);
			this.tableLayoutPanel1.Controls.Add(this.picWramWrites, 5, 3);
			this.tableLayoutPanel1.Controls.Add(this.chkShowWorkRamRegisterWrites, 4, 3);
			this.tableLayoutPanel1.Controls.Add(this.picWramReads, 2, 3);
			this.tableLayoutPanel1.Controls.Add(this.chkShowWorkRamRegisterReads, 1, 3);
			this.tableLayoutPanel1.Controls.Add(this.label3, 0, 3);
			this.tableLayoutPanel1.Controls.Add(this.picCpuWrites, 5, 2);
			this.tableLayoutPanel1.Controls.Add(this.chkShowCpuRegisterWrites, 4, 2);
			this.tableLayoutPanel1.Controls.Add(this.picCpuReads, 2, 2);
			this.tableLayoutPanel1.Controls.Add(this.chkShowCpuRegisterReads, 1, 2);
			this.tableLayoutPanel1.Controls.Add(this.label2, 0, 2);
			this.tableLayoutPanel1.Controls.Add(this.picApuWrites, 5, 1);
			this.tableLayoutPanel1.Controls.Add(this.chkShowApuRegisterWrites, 4, 1);
			this.tableLayoutPanel1.Controls.Add(this.picApuReads, 2, 1);
			this.tableLayoutPanel1.Controls.Add(this.chkShowApuRegisterReads, 1, 1);
			this.tableLayoutPanel1.Controls.Add(this.label1, 0, 1);
			this.tableLayoutPanel1.Controls.Add(this.picPpuWrites, 5, 0);
			this.tableLayoutPanel1.Controls.Add(this.chkShowPpuRegisterWrites, 4, 0);
			this.tableLayoutPanel1.Controls.Add(this.chkShowPpuRegisterReads, 1, 0);
			this.tableLayoutPanel1.Controls.Add(this.lblPpuRegisters, 0, 0);
			this.tableLayoutPanel1.Controls.Add(this.picPpuReads, 2, 0);
			this.tableLayoutPanel1.Controls.Add(this.chkShowMarkedBreakpoints, 0, 5);
			this.tableLayoutPanel1.Controls.Add(this.picMarkedBreakpoints, 2, 5);
			this.tableLayoutPanel1.Controls.Add(this.chkShowPreviousFrameEvents, 0, 6);
			this.tableLayoutPanel1.Controls.Add(this.grpDmaFilters, 0, 7);
			this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel1.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel1.Name = "tableLayoutPanel1";
			this.tableLayoutPanel1.RowCount = 8;
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel1.Size = new System.Drawing.Size(255, 510);
			this.tableLayoutPanel1.TabIndex = 0;
			// 
			// picNmi
			// 
			this.picNmi.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picNmi.Location = new System.Drawing.Point(238, 95);
			this.picNmi.Name = "picNmi";
			this.picNmi.Size = new System.Drawing.Size(14, 14);
			this.picNmi.TabIndex = 25;
			this.picNmi.TabStop = false;
			// 
			// chkShowNmi
			// 
			this.chkShowNmi.AutoSize = true;
			this.chkShowNmi.Location = new System.Drawing.Point(181, 95);
			this.chkShowNmi.Name = "chkShowNmi";
			this.chkShowNmi.Size = new System.Drawing.Size(46, 17);
			this.chkShowNmi.TabIndex = 24;
			this.chkShowNmi.Text = "NMI";
			this.chkShowNmi.UseVisualStyleBackColor = true;
			this.chkShowNmi.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// picIrq
			// 
			this.picIrq.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picIrq.Location = new System.Drawing.Point(146, 95);
			this.picIrq.Name = "picIrq";
			this.picIrq.Size = new System.Drawing.Size(14, 14);
			this.picIrq.TabIndex = 23;
			this.picIrq.TabStop = false;
			// 
			// chkShowIrq
			// 
			this.chkShowIrq.AutoSize = true;
			this.chkShowIrq.Location = new System.Drawing.Point(88, 95);
			this.chkShowIrq.Name = "chkShowIrq";
			this.chkShowIrq.Size = new System.Drawing.Size(45, 17);
			this.chkShowIrq.TabIndex = 22;
			this.chkShowIrq.Text = "IRQ";
			this.chkShowIrq.UseVisualStyleBackColor = true;
			this.chkShowIrq.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// label4
			// 
			this.label4.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(3, 97);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(51, 13);
			this.label4.TabIndex = 21;
			this.label4.Text = "Interrupts";
			// 
			// picWramWrites
			// 
			this.picWramWrites.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picWramWrites.Location = new System.Drawing.Point(238, 72);
			this.picWramWrites.Name = "picWramWrites";
			this.picWramWrites.Size = new System.Drawing.Size(14, 14);
			this.picWramWrites.TabIndex = 20;
			this.picWramWrites.TabStop = false;
			// 
			// chkShowWorkRamRegisterWrites
			// 
			this.chkShowWorkRamRegisterWrites.AutoSize = true;
			this.chkShowWorkRamRegisterWrites.Location = new System.Drawing.Point(181, 72);
			this.chkShowWorkRamRegisterWrites.Name = "chkShowWorkRamRegisterWrites";
			this.chkShowWorkRamRegisterWrites.Size = new System.Drawing.Size(51, 17);
			this.chkShowWorkRamRegisterWrites.TabIndex = 19;
			this.chkShowWorkRamRegisterWrites.Text = "Write";
			this.chkShowWorkRamRegisterWrites.UseVisualStyleBackColor = true;
			this.chkShowWorkRamRegisterWrites.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// picWramReads
			// 
			this.picWramReads.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picWramReads.Location = new System.Drawing.Point(146, 72);
			this.picWramReads.Name = "picWramReads";
			this.picWramReads.Size = new System.Drawing.Size(14, 14);
			this.picWramReads.TabIndex = 18;
			this.picWramReads.TabStop = false;
			// 
			// chkShowWorkRamRegisterReads
			// 
			this.chkShowWorkRamRegisterReads.AutoSize = true;
			this.chkShowWorkRamRegisterReads.Location = new System.Drawing.Point(88, 72);
			this.chkShowWorkRamRegisterReads.Name = "chkShowWorkRamRegisterReads";
			this.chkShowWorkRamRegisterReads.Size = new System.Drawing.Size(52, 17);
			this.chkShowWorkRamRegisterReads.TabIndex = 17;
			this.chkShowWorkRamRegisterReads.Text = "Read";
			this.chkShowWorkRamRegisterReads.UseVisualStyleBackColor = true;
			this.chkShowWorkRamRegisterReads.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// label3
			// 
			this.label3.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(3, 74);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(73, 13);
			this.label3.TabIndex = 16;
			this.label3.Text = "WRAM Regs:";
			// 
			// picCpuWrites
			// 
			this.picCpuWrites.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picCpuWrites.Location = new System.Drawing.Point(238, 49);
			this.picCpuWrites.Name = "picCpuWrites";
			this.picCpuWrites.Size = new System.Drawing.Size(14, 14);
			this.picCpuWrites.TabIndex = 15;
			this.picCpuWrites.TabStop = false;
			// 
			// chkShowCpuRegisterWrites
			// 
			this.chkShowCpuRegisterWrites.AutoSize = true;
			this.chkShowCpuRegisterWrites.Location = new System.Drawing.Point(181, 49);
			this.chkShowCpuRegisterWrites.Name = "chkShowCpuRegisterWrites";
			this.chkShowCpuRegisterWrites.Size = new System.Drawing.Size(51, 17);
			this.chkShowCpuRegisterWrites.TabIndex = 14;
			this.chkShowCpuRegisterWrites.Text = "Write";
			this.chkShowCpuRegisterWrites.UseVisualStyleBackColor = true;
			this.chkShowCpuRegisterWrites.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// picCpuReads
			// 
			this.picCpuReads.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picCpuReads.Location = new System.Drawing.Point(146, 49);
			this.picCpuReads.Name = "picCpuReads";
			this.picCpuReads.Size = new System.Drawing.Size(14, 14);
			this.picCpuReads.TabIndex = 13;
			this.picCpuReads.TabStop = false;
			// 
			// chkShowCpuRegisterReads
			// 
			this.chkShowCpuRegisterReads.AutoSize = true;
			this.chkShowCpuRegisterReads.Location = new System.Drawing.Point(88, 49);
			this.chkShowCpuRegisterReads.Name = "chkShowCpuRegisterReads";
			this.chkShowCpuRegisterReads.Size = new System.Drawing.Size(52, 17);
			this.chkShowCpuRegisterReads.TabIndex = 12;
			this.chkShowCpuRegisterReads.Text = "Read";
			this.chkShowCpuRegisterReads.UseVisualStyleBackColor = true;
			this.chkShowCpuRegisterReads.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// label2
			// 
			this.label2.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(3, 51);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(79, 13);
			this.label2.TabIndex = 11;
			this.label2.Text = "CPU Registers:";
			// 
			// picApuWrites
			// 
			this.picApuWrites.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picApuWrites.Location = new System.Drawing.Point(238, 26);
			this.picApuWrites.Name = "picApuWrites";
			this.picApuWrites.Size = new System.Drawing.Size(14, 14);
			this.picApuWrites.TabIndex = 10;
			this.picApuWrites.TabStop = false;
			// 
			// chkShowApuRegisterWrites
			// 
			this.chkShowApuRegisterWrites.AutoSize = true;
			this.chkShowApuRegisterWrites.Location = new System.Drawing.Point(181, 26);
			this.chkShowApuRegisterWrites.Name = "chkShowApuRegisterWrites";
			this.chkShowApuRegisterWrites.Size = new System.Drawing.Size(51, 17);
			this.chkShowApuRegisterWrites.TabIndex = 9;
			this.chkShowApuRegisterWrites.Text = "Write";
			this.chkShowApuRegisterWrites.UseVisualStyleBackColor = true;
			this.chkShowApuRegisterWrites.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// picApuReads
			// 
			this.picApuReads.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picApuReads.Location = new System.Drawing.Point(146, 26);
			this.picApuReads.Name = "picApuReads";
			this.picApuReads.Size = new System.Drawing.Size(14, 14);
			this.picApuReads.TabIndex = 8;
			this.picApuReads.TabStop = false;
			// 
			// chkShowApuRegisterReads
			// 
			this.chkShowApuRegisterReads.AutoSize = true;
			this.chkShowApuRegisterReads.Location = new System.Drawing.Point(88, 26);
			this.chkShowApuRegisterReads.Name = "chkShowApuRegisterReads";
			this.chkShowApuRegisterReads.Size = new System.Drawing.Size(52, 17);
			this.chkShowApuRegisterReads.TabIndex = 7;
			this.chkShowApuRegisterReads.Text = "Read";
			this.chkShowApuRegisterReads.UseVisualStyleBackColor = true;
			this.chkShowApuRegisterReads.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// label1
			// 
			this.label1.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(3, 28);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(79, 13);
			this.label1.TabIndex = 6;
			this.label1.Text = "APU Registers:";
			// 
			// picPpuWrites
			// 
			this.picPpuWrites.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picPpuWrites.Location = new System.Drawing.Point(238, 3);
			this.picPpuWrites.Name = "picPpuWrites";
			this.picPpuWrites.Size = new System.Drawing.Size(14, 14);
			this.picPpuWrites.TabIndex = 5;
			this.picPpuWrites.TabStop = false;
			// 
			// chkShowPpuRegisterWrites
			// 
			this.chkShowPpuRegisterWrites.AutoSize = true;
			this.chkShowPpuRegisterWrites.Location = new System.Drawing.Point(181, 3);
			this.chkShowPpuRegisterWrites.Name = "chkShowPpuRegisterWrites";
			this.chkShowPpuRegisterWrites.Size = new System.Drawing.Size(51, 17);
			this.chkShowPpuRegisterWrites.TabIndex = 2;
			this.chkShowPpuRegisterWrites.Text = "Write";
			this.chkShowPpuRegisterWrites.UseVisualStyleBackColor = true;
			this.chkShowPpuRegisterWrites.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkShowPpuRegisterReads
			// 
			this.chkShowPpuRegisterReads.AutoSize = true;
			this.chkShowPpuRegisterReads.Location = new System.Drawing.Point(88, 3);
			this.chkShowPpuRegisterReads.Name = "chkShowPpuRegisterReads";
			this.chkShowPpuRegisterReads.Size = new System.Drawing.Size(52, 17);
			this.chkShowPpuRegisterReads.TabIndex = 0;
			this.chkShowPpuRegisterReads.Text = "Read";
			this.chkShowPpuRegisterReads.UseVisualStyleBackColor = true;
			this.chkShowPpuRegisterReads.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// lblPpuRegisters
			// 
			this.lblPpuRegisters.Anchor = System.Windows.Forms.AnchorStyles.Left;
			this.lblPpuRegisters.AutoSize = true;
			this.lblPpuRegisters.Location = new System.Drawing.Point(3, 5);
			this.lblPpuRegisters.Name = "lblPpuRegisters";
			this.lblPpuRegisters.Size = new System.Drawing.Size(79, 13);
			this.lblPpuRegisters.TabIndex = 3;
			this.lblPpuRegisters.Text = "PPU Registers:";
			// 
			// picPpuReads
			// 
			this.picPpuReads.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picPpuReads.Location = new System.Drawing.Point(146, 3);
			this.picPpuReads.Name = "picPpuReads";
			this.picPpuReads.Size = new System.Drawing.Size(14, 14);
			this.picPpuReads.TabIndex = 4;
			this.picPpuReads.TabStop = false;
			// 
			// chkShowMarkedBreakpoints
			// 
			this.chkShowMarkedBreakpoints.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.chkShowMarkedBreakpoints, 2);
			this.chkShowMarkedBreakpoints.Enabled = false;
			this.chkShowMarkedBreakpoints.Location = new System.Drawing.Point(3, 118);
			this.chkShowMarkedBreakpoints.Name = "chkShowMarkedBreakpoints";
			this.chkShowMarkedBreakpoints.Size = new System.Drawing.Size(121, 17);
			this.chkShowMarkedBreakpoints.TabIndex = 26;
			this.chkShowMarkedBreakpoints.Text = "Marked Breakpoints";
			this.chkShowMarkedBreakpoints.UseVisualStyleBackColor = true;
			this.chkShowMarkedBreakpoints.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// picMarkedBreakpoints
			// 
			this.picMarkedBreakpoints.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
			this.picMarkedBreakpoints.Location = new System.Drawing.Point(146, 118);
			this.picMarkedBreakpoints.Name = "picMarkedBreakpoints";
			this.picMarkedBreakpoints.Size = new System.Drawing.Size(14, 14);
			this.picMarkedBreakpoints.TabIndex = 27;
			this.picMarkedBreakpoints.TabStop = false;
			// 
			// chkShowPreviousFrameEvents
			// 
			this.chkShowPreviousFrameEvents.AutoSize = true;
			this.tableLayoutPanel1.SetColumnSpan(this.chkShowPreviousFrameEvents, 6);
			this.chkShowPreviousFrameEvents.Location = new System.Drawing.Point(3, 148);
			this.chkShowPreviousFrameEvents.Margin = new System.Windows.Forms.Padding(3, 10, 3, 3);
			this.chkShowPreviousFrameEvents.Name = "chkShowPreviousFrameEvents";
			this.chkShowPreviousFrameEvents.Size = new System.Drawing.Size(167, 17);
			this.chkShowPreviousFrameEvents.TabIndex = 28;
			this.chkShowPreviousFrameEvents.Text = "Show previous frame\'s events";
			this.chkShowPreviousFrameEvents.UseVisualStyleBackColor = true;
			this.chkShowPreviousFrameEvents.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// grpDmaFilters
			// 
			this.tableLayoutPanel1.SetColumnSpan(this.grpDmaFilters, 6);
			this.grpDmaFilters.Controls.Add(this.tableLayoutPanel2);
			this.grpDmaFilters.Location = new System.Drawing.Point(3, 171);
			this.grpDmaFilters.Name = "grpDmaFilters";
			this.grpDmaFilters.Size = new System.Drawing.Size(249, 113);
			this.grpDmaFilters.TabIndex = 29;
			this.grpDmaFilters.TabStop = false;
			this.grpDmaFilters.Text = "DMA Filters";
			// 
			// tableLayoutPanel2
			// 
			this.tableLayoutPanel2.ColumnCount = 2;
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel2.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 50F));
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel0, 0, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel1, 0, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel2, 0, 2);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel3, 0, 3);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel4, 1, 0);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel5, 1, 1);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel6, 1, 2);
			this.tableLayoutPanel2.Controls.Add(this.chkDmaChannel7, 1, 3);
			this.tableLayoutPanel2.Dock = System.Windows.Forms.DockStyle.Fill;
			this.tableLayoutPanel2.Location = new System.Drawing.Point(3, 16);
			this.tableLayoutPanel2.Name = "tableLayoutPanel2";
			this.tableLayoutPanel2.RowCount = 5;
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle());
			this.tableLayoutPanel2.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
			this.tableLayoutPanel2.Size = new System.Drawing.Size(243, 94);
			this.tableLayoutPanel2.TabIndex = 0;
			// 
			// chkDmaChannel0
			// 
			this.chkDmaChannel0.AutoSize = true;
			this.chkDmaChannel0.Location = new System.Drawing.Point(3, 3);
			this.chkDmaChannel0.Name = "chkDmaChannel0";
			this.chkDmaChannel0.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel0.TabIndex = 0;
			this.chkDmaChannel0.Text = "Channel 0";
			this.chkDmaChannel0.UseVisualStyleBackColor = true;
			this.chkDmaChannel0.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel1
			// 
			this.chkDmaChannel1.AutoSize = true;
			this.chkDmaChannel1.Location = new System.Drawing.Point(3, 26);
			this.chkDmaChannel1.Name = "chkDmaChannel1";
			this.chkDmaChannel1.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel1.TabIndex = 2;
			this.chkDmaChannel1.Text = "Channel 1";
			this.chkDmaChannel1.UseVisualStyleBackColor = true;
			this.chkDmaChannel1.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel2
			// 
			this.chkDmaChannel2.AutoSize = true;
			this.chkDmaChannel2.Location = new System.Drawing.Point(3, 49);
			this.chkDmaChannel2.Name = "chkDmaChannel2";
			this.chkDmaChannel2.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel2.TabIndex = 4;
			this.chkDmaChannel2.Text = "Channel 2";
			this.chkDmaChannel2.UseVisualStyleBackColor = true;
			this.chkDmaChannel2.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel3
			// 
			this.chkDmaChannel3.AutoSize = true;
			this.chkDmaChannel3.Location = new System.Drawing.Point(3, 72);
			this.chkDmaChannel3.Name = "chkDmaChannel3";
			this.chkDmaChannel3.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel3.TabIndex = 6;
			this.chkDmaChannel3.Text = "Channel 3";
			this.chkDmaChannel3.UseVisualStyleBackColor = true;
			this.chkDmaChannel3.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel4
			// 
			this.chkDmaChannel4.AutoSize = true;
			this.chkDmaChannel4.Location = new System.Drawing.Point(124, 3);
			this.chkDmaChannel4.Name = "chkDmaChannel4";
			this.chkDmaChannel4.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel4.TabIndex = 1;
			this.chkDmaChannel4.Text = "Channel 4";
			this.chkDmaChannel4.UseVisualStyleBackColor = true;
			this.chkDmaChannel4.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel5
			// 
			this.chkDmaChannel5.AutoSize = true;
			this.chkDmaChannel5.Location = new System.Drawing.Point(124, 26);
			this.chkDmaChannel5.Name = "chkDmaChannel5";
			this.chkDmaChannel5.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel5.TabIndex = 3;
			this.chkDmaChannel5.Text = "Channel 5";
			this.chkDmaChannel5.UseVisualStyleBackColor = true;
			this.chkDmaChannel5.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel6
			// 
			this.chkDmaChannel6.AutoSize = true;
			this.chkDmaChannel6.Location = new System.Drawing.Point(124, 49);
			this.chkDmaChannel6.Name = "chkDmaChannel6";
			this.chkDmaChannel6.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel6.TabIndex = 5;
			this.chkDmaChannel6.Text = "Channel 6";
			this.chkDmaChannel6.UseVisualStyleBackColor = true;
			this.chkDmaChannel6.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// chkDmaChannel7
			// 
			this.chkDmaChannel7.AutoSize = true;
			this.chkDmaChannel7.Location = new System.Drawing.Point(124, 72);
			this.chkDmaChannel7.Name = "chkDmaChannel7";
			this.chkDmaChannel7.Size = new System.Drawing.Size(74, 17);
			this.chkDmaChannel7.TabIndex = 7;
			this.chkDmaChannel7.Text = "Channel 7";
			this.chkDmaChannel7.UseVisualStyleBackColor = true;
			this.chkDmaChannel7.Click += new System.EventHandler(this.chkOption_Click);
			// 
			// picViewer
			// 
			this.picViewer.Dock = System.Windows.Forms.DockStyle.Fill;
			this.picViewer.Image = null;
			this.picViewer.ImageScale = 1;
			this.picViewer.ImageSize = new System.Drawing.Size(0, 0);
			this.picViewer.Location = new System.Drawing.Point(0, 0);
			this.picViewer.Name = "picViewer";
			this.picViewer.Selection = new System.Drawing.Rectangle(0, 0, 0, 0);
			this.picViewer.SelectionWrapPosition = 0;
			this.picViewer.Size = new System.Drawing.Size(686, 529);
			this.picViewer.TabIndex = 3;
			this.picViewer.MouseLeave += new System.EventHandler(this.picPicture_MouseLeave);
			this.picViewer.MouseMove += new System.Windows.Forms.MouseEventHandler(this.picPicture_MouseMove);
			// 
			// ctrlEventViewerPpuView
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.Controls.Add(this.picViewer);
			this.Controls.Add(this.grpOptions);
			this.Name = "ctrlEventViewerPpuView";
			this.Size = new System.Drawing.Size(947, 529);
			this.grpOptions.ResumeLayout(false);
			this.tableLayoutPanel1.ResumeLayout(false);
			this.tableLayoutPanel1.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.picNmi)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picIrq)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picWramWrites)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picWramReads)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picCpuWrites)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picCpuReads)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picApuWrites)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picApuReads)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picPpuWrites)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picPpuReads)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.picMarkedBreakpoints)).EndInit();
			this.grpDmaFilters.ResumeLayout(false);
			this.tableLayoutPanel2.ResumeLayout(false);
			this.tableLayoutPanel2.PerformLayout();
			this.ResumeLayout(false);

		}

		#endregion
		private System.Windows.Forms.Timer tmrOverlay;
		private System.Windows.Forms.GroupBox grpOptions;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
		private ctrlColorPicker picNmi;
		private System.Windows.Forms.CheckBox chkShowNmi;
		private ctrlColorPicker picIrq;
		private System.Windows.Forms.CheckBox chkShowIrq;
		private System.Windows.Forms.Label label4;
		private ctrlColorPicker picWramWrites;
		private System.Windows.Forms.CheckBox chkShowWorkRamRegisterWrites;
		private ctrlColorPicker picWramReads;
		private System.Windows.Forms.CheckBox chkShowWorkRamRegisterReads;
		private System.Windows.Forms.Label label3;
		private ctrlColorPicker picCpuWrites;
		private System.Windows.Forms.CheckBox chkShowCpuRegisterWrites;
		private ctrlColorPicker picCpuReads;
		private System.Windows.Forms.CheckBox chkShowCpuRegisterReads;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.CheckBox chkShowApuRegisterWrites;
		private ctrlColorPicker picApuReads;
		private System.Windows.Forms.CheckBox chkShowApuRegisterReads;
		private System.Windows.Forms.Label label1;
		private ctrlColorPicker picPpuWrites;
		private System.Windows.Forms.CheckBox chkShowPpuRegisterWrites;
		private System.Windows.Forms.CheckBox chkShowPpuRegisterReads;
		private System.Windows.Forms.Label lblPpuRegisters;
		private ctrlColorPicker picPpuReads;
		private ctrlColorPicker picApuWrites;
		private System.Windows.Forms.CheckBox chkShowMarkedBreakpoints;
		private ctrlColorPicker picMarkedBreakpoints;
		private System.Windows.Forms.CheckBox chkShowPreviousFrameEvents;
		private PpuViewer.ctrlImagePanel picViewer;
		private System.Windows.Forms.GroupBox grpDmaFilters;
		private System.Windows.Forms.TableLayoutPanel tableLayoutPanel2;
		private System.Windows.Forms.CheckBox chkDmaChannel0;
		private System.Windows.Forms.CheckBox chkDmaChannel1;
		private System.Windows.Forms.CheckBox chkDmaChannel2;
		private System.Windows.Forms.CheckBox chkDmaChannel3;
		private System.Windows.Forms.CheckBox chkDmaChannel4;
		private System.Windows.Forms.CheckBox chkDmaChannel5;
		private System.Windows.Forms.CheckBox chkDmaChannel6;
		private System.Windows.Forms.CheckBox chkDmaChannel7;
	}
}
