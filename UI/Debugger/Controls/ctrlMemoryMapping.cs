using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	class ctrlMemoryMapping : Control
	{
		Font _largeFont = new Font("Arial", 9);
		Font _mediumFont = new Font("Arial", 8);
		Font _smallFont = new Font("Arial", 7);

		List<MemoryRegionInfo> _regions = new List<MemoryRegionInfo>();

		public ctrlMemoryMapping()
		{
			this.DoubleBuffered = true;
			this.ResizeRedraw = true;
		}

		private void UpdateRegionArray(List<MemoryRegionInfo> regions)
		{
			if(regions.Count != _regions.Count) {
				_regions = regions;
				this.Invalidate();
			} else {
				for(int i = 0; i < regions.Count; i++) {
					if(_regions[i].Color != regions[i].Color || _regions[i].Name != regions[i].Name || _regions[i].Size != regions[i].Size) {
						_regions = regions;
						this.Invalidate();
						return;
					}
				}
			}
		}

		public void UpdateCpuRegions(GbState gbState)
		{
			GbMemoryManagerState state = gbState.MemoryManager;
			List<MemoryRegionInfo> regions = new List<MemoryRegionInfo>();

			Action<int> addEmpty = (int size) => { regions.Add(new MemoryRegionInfo() { Name = "N/A", Size = size, Color = Color.FromArgb(222, 222, 222) }); };
			
			Action<int, int, RegisterAccess> addWorkRam = (int page, int size, RegisterAccess type) => {
				string name = size >= 0x1000 ? ("WRAM ($" + page.ToString("X2") + ")") : (size >= 0x800 ? ("$" + page.ToString("X2")) : "");
				regions.Add(new MemoryRegionInfo() { Name = name, Size = size, Color = Color.FromArgb(0xCD, 0xDC, 0xFA), AccessType = type });
			};

			Action<int, int, RegisterAccess> addSaveRam = (int page, int size, RegisterAccess type) => {
				string name = size >= 0x2000 ? ("Save RAM ($" + page.ToString("X2") + ")") : (size >= 0x800 ? ("$" + page.ToString("X2")) : "");
				regions.Add(new MemoryRegionInfo() { Name = name, Size = size, Color = Color.FromArgb(0xFA, 0xDC, 0xCD), AccessType = type });
			};

			Action<int, int, RegisterAccess> addCartRam = (int page, int size, RegisterAccess type) => {
				string name = size >= 0x2000 ? ("Cart RAM ($" + page.ToString("X2") + ")") : (size >= 0x800 ? ("$" + page.ToString("X2")) : "");
				regions.Add(new MemoryRegionInfo() { Name = name, Size = size, Color = Color.FromArgb(0xFA, 0xDC, 0xCD), AccessType = type });
			};

			Action<int, int, Color> addPrgRom = (int page, int size, Color color) => { regions.Add(new MemoryRegionInfo() { Name = "$" + page.ToString("X2"), Size = size, Color = color }); };

			GbMemoryType memoryType = GbMemoryType.None;
			RegisterAccess accessType = RegisterAccess.None;
			int currentSize = 0;
			int startIndex = 0;
			bool alternateColor = true;

			const int prgBankSize = 0x4000;
			const int otherBankSize = 0x2000;

			Action<int> addSection = (int i) => {
				if(currentSize == 0) {
					return;
				}

				if(memoryType == GbMemoryType.None) {
					addEmpty(currentSize);
				} else if(memoryType == GbMemoryType.PrgRom) {
					addPrgRom((int)(state.MemoryOffset[startIndex] / prgBankSize), currentSize, alternateColor ? Color.FromArgb(0xC4, 0xE7, 0xD4) : Color.FromArgb(0xA4, 0xD7, 0xB4));
					alternateColor = !alternateColor;
				} else if(memoryType == GbMemoryType.WorkRam) {
					addWorkRam((int)(state.MemoryOffset[startIndex] / otherBankSize), currentSize, accessType);
				} else if(memoryType == GbMemoryType.CartRam) {
					if(gbState.HasBattery) {
						addSaveRam((int)(state.MemoryOffset[startIndex] / otherBankSize), currentSize, accessType);
					} else {
						addCartRam((int)(state.MemoryOffset[startIndex] / otherBankSize), currentSize, accessType);
					}
				}
				currentSize = 0;
				startIndex = i;
			};

			for(int i = 0; i < 0xFE; i++) {
				if(i == 0x80) {
					addSection(i);
					regions.Add(new MemoryRegionInfo() { Name = "VRAM", Size = 0x2000, Color = Color.FromArgb(0xFA, 0xDC, 0xCD), AccessType = RegisterAccess.ReadWrite });
					addSection(i);
					memoryType = GbMemoryType.None;
					accessType = RegisterAccess.None;
					currentSize = 0;
					i += 0x20;
				}

				if(state.MemoryAccessType[i] != RegisterAccess.None) {
					bool forceNewBlock = (
						(memoryType == GbMemoryType.PrgRom && state.MemoryOffset[i] % prgBankSize == 0) ||
						(memoryType == GbMemoryType.WorkRam && state.MemoryOffset[i] % otherBankSize == 0) ||
						(memoryType == GbMemoryType.CartRam && state.MemoryOffset[i] % otherBankSize == 0)
					);

					if(forceNewBlock || memoryType != state.MemoryType[i] || state.MemoryOffset[i] - state.MemoryOffset[i-1] != 0x100) {
						addSection(i);
					}
					memoryType = state.MemoryType[i];
					accessType = state.MemoryAccessType[i];
				} else {
					if(memoryType != GbMemoryType.None) {
						addSection(i);
					}
					memoryType = GbMemoryType.None;
					accessType = RegisterAccess.None;
				}
				currentSize += 0x100;
			}

			regions.Add(new MemoryRegionInfo() { Name = "", Size = 0x200, Color = Color.FromArgb(222, 222, 222) });

			UpdateRegionArray(regions);
		}

		protected override void OnPaint(PaintEventArgs e)
		{
			base.OnPaint(e);

			e.Graphics.Clear(Color.LightGray);

			if(_regions.Count > 0) {
				Rectangle rect = Rectangle.Inflate(this.ClientRectangle, -2, -1);

				int totalSize = 0;
				foreach(MemoryRegionInfo region in _regions) {
					totalSize += region.Size;
				}

				float pixelsPerByte = (float)rect.Width / totalSize;

				float currentPosition = 1;
				int byteOffset = 0;
				foreach(MemoryRegionInfo region in _regions) {
					float length = pixelsPerByte * region.Size;
					using(Brush brush = new SolidBrush(region.Color)) {
						e.Graphics.FillRectangle(brush, currentPosition, 0, length, rect.Height);
					}
					e.Graphics.DrawRectangle(Pens.Black, currentPosition, 0, length, rect.Height);

					if(region.Size > 0x200) {
						e.Graphics.RotateTransform(-90);
						SizeF textSize = e.Graphics.MeasureString(byteOffset.ToString("X4"), _mediumFont);
						e.Graphics.DrawString(byteOffset.ToString("X4"), _mediumFont, Brushes.Black, -rect.Height + (rect.Height - textSize.Width) / 2, currentPosition + 3);
						e.Graphics.ResetTransform();

						textSize = e.Graphics.MeasureString(region.Name, _largeFont);
						e.Graphics.DrawString(region.Name, _largeFont, Brushes.Black, currentPosition + 12 + ((length - 12) / 2 - textSize.Width / 2), rect.Height / 2 - 7);

						if(region.AccessType != RegisterAccess.None) {
							string accessTypeString = "";
							if(((int)region.AccessType & (int)RegisterAccess.Read) != 0) {
								accessTypeString += "R";
							}
							if(((int)region.AccessType & (int)RegisterAccess.Write) != 0) {
								accessTypeString += "W";
							}
							if(string.IsNullOrWhiteSpace(accessTypeString)) {
								//Mark it as "open bus" if it cannot be written/read
								accessTypeString = "OB";
							}

							SizeF size = e.Graphics.MeasureString(accessTypeString, _smallFont);
							e.Graphics.DrawString(accessTypeString, _smallFont, Brushes.Black, currentPosition + length - size.Width - 1, rect.Height - size.Height + 2);
						}
					}
					currentPosition += length;
					byteOffset += region.Size;
				}
			}
		}
	}

	class MemoryRegionInfo
	{
		public string Name { get; set; }
		public int Size { get; set; }
		public Color Color { get; set; }
		public RegisterAccess AccessType { get; set; } = RegisterAccess.None;
	}
}
