using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace MonsterDifficultyConfig
{
    public partial class BarGraphPanel : UserControl
    {
        List<int> values = new List<int>();
        public BarGraphPanel()
        {
            InitializeComponent();
        }

        public List<int> Values { get { return values; } }

        Brush barBrush = new SolidBrush(Color.DarkSlateBlue);
        Brush valTextBrush = new SolidBrush(Color.Turquoise);
        Brush indexTextBrush = new SolidBrush(Color.Black);
        protected override void OnPaint(PaintEventArgs e)
        {
            base.OnPaint(e);

            // draw a bar for each data point we have
            float sepWidth = BarWidth / 2;

            int bottomArea = 20;
            float heightScale = BiggestValue * 1.1f / (Height-bottomArea);

            float xStart = sepWidth * 0.5f;
            StringFormat vertical = new StringFormat(StringFormatFlags.DirectionVertical);
            for (int i = 0; i < values.Count; i++)
            {
                float height = heightScale == 0 ? 0 : values[i] / heightScale;
                float top = Height - bottomArea - height;
                float textTop = Math.Min(top, Height - bottomArea - 14);
                if (highlightedColumn == i)
                {
                    e.Graphics.FillRectangle(valTextBrush, xStart, top, BarWidth, height);
                    e.Graphics.DrawString(values[i].ToString(), this.Font, indexTextBrush, xStart - sepWidth + 2, textTop, vertical);
                }
                else
                {
                    e.Graphics.FillRectangle(barBrush, xStart, top, BarWidth, height);
                    e.Graphics.DrawString(values[i].ToString(), this.Font, valTextBrush, xStart - sepWidth + 2, textTop, vertical);
                }
                e.Graphics.DrawString(i.ToString(), new Font(this.Font, FontStyle.Regular), indexTextBrush, i>9 ? xStart - 3 : xStart, Height - bottomArea);
                xStart += BarWidth + sepWidth;
            }
        }

        private int BiggestValue
        {
            get
            {
                int biggest = 0;
                foreach (int i in values)
                    if (i > biggest)
                        biggest = i;
                return biggest;
            }
        }

        public float BarWidth
        {
            get
            {
                return (float)Width / (values.Count * 1.5f + 0.5f);
            }
        }

        // highlight the given column
        private int highlightedColumn = -1;
        public void Highlight(int col)
        {
            if (col > -1 && col < values.Count)
            {
                highlightedColumn = col;
                Invalidate();
            }
        }

        public void ClearHighlight()
        {
            highlightedColumn = -1;
            Invalidate();
        }
    }
}
