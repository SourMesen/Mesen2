using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.IO.Compression;
using static Mesen.GUI.Controls.ctrlRecentGames;
using System.Drawing.Drawing2D;
using Mesen.GUI.Forms;
using Mesen.GUI.Emulation;

namespace Mesen.GUI.Controls
{
	public partial class ctrlRecentGame : UserControl
	{
		public event RecentGameLoadedHandler OnRecentGameLoaded;
		private RecentGameInfo _recentGame;

		public ctrlRecentGame()
		{
			InitializeComponent();
		}

		public GameScreenMode Mode { get; set; }

		public RecentGameInfo RecentGame
		{
			get { return _recentGame; }
			set
			{
				if(value == null) {
					_recentGame = null;
					picPreviousState.Visible = false;
					lblGameName.Visible = false;
					lblSaveDate.Visible = false;
					return;
				}

				if(_recentGame == value) {
					return;
				}

				_recentGame = value;

				if(this.Mode == GameScreenMode.RecentGames) {
					lblGameName.Text = !string.IsNullOrEmpty(value.Name) ? value.Name : Path.GetFileNameWithoutExtension(_recentGame.FileName);
					lblGameName.Visible = true;
				} else {
					lblGameName.Text = value.Name;
					lblGameName.Visible = !string.IsNullOrEmpty(value.Name);
				}

				bool fileExists = File.Exists(_recentGame.FileName);
				if(fileExists) {
					lblSaveDate.Text = new FileInfo(_recentGame.FileName).LastWriteTime.ToString();
				} else {
					lblSaveDate.Text = ResourceHelper.GetMessage("EmptyState");
					picPreviousState.Image = null;
				}
				this.Enabled = fileExists || this.Mode == GameScreenMode.SaveState;

				lblSaveDate.Visible = true;
				picPreviousState.Visible = true;

				if(fileExists) {
					Task.Run(() => {
						Image img = null;
						try {
							if(this.Mode != GameScreenMode.RecentGames && Path.GetExtension(value.FileName) == ".mss") {
								img = EmuApi.GetSaveStatePreview(value.FileName);
							} else {
								ZipArchive zip = new ZipArchive(new MemoryStream(File.ReadAllBytes(value.FileName)));
								ZipArchiveEntry entry = zip.GetEntry("Screenshot.png");
								if(entry != null) {
									using(Stream stream = entry.Open()) {
										img = Image.FromStream(stream);
									}
								}
								using(StreamReader sr = new StreamReader(zip.GetEntry("RomInfo.txt").Open())) {
									sr.ReadLine(); //skip first line (rom name)
									value.RomPath = sr.ReadLine();
								}
							}
						} catch { }

						this.BeginInvoke((Action)(() => {
							picPreviousState.Image = img;
						}));
					});
				}
			}
		}

		public bool Highlight
		{
			set { picPreviousState.Highlight = value; }
		}

		private void picPreviousState_Click(object sender, EventArgs e)
		{
			ProcessClick();
		}

		public void ProcessClick()
		{
			if(!this.Enabled) {
				return;
			}

			if(Path.GetExtension(_recentGame.FileName) == ".rgd") {
				 EmuRunner.LoadRecentGame(_recentGame.FileName);
			} else {
				switch(this.Mode) {
					case GameScreenMode.LoadState: EmuApi.LoadState(_recentGame.SaveSlot); break;
					case GameScreenMode.SaveState: EmuApi.SaveState(_recentGame.SaveSlot); break;
				}
			}
			OnRecentGameLoaded?.Invoke(_recentGame);
		}
	}

	public class GamePreviewBox : PictureBox
	{
		public InterpolationMode InterpolationMode { get; set; }
		private bool _hovered = false;
		private bool _highlight = false;

		public GamePreviewBox()
		{
			DoubleBuffered = true;
			InterpolationMode = InterpolationMode.Default;
		}

		public bool Highlight
		{
			get { return _highlight; }
			set
			{
				_highlight = value;
				this.Invalidate();
			}
		}

		protected override void OnEnabledChanged(EventArgs e)
		{
			base.OnEnabledChanged(e);
			_hovered = false;
			this.Invalidate();
		}

		protected override void OnMouseEnter(EventArgs e)
		{
			base.OnMouseEnter(e);
			_hovered = true;
			this.Invalidate();
		}

		protected override void OnMouseLeave(EventArgs e)
		{
			base.OnMouseLeave(e);
			_hovered = false;
			this.Invalidate();
		}

		protected override void OnPaint(PaintEventArgs pe)
		{
			pe.Graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
			base.OnPaint(pe);

			using(Pen pen = new Pen(_hovered && this.Enabled ? Color.DeepSkyBlue : (_highlight ? Color.DodgerBlue : Color.DimGray), 2)) {
				pe.Graphics.DrawRectangle(pen, 1, 1, this.Width - 2, this.Height - 2);
			}
		}
	}
}
