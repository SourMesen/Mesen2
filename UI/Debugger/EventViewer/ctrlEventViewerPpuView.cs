using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Runtime.InteropServices;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;
using Mesen.GUI.Config;
using System.Drawing.Imaging;
using Mesen.GUI.Debugger.Labels;
using System.Collections.ObjectModel;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlEventViewerPpuView : BaseControl
	{
		public const int HdmaChannelFlag = 0x40;

		private int _baseWidth = 1364 / 2;

		private Point _lastPos = new Point(-1, -1);
		private bool _needUpdate = false;
		private Bitmap _screenBitmap = null;
		private Bitmap _overlayBitmap = null;
		private Bitmap _displayBitmap = null;
		private byte[] _pictureData = null;
		private Font _overlayFont;
		private Font _smallOverlayFont;

		public UInt32 ScanlineCount { get; set; } = 262;
		public int ImageScale { get { return picViewer.ImageScale; } set { picViewer.ImageScale = value; } }

		public ctrlEventViewerPpuView()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if(!IsDesignMode) {
				tmrOverlay.Start();
				_overlayFont = new Font(BaseControl.MonospaceFontFamily, 10);
				_smallOverlayFont = new Font(BaseControl.MonospaceFontFamily, 8);
			}
		}
		
		public void RefreshViewer()
		{
			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			_pictureData = DebugApi.GetEventViewerOutput(ScanlineCount, options);

			int picHeight = (int)ScanlineCount*2;
			if(_screenBitmap == null || _screenBitmap.Height != picHeight) {
				_screenBitmap = new Bitmap(_baseWidth, picHeight, PixelFormat.Format32bppPArgb);
				_overlayBitmap = new Bitmap(_baseWidth, picHeight, PixelFormat.Format32bppPArgb);
				_displayBitmap = new Bitmap(_baseWidth, picHeight, PixelFormat.Format32bppPArgb);
			}

			GCHandle handle = GCHandle.Alloc(this._pictureData, GCHandleType.Pinned);
			try {
				Bitmap source = new Bitmap(_baseWidth, (int)ScanlineCount*2, _baseWidth*4, PixelFormat.Format32bppPArgb, handle.AddrOfPinnedObject());
				using(Graphics g = Graphics.FromImage(_screenBitmap)) {
					g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
					g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
					g.PixelOffsetMode = System.Drawing.Drawing2D.PixelOffsetMode.None;
					g.DrawImageUnscaled(source, 0, 0);
				}
			} finally {
				handle.Free();
			}

			UpdateDisplay(true);
		}

		private void UpdateDisplay(bool forceUpdate)
		{
			if(!_needUpdate && !forceUpdate) {
				return;
			}

			using(Graphics g = Graphics.FromImage(_displayBitmap)) {
				g.DrawImage(_screenBitmap, 0, 0);
				g.DrawImage(_overlayBitmap, 0, 0);

				if(_lastPos.X >= 0) {
					string location = _lastPos.X + ", " + _lastPos.Y;
					SizeF size = g.MeasureString(location, _overlayFont);
					int x = _lastPos.X + 5;
					int y = _lastPos.Y - (int)size.Height / 2 - 5;
					
					if(x/2 - picViewer.ScrollOffsets.X / picViewer.ImageScale + size.Width > (picViewer.Width / picViewer.ImageScale) - 5) {
						x -= (int)size.Width * 2 + 10;
					}
					if(y*2 - picViewer.ScrollOffsets.Y / picViewer.ImageScale < size.Height + 5) {
						y = _lastPos.Y + 5;
					}

					g.DrawOutlinedString(location, _overlayFont, Brushes.Black, Brushes.White, x/2, y*2);

					location = GetCycle(_lastPos.X).ToString();
					g.DrawOutlinedString(location, _smallOverlayFont, Brushes.Black, Brushes.White, x/2, y*2 + (int)size.Height - 5);
				}
			}

			picViewer.ImageSize = new Size(_baseWidth, (int)ScanlineCount*2);
			picViewer.Image = _displayBitmap;
			_needUpdate = false;
		}

		private void UpdateOverlay(Point p)
		{
			Point pos = GetHClockAndScanline(p);

			if(_lastPos == pos) {
				//Same x,y location, no need to update
				return;
			}

			using(Graphics g = Graphics.FromImage(_overlayBitmap)) {
				g.Clear(Color.Transparent);
				using(Pen bg = new Pen(Color.FromArgb(128, Color.LightGray))) {
					g.DrawRectangle(bg, pos.X / 2 - 1, 0, 3, _overlayBitmap.Height);
					g.DrawRectangle(bg, 0, pos.Y * 2 - 1, _overlayBitmap.Width, 3);
				}
			}

			_needUpdate = true;
			_lastPos = pos;
		}

		private void ClearOverlay()
		{
			using(Graphics g = Graphics.FromImage(_overlayBitmap)) {
				g.Clear(Color.Transparent);
			}
			UpdateDisplay(false);
			_lastPos = new Point(-1, -1);
		}

		private Point GetHClockAndScanline(Point location)
		{
			return new Point(
				(location.X / this.ImageScale) * 2,
				((location.Y & ~0x01) / this.ImageScale) / 2
			);
		}

		private int GetCycle(int hClock)
		{
			if(hClock <= 1292) {
				return hClock >> 2;
			} else if(hClock <= 1310) {
				return (hClock - 2) >> 2;
			} else {
				return (hClock - 4) >> 2;
			}
		}

		private void picPicture_MouseMove(object sender, MouseEventArgs e)
		{
			Point pos = GetHClockAndScanline(e.Location);
			if(_lastPos == pos) {
				return;
			}

			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			DebugEventInfo evt = new DebugEventInfo();
			DebugApi.GetEventViewerEvent(ref evt, (UInt16)pos.Y, (UInt16)pos.X, options);
			if(evt.ProgramCounter == 0xFFFFFFFF) {
				ResetTooltip();
				UpdateOverlay(e.Location);
				return;
			}

			Dictionary<string, string> values = new Dictionary<string, string>() {
				{ "Type", ResourceHelper.GetEnumText(evt.Type) },
				{ "Scanline", evt.Scanline.ToString() },
				{ "H-Clock", evt.Cycle.ToString() + " (" + GetCycle(evt.Cycle) + ")" },
				{ "PC", "$" + evt.ProgramCounter.ToString("X6") },
			};

			switch(evt.Type) {
				case DebugEventType.Register:
					bool isWrite = evt.Operation.Type == MemoryOperationType.Write || evt.Operation.Type == MemoryOperationType.DmaWrite;
					bool isDma = evt.Operation.Type == MemoryOperationType.DmaWrite|| evt.Operation.Type == MemoryOperationType.DmaRead;

					CodeLabel label = LabelManager.GetLabel(new AddressInfo() { Address = (int)evt.Operation.Address, Type = SnesMemoryType.CpuMemory });
					string registerText = "$" + evt.Operation.Address.ToString("X4");
					if(label != null) {
						registerText = label.Label + " (" + registerText + ")";
					}

					values["Register"] = registerText + (isWrite ? " (Write)" : " (Read)") + (isDma ? " (DMA)" : "");
					values["Value"] = "$" + evt.Operation.Value.ToString("X2");

					if(isDma) {
						bool indirectHdma = false;
						values["Channel"] = (evt.DmaChannel & 0x07).ToString();

						if((evt.DmaChannel & ctrlEventViewerPpuView.HdmaChannelFlag) != 0) {
							indirectHdma = evt.DmaChannelInfo.HdmaIndirectAddressing;
							values["Channel"] += indirectHdma ? " (Indirect HDMA)" : " (HDMA)";
							values["Line Counter"] = "$" + evt.DmaChannelInfo.HdmaLineCounterAndRepeat.ToString("X2");
						}

						values["Mode"] = evt.DmaChannelInfo.TransferMode.ToString();

						int aBusAddress;
						if(indirectHdma) {
							aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.TransferSize;
						} else {
							aBusAddress = (evt.DmaChannelInfo.SrcBank << 16) | evt.DmaChannelInfo.SrcAddress;
						}

						if(!evt.DmaChannelInfo.InvertDirection) {
							values["Transfer"] = "$" + aBusAddress.ToString("X4") + " -> $" + evt.DmaChannelInfo.DestAddress.ToString("X2");
						} else {
							values["Transfer"] = "$" + aBusAddress.ToString("X4") + " <- $" + evt.DmaChannelInfo.DestAddress.ToString("X2");
						}					
					}
					break;

				case DebugEventType.Breakpoint:
					ReadOnlyCollection<Breakpoint> breakpoints = BreakpointManager.Breakpoints;
					if(evt.BreakpointId >= 0 && evt.BreakpointId < breakpoints.Count) {
						Breakpoint bp = breakpoints[evt.BreakpointId];
						values["CPU Type"] = ResourceHelper.GetEnumText(bp.CpuType);
						values["BP Type"] = bp.ToReadableType();
						values["BP Addresses"] = bp.GetAddressString(true);
						if(bp.Condition.Length > 0) {
							values["BP Condition"] = bp.Condition;
						}
					}
					break;
			}

			UpdateOverlay(new Point((int)(evt.Cycle / 2 * this.ImageScale), (int)(evt.Scanline * 2 * this.ImageScale)));

			Form parentForm = this.FindForm();
			Point location = parentForm.PointToClient(Control.MousePosition);
			BaseForm.GetPopupTooltip(parentForm).SetTooltip(location, values);
		}

		private void ResetTooltip()
		{
			BaseForm.GetPopupTooltip(this.FindForm()).Visible = false;
		}

		private void picPicture_MouseLeave(object sender, EventArgs e)
		{
			ResetTooltip();
			ClearOverlay();
		}

		private void tmrOverlay_Tick(object sender, EventArgs e)
		{
			UpdateDisplay(false);
		}

		public void ZoomIn()
		{
			picViewer.ZoomIn();
		}

		public void ZoomOut()
		{
			picViewer.ZoomOut();
		}
	}
}
