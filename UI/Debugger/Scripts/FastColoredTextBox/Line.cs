using System.Collections.Generic;
using System;
using System.Text;
using System.Drawing;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Line of text
    /// </summary>
    public class Line : IList<Char>
    {
        protected List<Char> chars;

        public string FoldingStartMarker { get; set; }
        public string FoldingEndMarker { get; set; }
        /// <summary>
        /// Text of line was changed
        /// </summary>
        public bool IsChanged { get; set; }
        /// <summary>
        /// Time of last visit of caret in this line
        /// </summary>
        /// <remarks>This property can be used for forward/backward navigating</remarks>
        public DateTime LastVisit { get; set; }
        /// <summary>
        /// Background brush.
        /// </summary>
        public Brush BackgroundBrush { get; set;}
        /// <summary>
        /// Unique ID
        /// </summary>
        public int UniqueId { get; private set; }
        /// <summary>
        /// Count of needed start spaces for AutoIndent
        /// </summary>
        public int AutoIndentSpacesNeededCount
        {
            get;
            set;
        }

        internal Line(int uid)
        {
            this.UniqueId = uid;
            chars = new List<Char>();
        }


        /// <summary>
        /// Clears style of chars, delete folding markers
        /// </summary>
        public void ClearStyle(StyleIndex styleIndex)
        {
            FoldingStartMarker = null;
            FoldingEndMarker = null;
            for (int i = 0; i < Count; i++)
            {
                Char c = this[i];
                c.style &= ~styleIndex;
                this[i] = c;
            }
        }

        /// <summary>
        /// Text of the line
        /// </summary>
        public virtual string Text
        {
            get{
                StringBuilder sb = new StringBuilder(Count);
                foreach(Char c in this)
                    sb.Append(c.c);
                return sb.ToString();
            }
        }

        /// <summary>
        /// Clears folding markers
        /// </summary>
        public void ClearFoldingMarkers()
        {
            FoldingStartMarker = null;
            FoldingEndMarker = null;
        }

        /// <summary>
        /// Count of start spaces
        /// </summary>
        public int StartSpacesCount
        {
            get
            {
                int spacesCount = 0;
                for (int i = 0; i < Count; i++)
                    if (this[i].c == ' ')
                        spacesCount++;
                    else
                        break;
                return spacesCount;
            }
        }

        public int IndexOf(Char item)
        {
            return chars.IndexOf(item);
        }

        public void Insert(int index, Char item)
        {
            chars.Insert(index, item);
        }

        public void RemoveAt(int index)
        {
            chars.RemoveAt(index);
        }

        public Char this[int index]
        {
            get
            {
                return chars[index];
            }
            set
            {
                chars[index] = value;
            }
        }

        public void Add(Char item)
        {
            chars.Add(item);
        }

        public void Clear()
        {
            chars.Clear();
        }

        public bool Contains(Char item)
        {
            return chars.Contains(item);
        }

        public void CopyTo(Char[] array, int arrayIndex)
        {
            chars.CopyTo(array, arrayIndex);
        }

        /// <summary>
        /// Chars count
        /// </summary>
        public int Count
        {
            get { return chars.Count; }
        }

        public bool IsReadOnly
        {
            get {  return false; }
        }

        public bool Remove(Char item)
        {
            return chars.Remove(item);
        }

        public IEnumerator<Char> GetEnumerator()
        {
            return chars.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return chars.GetEnumerator() as System.Collections.IEnumerator;
        }

        public virtual void RemoveRange(int index, int count)
        {
            if (index >= Count)
                return;
            chars.RemoveRange(index, Math.Min(Count - index, count));
        }

        public virtual void TrimExcess()
        {
            chars.TrimExcess();
        }

        public virtual void AddRange(IEnumerable<Char> collection)
        {
            chars.AddRange(collection);
        }
    }

    public struct LineInfo
    {
        List<int> cutOffPositions;
        //Y coordinate of line on screen
        internal int startY;
        internal int bottomPadding;
        //indent of secondary wordwrap strings (in chars)
        internal int wordWrapIndent;
        /// <summary>
        /// Visible state
        /// </summary>
        public VisibleState VisibleState;

        public LineInfo(int startY)
        {
            cutOffPositions = null;
            VisibleState = VisibleState.Visible;
            this.startY = startY;
            bottomPadding = 0;
            wordWrapIndent = 0;
        }
        /// <summary>
        /// Positions for wordwrap cutoffs
        /// </summary>
        public List<int> CutOffPositions
        {
            get
            {
                if (cutOffPositions == null)
                    cutOffPositions = new List<int>();
                return cutOffPositions;
            }
        }

        /// <summary>
        /// Count of wordwrap string count for this line
        /// </summary>
        public int WordWrapStringsCount
        {
            get
            {
                switch (VisibleState)
                {
                    case VisibleState.Visible:
                         if (cutOffPositions == null)
                            return 1;
                         else
                            return cutOffPositions.Count + 1;
                    case VisibleState.Hidden: return 0;
                    case VisibleState.StartOfHiddenBlock: return 1;
                }

                return 0;
            }
        }

        internal int GetWordWrapStringStartPosition(int iWordWrapLine)
        {
            return iWordWrapLine == 0 ? 0 : CutOffPositions[iWordWrapLine - 1];
        }

        internal int GetWordWrapStringFinishPosition(int iWordWrapLine, Line line)
        {
            if (WordWrapStringsCount <= 0)
                return 0;
            return iWordWrapLine == WordWrapStringsCount - 1 ? line.Count - 1 : CutOffPositions[iWordWrapLine] - 1;
        }

        /// <summary>
        /// Gets index of wordwrap string for given char position
        /// </summary>
        public int GetWordWrapStringIndex(int iChar)
        {
            if (cutOffPositions == null || cutOffPositions.Count == 0) return 0;
            for (int i = 0; i < cutOffPositions.Count; i++)
                if (cutOffPositions[i] >/*>=*/ iChar)
                    return i;
            return cutOffPositions.Count;
        }
    }

    public enum VisibleState: byte
    {
        Visible, StartOfHiddenBlock, Hidden
    }

    public enum IndentMarker
    {
        None,
        Increased,
        Decreased
    }
}
