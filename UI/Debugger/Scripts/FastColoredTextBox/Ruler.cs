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
    public partial class Ruler : UserControl
    {
        public EventHandler TargetChanged;

        [DefaultValue(typeof(Color), "ControlLight")]
        public Color BackColor2 { get; set; }

        [DefaultValue(typeof(Color), "DarkGray")]
        public Color TickColor { get; set; }

        [DefaultValue(typeof(Color), "Black")]
        public Color CaretTickColor { get; set; }

        FastColoredTextBox target;

        [Description("Target FastColoredTextBox")]
        public FastColoredTextBox Target
        {
            get { return target; }
            set
            {
                if (target != null)
                    UnSubscribe(target);
                target = value;
                Subscribe(target);
                OnTargetChanged();
            }
        }

        public Ruler()
        {
            InitializeComponent();

            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint, true);
            MinimumSize = new Size(0, 24);
            MaximumSize = new Size(int.MaxValue/2, 24);

            BackColor2 = SystemColors.ControlLight;
            TickColor = Color.DarkGray;
            CaretTickColor = Color.Black;
        }



        protected virtual void OnTargetChanged()
        {
            if (TargetChanged != null)
                TargetChanged(this, EventArgs.Empty);
        }

        protected virtual void UnSubscribe(FastColoredTextBox target)
        {
            target.Scroll -= new ScrollEventHandler(target_Scroll);
            target.SelectionChanged -= new EventHandler(target_SelectionChanged);
            target.VisibleRangeChanged -= new EventHandler(target_VisibleRangeChanged);
        }

        protected virtual void Subscribe(FastColoredTextBox target)
        {
            target.Scroll += new ScrollEventHandler(target_Scroll);
            target.SelectionChanged += new EventHandler(target_SelectionChanged);
            target.VisibleRangeChanged += new EventHandler(target_VisibleRangeChanged);
        }

        void target_VisibleRangeChanged(object sender, EventArgs e)
        {
            Invalidate();
        }

        void target_SelectionChanged(object sender, EventArgs e)
        {
            Invalidate();
        }

        protected virtual void target_Scroll(object sender, ScrollEventArgs e)
        {
            Invalidate();
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            Invalidate();
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (target == null)
                return;

            Point car = PointToClient(target.PointToScreen(target.PlaceToPoint(target.Selection.Start)));

            Size fontSize = TextRenderer.MeasureText("W", Font);

            int column = 0;
            e.Graphics.FillRectangle(new LinearGradientBrush(new Rectangle(0, 0, Width, Height), BackColor, BackColor2, 270), new Rectangle(0, 0, Width, Height));

            float columnWidth = target.CharWidth;
            var sf = new StringFormat();
            sf.Alignment = StringAlignment.Center;
            sf.LineAlignment = StringAlignment.Near;

            var zeroPoint = target.PositionToPoint(0);
            zeroPoint = PointToClient(target.PointToScreen(zeroPoint));

            using (var pen = new Pen(TickColor))
            using (var textBrush = new SolidBrush(ForeColor))
            for (float x = zeroPoint.X; x < Right; x += columnWidth, ++column)
            {
                if (column % 10 == 0)
                    e.Graphics.DrawString(column.ToString(), Font, textBrush, x, 0f, sf);

                e.Graphics.DrawLine(pen, (int)x, fontSize.Height + (column % 5 == 0 ? 1 : 3), (int)x, Height - 4);
            }

            using (var pen = new Pen(TickColor))
                e.Graphics.DrawLine(pen, new Point(car.X - 3, Height - 3), new Point(car.X + 3, Height - 3));

            using (var pen = new Pen(CaretTickColor))
            {
                e.Graphics.DrawLine(pen, new Point(car.X - 2, fontSize.Height + 3), new Point(car.X - 2, Height - 4));
                e.Graphics.DrawLine(pen, new Point(car.X, fontSize.Height + 1), new Point(car.X, Height - 4));
                e.Graphics.DrawLine(pen, new Point(car.X + 2, fontSize.Height + 3), new Point(car.X + 2, Height - 4));
            }
        }
    }
}
