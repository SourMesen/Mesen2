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
using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using Mesen.GUI.Controls;

namespace Mesen.GUI.Controls
{
	public partial class ctrlRecentGames : BaseControl
	{
		private int _columnCount = 0;
		private int _rowCount = 0;
		private int _elementsPerPage = 0;

		private bool _needResume = false;
		private int _currentIndex = 0;
		private List<RecentGameInfo> _recentGames = new List<RecentGameInfo>();
		private List<ctrlRecentGame> _controls = new List<ctrlRecentGame>();

		public delegate void RecentGameLoadedHandler(RecentGameInfo gameInfo);
		public event RecentGameLoadedHandler OnRecentGameLoaded;

		public new event MouseEventHandler MouseMove
		{
			add { this.tlpPreviousState.MouseMove += value; this.tlpTitle.MouseMove += value; }
			remove { this.tlpPreviousState.MouseMove -= value; this.tlpTitle.MouseMove -= value; }
		}

		public new event EventHandler DoubleClick
		{
			add { this.tlpPreviousState.DoubleClick += value; this.tlpTitle.DoubleClick += value; }
			remove { this.tlpPreviousState.DoubleClick -= value; this.tlpTitle.DoubleClick -= value; }
		}

		public ctrlRecentGames()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			DoubleBuffered = true;
			picPrevGame.Image.RotateFlip(RotateFlipType.RotateNoneFlipX);
		}

		private void InitGrid()
		{
			int columnCount = 1;
			if(ClientSize.Width > 850 && ClientSize.Height > 850) {
				columnCount = 3;
			} else if(ClientSize.Width > 450 && ClientSize.Height > 450) {
				columnCount = 2;
			}

			if(_recentGames.Count <= 1) {
				columnCount = 1;
			} else if(_recentGames.Count <= 4) {
				columnCount = Math.Min(2, columnCount);
			}

			int elementsPerPage = columnCount * columnCount;
			int rowCount = columnCount;

			if(Mode != GameScreenMode.RecentGames) {
				elementsPerPage = 12;
				columnCount = 4;
				rowCount = 3;
			}

			if(_columnCount == columnCount && _elementsPerPage == elementsPerPage && _rowCount == rowCount) {
				return;
			}

			_columnCount = columnCount;
			_rowCount = rowCount;
			_elementsPerPage = elementsPerPage;

			_controls = new List<ctrlRecentGame>();
			tlpGrid.SuspendLayout();
			tlpGrid.ColumnCount = _columnCount;
			tlpGrid.RowCount = _rowCount;
			tlpGrid.ColumnStyles.Clear();
			tlpGrid.RowStyles.Clear();
			tlpGrid.Controls.Clear();
			for(int j = 0; j < _columnCount; j++) {
				tlpGrid.RowStyles.Add(new RowStyle(SizeType.Percent, 100F / _columnCount));
				tlpGrid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100F / _columnCount));
			}

			for(int j = 0; j < _rowCount; j++) {
				for(int i = 0; i < _columnCount; i++) {
					ctrlRecentGame ctrl = new ctrlRecentGame();
					ctrl.OnRecentGameLoaded += RecentGameLoaded;
					ctrl.Dock = DockStyle.Fill;
					ctrl.Margin = new Padding(2);
					tlpGrid.Controls.Add(ctrl, i, j);
					_controls.Add(ctrl);
				}
			}
			tlpGrid.ResumeLayout();
			UpdateGameInfo();

			picPrevGame.Visible = _recentGames.Count > _elementsPerPage;
			picNextGame.Visible = _recentGames.Count > _elementsPerPage;
		}

		public int GameCount
		{
			get { return _recentGames.Count; }
		}

		public GameScreenMode Mode { get; private set; } = GameScreenMode.RecentGames;

		private bool Pause()
		{
			if(!EmuApi.IsPaused()) {
				EmuApi.Pause();
				return true;
			}
			return false;
		}

		public void ShowScreen(GameScreenMode mode)
		{
			if(mode == GameScreenMode.RecentGames && ConfigManager.Config.Preferences.DisableGameSelectionScreen) {
				this.Visible = false;
				return;
			} else if(mode != GameScreenMode.RecentGames && Mode == mode && this.Visible) {
				this.Visible = false;
				if(_needResume) {
					EmuApi.Resume();
				}
				return;
			}

			Mode = mode;
			_recentGames = new List<RecentGameInfo>();
			_currentIndex = 0;

			if(mode == GameScreenMode.RecentGames) {
				_needResume = false;
				tlpTitle.Visible = false;
				List<string> files = Directory.GetFiles(ConfigManager.RecentGamesFolder, "*.rgd").OrderByDescending((file) => new FileInfo(file).LastWriteTime).ToList();
				for(int i = 0; i < files.Count && _recentGames.Count < 36; i++) {
					_recentGames.Add(new RecentGameInfo() { FileName = files[i] });
				}
			} else {
				if(!this.Visible) {
					_needResume = Pause();
				}

				lblScreenTitle.Text = mode == GameScreenMode.LoadState ? ResourceHelper.GetMessage("LoadStateDialog") : ResourceHelper.GetMessage("SaveStateDialog");
				tlpTitle.Visible = true;

				string romName = EmuApi.GetRomInfo().GetRomName();
				for(int i = 0; i < (mode == GameScreenMode.LoadState ? 11 : 10); i++) {
					_recentGames.Add(new RecentGameInfo() { FileName = Path.Combine(ConfigManager.SaveStateFolder, romName + "_" + (i + 1) + ".mss"), Name = i == 10 ? ResourceHelper.GetMessage("AutoSave") : ResourceHelper.GetMessage("SlotNumber", i+1), SaveSlot = (uint)i+1 });
				}
				if(mode == GameScreenMode.LoadState) {
					_recentGames.Add(new RecentGameInfo() { FileName = Path.Combine(ConfigManager.RecentGamesFolder, romName + ".rgd"), Name = ResourceHelper.GetMessage("LastSession") });
				}
			}

			InitGrid();

			if(_recentGames.Count == 0) {
				this.Visible = false;
				tmrInput.Enabled = false;
			} else {
				UpdateGameInfo();
				tmrInput.Enabled = true;
			}

			picPrevGame.Visible = _recentGames.Count > _elementsPerPage;
			picNextGame.Visible = _recentGames.Count > _elementsPerPage;
			this.Visible = true;
		}

		public void UpdateGameInfo()
		{
			int count = _recentGames.Count;
			int pageStart = _currentIndex / _elementsPerPage * _elementsPerPage;

			for(int i = 0; i < _elementsPerPage; i++) {
				_controls[i].Mode = Mode;
				_controls[i].RecentGame = count > pageStart + i ? _recentGames[pageStart + i] : null;
				_controls[i].Highlight = (_currentIndex % _elementsPerPage) == i;
			}
		}

		protected override void OnResize(EventArgs e)
		{
			if(Program.IsMono) {
				//Fix resize issues
				picNextGame.Dock = DockStyle.None;
				picPrevGame.Dock = DockStyle.None;
				picNextGame.Dock = DockStyle.Fill;
				picPrevGame.Dock = DockStyle.Fill;
			}

			if(this._columnCount > 0) {
				InitGrid();
			}
			base.OnResize(e);
		}

		private void picNextGame_MouseDown(object sender, MouseEventArgs e)
		{
			GoToNextPage();
		}

		private void picPrevGame_MouseDown(object sender, MouseEventArgs e)
		{
			GoToPreviousPage();
		}

		private void GoToPreviousPage()
		{
			if(_currentIndex < _elementsPerPage) {
				_currentIndex = _recentGames.Count - 1;
			} else {
				_currentIndex -= _elementsPerPage;
			}
			UpdateGameInfo();
		}

		private bool IsOnLastPage { get { return (_currentIndex / _elementsPerPage) == ((_recentGames.Count - 1) / _elementsPerPage); } }

		private void GoToNextPage()
		{
			if(_currentIndex + _elementsPerPage < _recentGames.Count) {
				_currentIndex += _elementsPerPage;
			} else {
				_currentIndex = IsOnLastPage ? 0 : (_recentGames.Count - 1);
			}
			UpdateGameInfo();
		}

		private void RecentGameLoaded(RecentGameInfo gameInfo)
		{
			if(this.Mode == GameScreenMode.RecentGames) {
				OnRecentGameLoaded?.Invoke(gameInfo);
			}
			if(this._needResume) {
				EmuApi.Resume();
			}
			this.Visible = false;
		}

		private bool _waitForRelease = false;
		private void tmrInput_Tick(object sender, EventArgs e)
		{
			//Use player 1's controls to navigate the recent game selection screen
			if(Application.OpenForms.Count > 0 && Application.OpenForms[0].ContainsFocus && this.Visible) {
				if(Mode != GameScreenMode.RecentGames && !EmuApi.IsPaused()) {
					this.Visible = false;
					return;
				}
				List<uint> keyCodes = InputApi.GetPressedKeys();
				uint keyCode = keyCodes.Count > 0 ? keyCodes[0] : 0;
				if(keyCode > 0) {
					if(!_waitForRelease) {
						List<KeyMapping> mappings = new List<KeyMapping>() {
							ConfigManager.Config.Input.Controllers[0].Keys.Mapping1,
							ConfigManager.Config.Input.Controllers[0].Keys.Mapping2,
							ConfigManager.Config.Input.Controllers[0].Keys.Mapping3,
							ConfigManager.Config.Input.Controllers[0].Keys.Mapping4
						};

						foreach(KeyMapping mapping in mappings) {
							if(mapping.Left == keyCode) {
								_waitForRelease = true;
								if(_currentIndex == 0) {
									_currentIndex = _recentGames.Count - 1;
								} else {
									_currentIndex--;
								}
								UpdateGameInfo();
							} else if(mapping.Right == keyCode) {
								_waitForRelease = true;
								_currentIndex = (_currentIndex + 1) % _recentGames.Count;
								UpdateGameInfo();
							} else if(mapping.Down == keyCode) {
								_waitForRelease = true;
								if(_currentIndex + _columnCount < _recentGames.Count) {
									_currentIndex += _columnCount;
								} else {
									_currentIndex = Math.Min(_currentIndex % _columnCount, _recentGames.Count - 1);
								}
								UpdateGameInfo();
							} else if(mapping.Up == keyCode) {
								_waitForRelease = true;
								if(_currentIndex < _columnCount) {
									_currentIndex = _recentGames.Count - (_columnCount - (_currentIndex % _columnCount));
								} else {
									_currentIndex -= _columnCount;
								}
								UpdateGameInfo();
							} else if(mapping.A == keyCode || mapping.B == keyCode || mapping.Select == keyCode || mapping.Start == keyCode) {
								_waitForRelease = true;
								_controls[_currentIndex % _elementsPerPage].ProcessClick();
							}
						}
					}
				} else {
					_waitForRelease = false;
				}
			}
		}

		private void picClose_Click(object sender, EventArgs e)
		{
			if(_needResume) {
				EmuApi.Resume();
			}
			this.Visible = false;
		}
	}

	public class DBTableLayoutPanel : TableLayoutPanel
	{
		public DBTableLayoutPanel()
		{
			DoubleBuffered = true;
		}
	}

	public class RecentGameInfo
	{
		public string FileName { get; set; }
		public string Name { get; set; }
		public uint SaveSlot { get; set; }
		public ResourcePath RomPath { get; set; }
	}

	public enum GameScreenMode
	{
		RecentGames,
		LoadState,
		SaveState
	}
}
