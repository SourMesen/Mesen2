using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Drawing.Drawing2D;
using System.Text;
using System.Windows.Forms;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Shows document map of FCTB
    /// </summary>
    public class DocumentMap : Control
    {
        public EventHandler TargetChanged;

        FastColoredTextBox target;
        private float scale = 0.3f;
        private bool needRepaint = true;
        private Place startPlace = Place.Empty;
        private bool scrollbarVisible = true;

        [Description("Target FastColoredTextBox")]
        public FastColoredTextBox Target
        {
            get { return target; }
            set
            {
                if (target != null)
                    UnSubscribe(target);

                target = value;
                if (value != null)
                {
                    Subscribe(target);
                }
                OnTargetChanged();
            }
        }

        /// <summary>
        /// Scale
        /// </summary>
        [Description("Scale")]
        [DefaultValue(0.3f)]
        public new float Scale
        {
            get { return scale; }
            set
            {
                scale = value;
                NeedRepaint();
            }
        }

        /// <summary>
        /// Scrollbar visibility
        /// </summary>
        [Description("Scrollbar visibility")]
        [DefaultValue(true)]
        public bool ScrollbarVisible
        {
            get { return scrollbarVisible; }
            set
            {
                scrollbarVisible = value;
                NeedRepaint();
            }
        }

        public DocumentMap()
        {
            ForeColor = Color.Maroon;
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint | ControlStyles.ResizeRedraw, true);
            Application.Idle += Application_Idle;
        }

        void Application_Idle(object sender, EventArgs e)
        {
            if(needRepaint)
                Invalidate();
        }

        protected virtual void OnTargetChanged()
        {
            NeedRepaint();

            if (TargetChanged != null)
                TargetChanged(this, EventArgs.Empty);
        }

        protected virtual void UnSubscribe(FastColoredTextBox target)
        {
            target.Scroll -= new ScrollEventHandler(Target_Scroll);
            target.SelectionChangedDelayed -= new EventHandler(Target_SelectionChanged);
            target.VisibleRangeChanged -= new EventHandler(Target_VisibleRangeChanged);
        }

        protected virtual void Subscribe(FastColoredTextBox target)
        {
            target.Scroll += new ScrollEventHandler(Target_Scroll);
            target.SelectionChangedDelayed += new EventHandler(Target_SelectionChanged);
            target.VisibleRangeChanged += new EventHandler(Target_VisibleRangeChanged);
        }

        protected virtual void Target_VisibleRangeChanged(object sender, EventArgs e)
        {
            NeedRepaint();
        }

        protected virtual void Target_SelectionChanged(object sender, EventArgs e)
        {
            NeedRepaint();
        }

        protected virtual void Target_Scroll(object sender, ScrollEventArgs e)
        {
            NeedRepaint();
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            NeedRepaint();
        }

        public void NeedRepaint()
        {
            needRepaint = true;
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (target == null)
                return;

            var zoom = this.Scale * 100 / target.Zoom;

            if (zoom <= float.Epsilon)
                return;

            //calc startPlace
            var r = target.VisibleRange;
            if (startPlace.iLine > r.Start.iLine)
                startPlace.iLine = r.Start.iLine;
            else
            {
                var endP = target.PlaceToPoint(r.End);
                endP.Offset(0, -(int)(ClientSize.Height / zoom) + target.CharHeight);
                var pp = target.PointToPlace(endP);
                if (pp.iLine > startPlace.iLine)
                    startPlace.iLine = pp.iLine;
            }
            startPlace.iChar = 0;
            //calc scroll pos
            var linesCount = target.Lines.Count;
            var sp1 = (float)r.Start.iLine / linesCount;
            var sp2 = (float)r.End.iLine / linesCount;

            //scale graphics
            e.Graphics.ScaleTransform(zoom, zoom);
            //draw text
            var size = new SizeF(ClientSize.Width / zoom, ClientSize.Height / zoom);
            target.DrawText(e.Graphics, startPlace, size.ToSize());

            //draw visible rect
            var p0 = target.PlaceToPoint(startPlace);
            var p1 = target.PlaceToPoint(r.Start);
            var p2 = target.PlaceToPoint(r.End);
            var y1 = p1.Y - p0.Y;
            var y2 = p2.Y + target.CharHeight - p0.Y;

            e.Graphics.SmoothingMode = SmoothingMode.HighQuality;

            using (var brush = new SolidBrush(Color.FromArgb(50, ForeColor)))
            using (var pen = new Pen(brush, 1 / zoom))
            {
                var rect = new Rectangle(0, y1, (int)((ClientSize.Width - 1) / zoom), y2 - y1);
                e.Graphics.FillRectangle(brush, rect);
                e.Graphics.DrawRectangle(pen, rect);
            }

            //draw scrollbar
            if (scrollbarVisible)
            {
                e.Graphics.ResetTransform();
                e.Graphics.SmoothingMode = SmoothingMode.None;

                using (var brush = new SolidBrush(Color.FromArgb(200, ForeColor)))
                {
                    var rect = new RectangleF(ClientSize.Width - 3, ClientSize.Height*sp1, 2,
                                              ClientSize.Height*(sp2 - sp1));
                    e.Graphics.FillRectangle(brush, rect);
                }
            }

            needRepaint = false;
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
                Scroll(e.Location);
            base.OnMouseDown(e);
        }

        protected override void OnMouseMove(MouseEventArgs e)
        {
            if (e.Button == System.Windows.Forms.MouseButtons.Left)
                Scroll(e.Location);
            base.OnMouseMove(e);
        }

        private void Scroll(Point point)
        {
            if (target == null)
                return;

            var zoom = this.Scale*100/target.Zoom;

            if (zoom <= float.Epsilon)
                return;

            var p0 = target.PlaceToPoint(startPlace);
            p0 = new Point(0, p0.Y + (int) (point.Y/zoom));
            var pp = target.PointToPlace(p0);
            target.DoRangeVisible(new Range(target, pp, pp), true);
            BeginInvoke((MethodInvoker)OnScroll);
        }

        private void OnScroll()
        {
            Refresh();
            target.Refresh();
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                Application.Idle -= Application_Idle;
                if (target != null)
                    UnSubscribe(target);
            }
            base.Dispose(disposing);
        }
    }
}
