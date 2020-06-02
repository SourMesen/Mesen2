using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.PpuViewer
{
	public partial class ctrlSpriteList : UserControl
	{
		public delegate void SpriteSelectedHandler(SpriteInfo sprite);
		public event SpriteSelectedHandler SpriteSelected;

		private List<SpriteInfo> _sprites;
		private DebugState _state;
		private byte[] _oamRam;
		private int _oamMode;
		private bool _isGameboyMode;
		private int _selectedIndex;
		private int _sortOrder = 1;
		private DateTime _lastRefresh = DateTime.MinValue;

		public ctrlSpriteList()
		{
			InitializeComponent();
		}

		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public bool HideOffscreenSprites
		{
			get { return chkHideOffscreenSprites.Checked; }
			set { chkHideOffscreenSprites.Checked = value; }
		}

		[Browsable(false), DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
		public void SetSelectedIndex(int index, bool scroll)
		{
			_selectedIndex = index;
			if(lstSprites.SelectedIndices.Count == 1 && lstSprites.SelectedIndices[0] < _sprites.Count && _sprites[lstSprites.SelectedIndices[0]].Index == _selectedIndex) {
				//The correct sprite is already selected, nothing to do
				return;
			}

			if(_selectedIndex < 0) {
				lstSprites.SelectedIndices.Clear();
				return;
			}

			for(int i = 0; i < _sprites.Count; i++) {
				if(_sprites[i].Index == index) {
					lstSprites.BeginUpdate();
					lstSprites.SelectedIndices.Clear();
					lstSprites.SelectedIndices.Add(i);
					if(scroll) {
						lstSprites.EnsureVisible(i);
						lstSprites.Focus();
					}
					lstSprites.EndUpdate();
					break;
				}
			}
		}

		public void SetData(DebugState state, byte[] oamRam, int oamMode, bool gameboyMode)
		{
			if(_oamRam == oamRam) {
				return;
			}

			_state = state;
			_oamRam = oamRam;
			_oamMode = oamMode;
			_isGameboyMode = gameboyMode;
			if((DateTime.Now - _lastRefresh).TotalMilliseconds > 200) {
				RefreshList();
			}
		}

		private void RefreshList()
		{
			List<SpriteInfo> sprites = new List<SpriteInfo>();
			for(int i = 0; i < (_isGameboyMode ? 40 : 128); i++) {
				SpriteInfo sprite;
				if(_isGameboyMode) {
					sprite = SpriteInfo.GetGbSpriteInfo(_oamRam, i, _state.Gameboy.Ppu.LargeSprites, _state.Gameboy.Ppu.CgbEnabled);
				} else {
					sprite = SpriteInfo.GetSpriteInfo(_oamRam, _state.Ppu.OamMode, i);
				}

				if(!chkHideOffscreenSprites.Checked || sprite.IsVisible()) {
					sprites.Add(sprite);
				}
			}

			sprites.Sort((SpriteInfo a, SpriteInfo b) => {
				int column = Math.Abs(_sortOrder);
				int order = _sortOrder >= 0 ? 1 : -1;

				int value = 0;
				if(column == 1) {
					value = (a.Index - b.Index) * order;
				} else if(column == 2) {
					value = (a.X - b.X) * order;
				} else if(column == 3) {
					value = (a.Y - b.Y) * order;
				} else if(column == 4) {
					value = ((a.LargeSprite ? 1 : 0) - (b.LargeSprite ? 1 : 0)) * order;
				} else if(column == 5) {
					value = (a.TileIndex - b.TileIndex) * order;
				} else if(column == 6) {
					value = (a.Priority - b.Priority) * order;
				} else if(column == 7) {
					value = (a.Palette - b.Palette) * order;
				} else if(column == 8) {
					value = (a.Flags - b.Flags) * order;
				}

				if(value == 0) {
					value = (a.Index - b.Index);
				}

				return value;
			});

			lstSprites.BeginUpdate();
			_sprites = sprites;
			lstSprites.VirtualListSize = _sprites.Count;
			lstSprites.EndUpdate();
			SetSelectedIndex(_selectedIndex, false);

			_lastRefresh = DateTime.Now;
		}

		private void lstSprites_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
			if(_oamRam == null) {
				return;
			}

			SpriteInfo sprite = _sprites[e.ItemIndex];
			e.Item = new ListViewItem(new string[] {
				sprite.Index.ToString(),
				sprite.X.ToString(),
				sprite.Y.ToString(),
				sprite.Width.ToString() + "x" + sprite.Height.ToString(),
				sprite.TileIndex.ToString(),
				sprite.Priority.ToString(),
				sprite.Palette.ToString(),
				(sprite.HorizontalMirror ? "H" : "") + (sprite.VerticalMirror ? "V" : "") + (sprite.UseSecondTable ? "N" : "")
			});

			if(!sprite.IsVisible()) {
				e.Item.UseItemStyleForSubItems = true;
				e.Item.ForeColor = Color.Gray;
			}
		}

		private void lstSprites_SelectedIndexChanged(object sender, EventArgs e)
		{
			if(lstSprites.SelectedIndices.Count > 0) {
				int index = lstSprites.SelectedIndices[lstSprites.SelectedIndices.Count - 1];
				SpriteSelected?.Invoke(_sprites[index]);
			} else {
				SpriteSelected?.Invoke(null);
			}
		}

		private void chkHideOffscreenSprites_Click(object sender, EventArgs e)
		{
			RefreshList();
		}

		private void lstSprites_ColumnClick(object sender, ColumnClickEventArgs e)
		{
			if(Math.Abs(_sortOrder) == e.Column + 1) {
				_sortOrder = -_sortOrder;
			} else {
				_sortOrder = e.Column + 1;
			}

			RefreshList();
		}
	}
}
