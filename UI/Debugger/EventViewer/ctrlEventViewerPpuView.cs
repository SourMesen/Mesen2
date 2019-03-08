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

namespace Mesen.GUI.Debugger
{
	public partial class ctrlEventViewerPpuView : BaseControl
	{
		private int _baseWidth = 340 * 2;
		private int _baseHeight = 262 * 2;

		private EntityBinder _entityBinder = new EntityBinder();
		private frmInfoTooltip _tooltip = null;
		private Point _lastPos = new Point(-1, -1);
		private bool _needUpdate = false;
		private Bitmap _screenBitmap = null;
		private Bitmap _overlayBitmap = null;
		private Bitmap _displayBitmap = null;
		private byte[] _pictureData = null;
		private Font _overlayFont;
		private bool _zoomed = false;
		
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

				_entityBinder.Entity = ConfigManager.Config.Debug.EventViewer;
				_entityBinder.AddBinding(nameof(EventViewerInfo.ApuRegisterReadColor), picApuReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ApuRegisterWriteColor), picApuWrites);
				_entityBinder.AddBinding(nameof(EventViewerInfo.CpuRegisterReadColor), picCpuReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.CpuRegisterWriteColor), picCpuWrites);
				_entityBinder.AddBinding(nameof(EventViewerInfo.IrqColor), picIrq);
				_entityBinder.AddBinding(nameof(EventViewerInfo.BreakpointColor), picMarkedBreakpoints);
				_entityBinder.AddBinding(nameof(EventViewerInfo.NmiColor), picNmi);
				_entityBinder.AddBinding(nameof(EventViewerInfo.PpuRegisterReadColor), picPpuReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.PpuRegisterWriteColor), picPpuWrites);
				_entityBinder.AddBinding(nameof(EventViewerInfo.WorkRamRegisterReadColor), picWramReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.WorkRamRegisterWriteColor), picWramWrites);

				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowApuRegisterReads), chkShowApuRegisterReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowApuRegisterWrites), chkShowApuRegisterWrites);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowCpuRegisterReads), chkShowCpuRegisterReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowCpuRegisterWrites), chkShowCpuRegisterWrites);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowIrq), chkShowIrq);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowMarkedBreakpoints), chkShowMarkedBreakpoints);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowNmi), chkShowNmi);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowPpuRegisterReads), chkShowPpuRegisterReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowPpuRegisterWrites), chkShowPpuRegisterWrites);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowWorkRamRegisterReads), chkShowWorkRamRegisterReads);
				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowWorkRamRegisterWrites), chkShowWorkRamRegisterWrites);

				_entityBinder.AddBinding(nameof(EventViewerInfo.ShowPreviousFrameEvents), chkShowPreviousFrameEvents);

				_entityBinder.UpdateUI();

				RefreshData();
				RefreshViewer();
			}
		}

		protected override void OnHandleDestroyed(EventArgs e)
		{
			if(!IsDesignMode) {
				_entityBinder.UpdateObject();
				ConfigManager.ApplyChanges();
			}

			base.OnHandleDestroyed(e);
		}

		public void RefreshData()
		{
			this.BeginInvoke((Action)(() => {
				_entityBinder.UpdateObject();
			}));

			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			DebugApi.TakeEventSnapshot(options);
		}
		
		public void RefreshViewer()
		{
			_entityBinder.UpdateObject();
			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			_pictureData = DebugApi.GetEventViewerOutput(options);

			int picHeight = _baseHeight;
			if(_screenBitmap == null || _screenBitmap.Height != picHeight) {
				_screenBitmap = new Bitmap(_baseWidth, picHeight);
				_overlayBitmap = new Bitmap(_baseWidth, picHeight);
				_displayBitmap = new Bitmap(_baseWidth, picHeight);
			}

			GCHandle handle = GCHandle.Alloc(this._pictureData, GCHandleType.Pinned);
			try {
				Bitmap source = new Bitmap(_baseWidth, _baseHeight, _baseWidth*4, System.Drawing.Imaging.PixelFormat.Format32bppArgb, handle.AddrOfPinnedObject());
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
					string location = _lastPos.X / 2 + ", " + (_lastPos.Y / 2);
					SizeF size = g.MeasureString(location, _overlayFont);
					int x = _lastPos.X + 15;
					int y = _lastPos.Y - (int)size.Height - 5;
					if(x + size.Width > _displayBitmap.Width - 5) {
						x -= (int)size.Width + 20;
					}
					if(y < size.Height + 5) {
						y = _lastPos.Y + 5;
					}

					g.DrawOutlinedString(location, _overlayFont, Brushes.Black, Brushes.White, x, y);
				}
			}

			picViewer.Image = _displayBitmap;
			_needUpdate = false;
		}

		private void UpdateOverlay(Point p)
		{
			int x = ((p.X & ~0x01) / (_zoomed ? 2 : 1)) & ~0x01;
			int y = ((p.Y & ~0x01) / (_zoomed ? 2 : 1)) & ~0x01;

			if(_lastPos.X == x && _lastPos.Y == y) {
				//Same x,y location, no need to update
				return;
			}

			using(Graphics g = Graphics.FromImage(_overlayBitmap)) {
				g.Clear(Color.Transparent);
				using(Pen bg = new Pen(Color.FromArgb(128, Color.LightGray))) {
					g.DrawRectangle(bg, x - 1, 0, 3, _overlayBitmap.Height);
					g.DrawRectangle(bg, 0, y - 1, _overlayBitmap.Width, 3);
				}
			}

			_needUpdate = true;
			_lastPos = new Point(x, y);
		}

		private void ClearOverlay()
		{
			using(Graphics g = Graphics.FromImage(_overlayBitmap)) {
				g.Clear(Color.Transparent);
			}
			UpdateDisplay(false);
			_lastPos = new Point(-1, -1);
		}

		private void picPicture_MouseMove(object sender, MouseEventArgs e)
		{
			int cycle = ((e.X & ~0x01) / (_zoomed ? 2 : 1)) / 2;
			int scanline = ((e.Y & ~0x01) / (_zoomed ? 2 : 1)) / 2;

			EventViewerDisplayOptions options = ConfigManager.Config.Debug.EventViewer.GetInteropOptions();
			DebugEventInfo evt = DebugApi.GetEventViewerEvent((UInt16)scanline, (UInt16)cycle, options);
			if(evt.ProgramCounter == 0xFFFFFFFF) {
				ResetTooltip();
				UpdateOverlay(e.Location);
				return;
			}

			Dictionary<string, string> values = new Dictionary<string, string>() {
				{ "Type", ResourceHelper.GetEnumText(evt.Type) },
				{ "Scanline", evt.Scanline.ToString() },
				{ "Cycle", evt.Cycle.ToString() },
				{ "PC", "$" + evt.ProgramCounter.ToString("X6") },
			};

			switch(evt.Type) {
				case DebugEventType.Register:
					bool isWrite = evt.Operation.Type == MemoryOperationType.Write || evt.Operation.Type == MemoryOperationType.DmaWrite;
					bool isDma = evt.Operation.Type == MemoryOperationType.DmaWrite|| evt.Operation.Type == MemoryOperationType.DmaRead;
					values["Register"] = "$" + evt.Operation.Address.ToString("X4") + (isWrite ? " (Write)" : " (Read)") + (isDma ? " (DMA)" : "");
					values["Value"] = "$" + evt.Operation.Value.ToString("X2");
					break;

				case DebugEventType.Breakpoint:
					//TODO
					/*ReadOnlyCollection<Breakpoint> breakpoints = BreakpointManager.Breakpoints;
					if(debugEvent.BreakpointId >= 0 && debugEvent.BreakpointId < breakpoints.Count) {
						Breakpoint bp = breakpoints[debugEvent.BreakpointId];
						values["BP Type"] = bp.ToReadableType();
						values["BP Addresses"] = bp.GetAddressString(true);
						if(bp.Condition.Length > 0) {
							values["BP Condition"] = bp.Condition;
						}
					}*/
					break;
			}

			double scale = _zoomed ? 2 : 1;
			UpdateOverlay(new Point((int)(evt.Cycle * 2 * scale), (int)(evt.Scanline * 2 * scale)));

			ResetTooltip();
			Form parentForm = this.FindForm();
			_tooltip = new frmInfoTooltip(parentForm, values, 10);
			_tooltip.FormClosed += (s, ev) => { _tooltip = null; };
			Point location = picViewer.PointToScreen(e.Location);
			location.Offset(10, 10);
			_tooltip.SetFormLocation(location, this);
		}

		private void ResetTooltip()
		{
			if(_tooltip != null) {
				_tooltip.Close();
				_tooltip = null;
			}
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

		private void picPicture_DoubleClick(object sender, EventArgs e)
		{
			_zoomed = !_zoomed;
			UpdateViewerSize();
		}

		private void UpdateViewerSize()
		{
			picViewer.Width = (_zoomed ? _baseWidth * 2 : _baseWidth) + 2;
			picViewer.Height = (_zoomed ? _baseHeight * 2 : _baseHeight) + 2;
		}

		private void chkOption_Click(object sender, EventArgs e)
		{
			RefreshViewer();
		}
	}
}
