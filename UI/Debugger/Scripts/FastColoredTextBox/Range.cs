using System;
using System.Text;
using System.Drawing;
using System.Text.RegularExpressions;
using System.Collections.Generic;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Diapason of text chars
    /// </summary>
    public class Range : IEnumerable<Place>
    {
        Place start;
        Place end;
        public readonly FastColoredTextBox tb;
        int preferedPos = -1;
        int updating = 0;

        string cachedText;
        List<Place> cachedCharIndexToPlace;
        int cachedTextVersion = -1;

        /// <summary>
        /// Constructor
        /// </summary>
        public Range(FastColoredTextBox tb)
        {
            this.tb = tb;
        }

        /// <summary>
        /// Return true if no selected text
        /// </summary>
        public virtual bool IsEmpty
        {
            get
            {
                if (ColumnSelectionMode)
                    return Start.iChar == End.iChar;
                return Start == End;
            }
        }

        private bool columnSelectionMode;

        /// <summary>
        /// Column selection mode
        /// </summary>
        public bool ColumnSelectionMode
        {
            get { return columnSelectionMode; }
            set { columnSelectionMode = value; }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public Range(FastColoredTextBox tb, int iStartChar, int iStartLine, int iEndChar, int iEndLine)
            : this(tb)
        {
            start = new Place(iStartChar, iStartLine);
            end = new Place(iEndChar, iEndLine);
        }

        /// <summary>
        /// Constructor
        /// </summary>
        public Range(FastColoredTextBox tb, Place start, Place end)
            : this(tb)
        {
            this.start = start;
            this.end = end;
        }

        /// <summary>
        /// Constructor. Creates range of the line
        /// </summary>
        public Range(FastColoredTextBox tb, int iLine)
            : this(tb)
        {
            start = new Place(0, iLine);
            end = new Place(tb[iLine].Count, iLine);
        }

        public bool Contains(Place place)
        {
            if (place.iLine < Math.Min(start.iLine, end.iLine)) return false;
            if (place.iLine > Math.Max(start.iLine, end.iLine)) return false;

            Place s = start;
            Place e = end;
            //normalize start and end
            if (s.iLine > e.iLine || (s.iLine == e.iLine && s.iChar > e.iChar))
            {
                var temp = s;
                s = e;
                e = temp;
            }

            if (columnSelectionMode)
            {
                if (place.iChar < s.iChar || place.iChar > e.iChar) return false;
            }
            else
            {
                if (place.iLine == s.iLine && place.iChar < s.iChar) return false;
                if (place.iLine == e.iLine && place.iChar > e.iChar) return false;
            }

            return true;
        }

        /// <summary>
        /// Returns intersection with other range,
        /// empty range returned otherwise
        /// </summary>
        /// <param name="range"></param>
        /// <returns></returns>
        public virtual Range GetIntersectionWith(Range range)
        {
            if (ColumnSelectionMode)
                return GetIntersectionWith_ColumnSelectionMode(range);

            Range r1 = this.Clone();
            Range r2 = range.Clone();
            r1.Normalize();
            r2.Normalize();
            Place newStart = r1.Start > r2.Start ? r1.Start : r2.Start;
            Place newEnd = r1.End < r2.End ? r1.End : r2.End;
            if (newEnd < newStart) 
                return new Range(tb, start, start);
            return tb.GetRange(newStart, newEnd);
        }

        /// <summary>
        /// Returns union with other range.
        /// </summary>
        /// <param name="range"></param>
        /// <returns></returns>
        public Range GetUnionWith(Range range)
        {
            Range r1 = this.Clone();
            Range r2 = range.Clone();
            r1.Normalize();
            r2.Normalize();
            Place newStart = r1.Start < r2.Start ? r1.Start : r2.Start;
            Place newEnd = r1.End > r2.End ? r1.End : r2.End;

            return tb.GetRange(newStart, newEnd);
        }

        /// <summary>
        /// Select all chars of control
        /// </summary>
        public void SelectAll()
        {
            ColumnSelectionMode = false;

            Start = new Place(0, 0);
            if (tb.LinesCount == 0)
                Start = new Place(0, 0);
            else
            {
                end = new Place(0, 0);
                start = new Place(tb[tb.LinesCount - 1].Count, tb.LinesCount - 1);
            }
            if (this == tb.Selection)
                tb.Invalidate();
        }

        /// <summary>
        /// Start line and char position
        /// </summary>
        public Place Start
        {
            get { return start; }
            set
            {
                end = start = value;
                preferedPos = -1;
                OnSelectionChanged();
            }
        }

        /// <summary>
        /// Finish line and char position
        /// </summary>
        public Place End
        {
            get
            {
                return end;
            }
            set
            {
                end = value;
                OnSelectionChanged();
            }
        }

        /// <summary>
        /// Text of range
        /// </summary>
        /// <remarks>This property has not 'set' accessor because undo/redo stack works only with 
        /// FastColoredTextBox.Selection range. So, if you want to set text, you need to use FastColoredTextBox.Selection
        /// and FastColoredTextBox.InsertText() mehtod.
        /// </remarks>
        public virtual string Text
        {
            get
            {
                if (ColumnSelectionMode)
                    return Text_ColumnSelectionMode;

                int fromLine = Math.Min(end.iLine, start.iLine);
                int toLine = Math.Max(end.iLine, start.iLine);
                int fromChar = FromX;
                int toChar = ToX;
                if (fromLine < 0) return null;
                //
                StringBuilder sb = new StringBuilder();
                for (int y = fromLine; y <= toLine; y++)
                {
                    int fromX = y == fromLine ? fromChar : 0;
                    int toX = y == toLine ? Math.Min(tb[y].Count - 1, toChar - 1) : tb[y].Count - 1;
                    for (int x = fromX; x <= toX; x++)
                        sb.Append(tb[y][x].c);
                    if (y != toLine && fromLine != toLine)
                        sb.AppendLine();
                }
                return sb.ToString();
            }
        }

        internal void GetText(out string text, out List<Place> charIndexToPlace)
        {
            //try get cached text
            if (tb.TextVersion == cachedTextVersion)
            {
                text = cachedText;
                charIndexToPlace = cachedCharIndexToPlace;
                return;
            }
            //
            int fromLine = Math.Min(end.iLine, start.iLine);
            int toLine = Math.Max(end.iLine, start.iLine);
            int fromChar = FromX;
            int toChar = ToX;

            StringBuilder sb = new StringBuilder((toLine - fromLine)*50);
            charIndexToPlace = new List<Place>(sb.Capacity);
            if (fromLine >= 0)
            {
                for (int y = fromLine; y <= toLine; y++)
                {
                    int fromX = y == fromLine ? fromChar : 0;
                    int toX = y == toLine ? Math.Min(toChar - 1, tb[y].Count - 1) : tb[y].Count - 1;
                    for (int x = fromX; x <= toX; x++)
                    {
                        sb.Append(tb[y][x].c);
                        charIndexToPlace.Add(new Place(x, y));
                    }
                    if (y != toLine && fromLine != toLine)
                    foreach (char c in Environment.NewLine)
                    {
                        sb.Append(c);
                        charIndexToPlace.Add(new Place(tb[y].Count/*???*/, y));
                    }
                }
            }
            text = sb.ToString();
            charIndexToPlace.Add(End > Start ? End : Start);
            //caching
            cachedText = text;
            cachedCharIndexToPlace = charIndexToPlace;
            cachedTextVersion = tb.TextVersion;
        }

        /// <summary>
        /// Returns first char after Start place
        /// </summary>
        public char CharAfterStart
        {
            get
            {
                if (Start.iChar >= tb[Start.iLine].Count)
                    return '\n';
                return tb[Start.iLine][Start.iChar].c;
            }
        }

        /// <summary>
        /// Returns first char before Start place
        /// </summary>
        public char CharBeforeStart
        {
            get
            {
                if (Start.iChar > tb[Start.iLine].Count)
                    return '\n';
                if (Start.iChar <= 0)
                    return '\n';
                return tb[Start.iLine][Start.iChar - 1].c;
            }
        }

        /// <summary>
        /// Returns required char's number before start of the Range
        /// </summary>
        public string GetCharsBeforeStart(int charsCount)
        {
            var pos = tb.PlaceToPosition(Start) - charsCount;
            if(pos < 0) pos = 0;

            return new Range(tb, tb.PositionToPlace(pos), Start).Text;
        }

        /// <summary>
        /// Returns required char's number after start of the Range
        /// </summary>
        public string GetCharsAfterStart(int charsCount)
        {
            return GetCharsBeforeStart(-charsCount);
        }

        /// <summary>
        /// Clone range
        /// </summary>
        /// <returns></returns>
        public Range Clone()
        {
            return (Range)MemberwiseClone();
        }

        /// <summary>
        /// Return minimum of end.X and start.X
        /// </summary>
        internal int FromX
        {
            get
            {
                if (end.iLine < start.iLine) return end.iChar;
                if (end.iLine > start.iLine) return start.iChar;
                return Math.Min(end.iChar, start.iChar);
            }
        }

        /// <summary>
        /// Return maximum of end.X and start.X
        /// </summary>
        internal int ToX
        {
            get
            {
                if (end.iLine < start.iLine) return start.iChar;
                if (end.iLine > start.iLine) return end.iChar;
                return Math.Max(end.iChar, start.iChar);
            }
        }

        public int FromLine
        {
            get { return Math.Min(Start.iLine, End.iLine); }
        }

        public int ToLine
        {
            get { return Math.Max(Start.iLine, End.iLine); }
        }

        /// <summary>
        /// Move range right
        /// </summary>
        /// <remarks>This method jump over folded blocks</remarks>
        public bool GoRight()
        {
            Place prevStart = start;
            GoRight(false);
            return prevStart != start;
        }

        /// <summary>
        /// Move range left
        /// </summary>
        /// <remarks>This method can to go inside folded blocks</remarks>
        public virtual bool GoRightThroughFolded()
        {
            if (ColumnSelectionMode)
                return GoRightThroughFolded_ColumnSelectionMode();

            if (start.iLine >= tb.LinesCount - 1 && start.iChar >= tb[tb.LinesCount - 1].Count)
                return false;

            if (start.iChar < tb[start.iLine].Count)
                start.Offset(1, 0);
            else
                start = new Place(0, start.iLine + 1);

            preferedPos = -1;
            end = start;
            OnSelectionChanged();
            return true;
        }

        /// <summary>
        /// Move range left
        /// </summary>
        /// <remarks>This method jump over folded blocks</remarks>
        public bool GoLeft()
        {
            ColumnSelectionMode = false;

            Place prevStart = start;
            GoLeft(false);
            return prevStart != start;
        }

        /// <summary>
        /// Move range left
        /// </summary>
        /// <remarks>This method can to go inside folded blocks</remarks>
        public bool GoLeftThroughFolded()
        {
            ColumnSelectionMode = false;

            if (start.iChar == 0 && start.iLine == 0)
                return false;

            if (start.iChar > 0)
                start.Offset(-1, 0);
            else
                start = new Place(tb[start.iLine - 1].Count, start.iLine - 1);

            preferedPos = -1;
            end = start;
            OnSelectionChanged();
            return true;
        }

        public void GoLeft(bool shift)
        {
            ColumnSelectionMode = false;

            if (!shift)
            if (start > end)
            {
                Start = End;
                return;
            }

            if (start.iChar != 0 || start.iLine != 0)
            {
                if (start.iChar > 0 && tb.LineInfos[start.iLine].VisibleState == VisibleState.Visible)
                    start.Offset(-1, 0);
                else
                {
                    int i = tb.FindPrevVisibleLine(start.iLine);
                    if (i == start.iLine) return;
                    start = new Place(tb[i].Count, i);
                }
            }

            if (!shift)
                end = start;

            OnSelectionChanged();

            preferedPos = -1;
        }

        public void GoRight(bool shift)
        {
            ColumnSelectionMode = false;

            if (!shift)
            if (start < end)
            {
                Start = End;
                return;
            }

            if (start.iLine < tb.LinesCount - 1 || start.iChar < tb[tb.LinesCount - 1].Count)
            {
                if (start.iChar < tb[start.iLine].Count && tb.LineInfos[start.iLine].VisibleState == VisibleState.Visible)
                    start.Offset(1, 0);
                else
                {
                    int i = tb.FindNextVisibleLine(start.iLine);
                    if (i == start.iLine) return;
                    start = new Place(0, i);
                }
            }

            if (!shift)
                end = start;

            OnSelectionChanged();

            preferedPos = -1;
        }

        internal void GoUp(bool shift)
        {
            ColumnSelectionMode = false;

            if (!shift)
            if (start.iLine > end.iLine)
            {
                Start = End;
                return;
            }

            if (preferedPos < 0)
                preferedPos = start.iChar - tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar));

            int iWW = tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar);
            if (iWW == 0)
            {
                if (start.iLine <= 0) return;
                int i = tb.FindPrevVisibleLine(start.iLine);
                if (i == start.iLine) return;
                start.iLine = i;
                iWW = tb.LineInfos[start.iLine].WordWrapStringsCount;
            }

            if (iWW > 0)
            {
                int finish = tb.LineInfos[start.iLine].GetWordWrapStringFinishPosition(iWW - 1, tb[start.iLine]);
                start.iChar = tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(iWW - 1) + preferedPos;
                if (start.iChar > finish + 1)
                    start.iChar = finish + 1;
            }

            if (!shift)
                end = start;

            OnSelectionChanged();
        }

        internal void GoPageUp(bool shift)
        {
            ColumnSelectionMode = false;

            if (preferedPos < 0)
                preferedPos = start.iChar - tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar));

            int pageHeight = tb.ClientRectangle.Height / tb.CharHeight - 1;

            for (int i = 0; i < pageHeight; i++)
            {
                int iWW = tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar);
                if (iWW == 0)
                {
                    if (start.iLine <= 0) break;
                    //pass hidden
                    int newLine = tb.FindPrevVisibleLine(start.iLine);
                    if (newLine == start.iLine) break;
                    start.iLine = newLine;
                    iWW = tb.LineInfos[start.iLine].WordWrapStringsCount;
                }

                if (iWW > 0)
                {
                    int finish = tb.LineInfos[start.iLine].GetWordWrapStringFinishPosition(iWW - 1, tb[start.iLine]);
                    start.iChar = tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(iWW - 1) + preferedPos;
                    if (start.iChar > finish + 1)
                        start.iChar = finish + 1;
                }
            }

            if (!shift)
                end = start;

            OnSelectionChanged();
        }

        internal void GoDown(bool shift)
        {
            ColumnSelectionMode = false;

            if(!shift)
            if (start.iLine < end.iLine)
            {
                Start = End;
                return;
            }

            if (preferedPos < 0)
                preferedPos = start.iChar - tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar));

            int iWW = tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar);
            if (iWW >= tb.LineInfos[start.iLine].WordWrapStringsCount - 1)
            {
                if (start.iLine >= tb.LinesCount - 1) return;
                //pass hidden
                int i = tb.FindNextVisibleLine(start.iLine);
                if (i == start.iLine) return;
                start.iLine = i;
                iWW = -1;
            }

            if (iWW < tb.LineInfos[start.iLine].WordWrapStringsCount - 1)
            {
                int finish = tb.LineInfos[start.iLine].GetWordWrapStringFinishPosition(iWW + 1, tb[start.iLine]);
                start.iChar = tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(iWW + 1) + preferedPos;
                if (start.iChar > finish + 1)
                    start.iChar = finish + 1;
            }

            if (!shift)
                end = start;

            OnSelectionChanged();
        }

        internal void GoPageDown(bool shift)
        {
            ColumnSelectionMode = false;

            if (preferedPos < 0)
                preferedPos = start.iChar - tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar));

            int pageHeight = tb.ClientRectangle.Height / tb.CharHeight - 1;

            for (int i = 0; i < pageHeight; i++)
            {
                int iWW = tb.LineInfos[start.iLine].GetWordWrapStringIndex(start.iChar);
                if (iWW >= tb.LineInfos[start.iLine].WordWrapStringsCount - 1)
                {
                    if (start.iLine >= tb.LinesCount - 1) break;
                    //pass hidden
                    int newLine = tb.FindNextVisibleLine(start.iLine);
                    if (newLine == start.iLine) break;
                    start.iLine = newLine;
                    iWW = -1;
                }

                if (iWW < tb.LineInfos[start.iLine].WordWrapStringsCount - 1)
                {
                    int finish = tb.LineInfos[start.iLine].GetWordWrapStringFinishPosition(iWW + 1, tb[start.iLine]);
                    start.iChar = tb.LineInfos[start.iLine].GetWordWrapStringStartPosition(iWW + 1) + preferedPos;
                    if (start.iChar > finish + 1)
                        start.iChar = finish + 1;
                }
            }

            if (!shift)
                end = start;

            OnSelectionChanged();
        }

        internal void GoHome(bool shift)
        {
            ColumnSelectionMode = false;

            if (start.iLine < 0)
                return;

            if (tb.LineInfos[start.iLine].VisibleState != VisibleState.Visible)
                return;

            start = new Place(0, start.iLine);

            if (!shift)
                end = start;

            OnSelectionChanged();

            preferedPos = -1;
        }

        internal void GoEnd(bool shift)
        {
            ColumnSelectionMode = false;

            if (start.iLine < 0)
                return;
            if (tb.LineInfos[start.iLine].VisibleState != VisibleState.Visible)
                return;

            start = new Place(tb[start.iLine].Count, start.iLine);

            if (!shift)
                end = start;

            OnSelectionChanged();

            preferedPos = -1;
        }

        /// <summary>
        /// Set style for range
        /// </summary>
        public void SetStyle(Style style)
        {
            //search code for style
            int code = tb.GetOrSetStyleLayerIndex(style);
            //set code to chars
            SetStyle(ToStyleIndex(code));
            //
            tb.Invalidate();
        }

        /// <summary>
        /// Set style for given regex pattern
        /// </summary>
        public void SetStyle(Style style, string regexPattern)
        {
            //search code for style
            StyleIndex layer = ToStyleIndex(tb.GetOrSetStyleLayerIndex(style));
            SetStyle(layer, regexPattern, RegexOptions.None);
        }

        /// <summary>
        /// Set style for given regex
        /// </summary>
        public void SetStyle(Style style, Regex regex)
        {
            //search code for style
            StyleIndex layer = ToStyleIndex(tb.GetOrSetStyleLayerIndex(style));
            SetStyle(layer, regex);
        }


        /// <summary>
        /// Set style for given regex pattern
        /// </summary>
        public void SetStyle(Style style, string regexPattern, RegexOptions options)
        {
            //search code for style
            StyleIndex layer = ToStyleIndex(tb.GetOrSetStyleLayerIndex(style));
            SetStyle(layer, regexPattern, options);
        }

        /// <summary>
        /// Set style for given regex pattern
        /// </summary>
        public void SetStyle(StyleIndex styleLayer, string regexPattern, RegexOptions options)
        {
            if (Math.Abs(Start.iLine - End.iLine) > 1000)
                options |= SyntaxHighlighter.RegexCompiledOption;
            //
            foreach (var range in GetRanges(regexPattern, options))
                range.SetStyle(styleLayer);
            //
            tb.Invalidate();
        }

        /// <summary>
        /// Set style for given regex pattern
        /// </summary>
        public void SetStyle(StyleIndex styleLayer, Regex regex)
        {
            foreach (var range in GetRanges(regex))
                range.SetStyle(styleLayer);
            //
            tb.Invalidate();
        }

        /// <summary>
        /// Appends style to chars of range
        /// </summary>
        public void SetStyle(StyleIndex styleIndex)
        {
            //set code to chars
            int fromLine = Math.Min(End.iLine, Start.iLine);
            int toLine = Math.Max(End.iLine, Start.iLine);
            int fromChar = FromX;
            int toChar = ToX;
            if (fromLine < 0) return;
            //
            for (int y = fromLine; y <= toLine; y++)
            {
                int fromX = y == fromLine ? fromChar : 0;
                int toX = y == toLine ? Math.Min(toChar - 1, tb[y].Count - 1) : tb[y].Count - 1;
                for (int x = fromX; x <= toX; x++)
                {
                    Char c = tb[y][x];
                    c.style |= styleIndex;
                    tb[y][x] = c;
                }
            }
        }

        /// <summary>
        /// Sets folding markers
        /// </summary>
        /// <param name="startFoldingPattern">Pattern for start folding line</param>
        /// <param name="finishFoldingPattern">Pattern for finish folding line</param>
        public void SetFoldingMarkers(string startFoldingPattern, string finishFoldingPattern)
        {
            SetFoldingMarkers(startFoldingPattern, finishFoldingPattern, SyntaxHighlighter.RegexCompiledOption);
        }

        /// <summary>
        /// Sets folding markers
        /// </summary>
        /// <param name="startFoldingPattern">Pattern for start folding line</param>
        /// <param name="finishFoldingPattern">Pattern for finish folding line</param>
        public void SetFoldingMarkers(string startFoldingPattern, string finishFoldingPattern, RegexOptions options)
        {
            if (startFoldingPattern == finishFoldingPattern)
            {
                SetFoldingMarkers(startFoldingPattern, options);
                return;
            }

            foreach (var range in GetRanges(startFoldingPattern, options))
                tb[range.Start.iLine].FoldingStartMarker = startFoldingPattern;

            foreach (var range in GetRanges(finishFoldingPattern, options))
                tb[range.Start.iLine].FoldingEndMarker = startFoldingPattern;
            //
            tb.Invalidate();
        }

        /// <summary>
        /// Sets folding markers
        /// </summary>
        /// <param name="startEndFoldingPattern">Pattern for start and end folding line</param>
        public void SetFoldingMarkers(string foldingPattern, RegexOptions options)
        {
            foreach (var range in GetRanges(foldingPattern, options))
            {
                if (range.Start.iLine > 0)
                    tb[range.Start.iLine-1].FoldingEndMarker = foldingPattern;
                tb[range.Start.iLine].FoldingStartMarker = foldingPattern;
            }

            tb.Invalidate();
        }
        /// <summary>
        /// Finds ranges for given regex pattern
        /// </summary>
        /// <param name="regexPattern">Regex pattern</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRanges(string regexPattern)
        {
            return GetRanges(regexPattern, RegexOptions.None);
        }

        /// <summary>
        /// Finds ranges for given regex pattern
        /// </summary>
        /// <param name="regexPattern">Regex pattern</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRanges(string regexPattern, RegexOptions options)
        {
            //get text
            string text;
            List<Place> charIndexToPlace;
            GetText(out text, out charIndexToPlace);
            //create regex
            Regex regex = new Regex(regexPattern, options);
            //
            foreach (Match m in regex.Matches(text))
            {
                Range r = new Range(this.tb);
                //try get 'range' group, otherwise use group 0
                Group group = m.Groups["range"];
                if (!group.Success)
                    group = m.Groups[0];
                //
                r.Start = charIndexToPlace[group.Index];
                r.End = charIndexToPlace[group.Index + group.Length];
                yield return r;
            }
        }

        /// <summary>
        /// Finds ranges for given regex pattern.
        /// Search is separately in each line.
        /// This method requires less memory than GetRanges().
        /// </summary>
        /// <param name="regexPattern">Regex pattern</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRangesByLines(string regexPattern, RegexOptions options)
        {
            var regex = new Regex(regexPattern, options);
            foreach (var r in GetRangesByLines(regex)) 
                yield return r;
        }

        /// <summary>
        /// Finds ranges for given regex.
        /// Search is separately in each line.
        /// This method requires less memory than GetRanges().
        /// </summary>
        /// <param name="regex">Regex</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRangesByLines(Regex regex)
        {
            Normalize();

            var fts = tb.TextSource as FileTextSource; //<----!!!! ugly

            //enumaerate lines
            for (int iLine = Start.iLine; iLine <= End.iLine; iLine++)
            {
                //
                bool isLineLoaded = fts != null ? fts.IsLineLoaded(iLine) : true;
                //
                var r = new Range(tb, new Place(0, iLine), new Place(tb[iLine].Count, iLine));
                if (iLine == Start.iLine || iLine == End.iLine)
                    r = r.GetIntersectionWith(this);

                foreach (var foundRange in r.GetRanges(regex))
                    yield return foundRange;

                if (!isLineLoaded)
                    fts.UnloadLine(iLine);
            }
        }

        /// <summary>
        /// Finds ranges for given regex pattern.
        /// Search is separately in each line (order of lines is reversed).
        /// This method requires less memory than GetRanges().
        /// </summary>
        /// <param name="regexPattern">Regex pattern</param>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRangesByLinesReversed(string regexPattern, RegexOptions options)
        {
            Normalize();
            //create regex
            Regex regex = new Regex(regexPattern, options);
            //
            var fts = tb.TextSource as FileTextSource; //<----!!!! ugly

            //enumaerate lines
            for (int iLine = End.iLine; iLine >= Start.iLine; iLine--)
            {
                //
                bool isLineLoaded = fts != null ? fts.IsLineLoaded(iLine) : true;
                //
                var r = new Range(tb, new Place(0, iLine), new Place(tb[iLine].Count, iLine));
                if (iLine == Start.iLine || iLine == End.iLine)
                    r = r.GetIntersectionWith(this);

                var list = new List<Range>();

                foreach (var foundRange in r.GetRanges(regex))
                    list.Add(foundRange);

                for (int i = list.Count - 1; i >= 0; i--)
                    yield return list[i];

                if (!isLineLoaded)
                    fts.UnloadLine(iLine);
            }
        }

        /// <summary>
        /// Finds ranges for given regex
        /// </summary>
        /// <returns>Enumeration of ranges</returns>
        public IEnumerable<Range> GetRanges(Regex regex)
        {
            //get text
            string text;
            List<Place> charIndexToPlace;
            GetText(out text, out charIndexToPlace);
            //
            foreach (Match m in regex.Matches(text))
            {
                Range r = new Range(this.tb);
                //try get 'range' group, otherwise use group 0
                Group group = m.Groups["range"];
                if (!group.Success)
                    group = m.Groups[0];
                //
                r.Start = charIndexToPlace[group.Index];
                r.End = charIndexToPlace[group.Index + group.Length];
                yield return r;
            }
        }

        /// <summary>
        /// Clear styles of range
        /// </summary>
        public void ClearStyle(params Style[] styles)
        {
            try
            {
               ClearStyle(tb.GetStyleIndexMask(styles));
            }
            catch { ;}
        }

        /// <summary>
        /// Clear styles of range
        /// </summary>
        public void ClearStyle(StyleIndex styleIndex)
        {
            //set code to chars
            int fromLine = Math.Min(End.iLine, Start.iLine);
            int toLine = Math.Max(End.iLine, Start.iLine);
            int fromChar = FromX;
            int toChar = ToX;
            if (fromLine < 0) return;
            //
            for (int y = fromLine; y <= toLine; y++)
            {
                int fromX = y == fromLine ? fromChar : 0;
                int toX = y == toLine ? Math.Min(toChar - 1, tb[y].Count - 1) : tb[y].Count - 1;
                for (int x = fromX; x <= toX; x++)
                {
                    Char c = tb[y][x];
                    c.style &= ~styleIndex;
                    tb[y][x] = c;
                }
            }
            //
            tb.Invalidate();
        }

        /// <summary>
        /// Clear folding markers of all lines of range
        /// </summary>
        public void ClearFoldingMarkers()
        {
            //set code to chars
            int fromLine = Math.Min(End.iLine, Start.iLine);
            int toLine = Math.Max(End.iLine, Start.iLine);
            if (fromLine < 0) return;
            //
            for (int y = fromLine; y <= toLine; y++)
                tb[y].ClearFoldingMarkers();
            //
            tb.Invalidate();
        }

        void OnSelectionChanged()
        {
            //clear cache
            cachedTextVersion = -1;
            cachedText = null;
            cachedCharIndexToPlace = null;
            //
            if (tb.Selection == this)
                if (updating == 0)
                    tb.OnSelectionChanged();
        }

        /// <summary>
        /// Starts selection position updating
        /// </summary>
        public void BeginUpdate()
        {
            updating++;
        }

        /// <summary>
        /// Ends selection position updating
        /// </summary>
        public void EndUpdate()
        {
            updating--;
            if (updating == 0)
                OnSelectionChanged();
        }

        public override string ToString()
        {
            return "Start: " + Start + " End: " + End;
        }

        /// <summary>
        /// Exchanges Start and End if End appears before Start
        /// </summary>
        public void Normalize()
        {
            if (Start > End)
                Inverse();
        }

        /// <summary>
        /// Exchanges Start and End
        /// </summary>
        public void Inverse()
        {
            var temp = start;
            start = end;
            end = temp;
        }

        /// <summary>
        /// Expands range from first char of Start line to last char of End line
        /// </summary>
        public void Expand()
        {
            Normalize();
            start = new Place(0, start.iLine);
            end = new Place(tb.GetLineLength(end.iLine), end.iLine);
        }

        IEnumerator<Place> IEnumerable<Place>.GetEnumerator()
        {
            if (ColumnSelectionMode)
            {
                foreach(var p in GetEnumerator_ColumnSelectionMode())
                    yield return p;
                yield break;
            }

            int fromLine = Math.Min(end.iLine, start.iLine);
            int toLine = Math.Max(end.iLine, start.iLine);
            int fromChar = FromX;
            int toChar = ToX;
            if (fromLine < 0) yield break;
            //
            for (int y = fromLine; y <= toLine; y++)
            {
                int fromX = y == fromLine ? fromChar : 0;
                int toX = y == toLine ? Math.Min(toChar - 1, tb[y].Count - 1) : tb[y].Count - 1;
                for (int x = fromX; x <= toX; x++)
                    yield return new Place(x, y);
            }
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return (this as IEnumerable<Place>).GetEnumerator();
        }

        /// <summary>
        /// Chars of range (exclude \n)
        /// </summary>
        public IEnumerable<Char> Chars
        {
            get
            {
                if (ColumnSelectionMode)
                {
                    foreach (var p in GetEnumerator_ColumnSelectionMode())
                        yield return tb[p];
                    yield break;
                }

                int fromLine = Math.Min(end.iLine, start.iLine);
                int toLine = Math.Max(end.iLine, start.iLine);
                int fromChar = FromX;
                int toChar = ToX;
                if (fromLine < 0) yield break;
                //
                for (int y = fromLine; y <= toLine; y++)
                {
                    int fromX = y == fromLine ? fromChar : 0;
                    int toX = y == toLine ? Math.Min(toChar - 1, tb[y].Count - 1) : tb[y].Count - 1;
                    var line = tb[y];
                    for (int x = fromX; x <= toX; x++)
                        yield return line[x];
                }
            }
        }

        /// <summary>
        /// Get fragment of text around Start place. Returns maximal matched to pattern fragment.
        /// </summary>
        /// <param name="allowedSymbolsPattern">Allowed chars pattern for fragment</param>
        /// <returns>Range of found fragment</returns>
        public Range GetFragment(string allowedSymbolsPattern)
        {
            return GetFragment(allowedSymbolsPattern, RegexOptions.None);
        }

        /// <summary>
        /// Get fragment of text around Start place. Returns maximal matched to given Style.
        /// </summary>
        /// <param name="style">Allowed style for fragment</param>
        /// <returns>Range of found fragment</returns>
        public Range GetFragment(Style style, bool allowLineBreaks)
        {
            var mask = tb.GetStyleIndexMask(new Style[] { style });
            //
            Range r = new Range(tb);
            r.Start = Start;
            //go left, check style
            while (r.GoLeftThroughFolded())
            {
                if (!allowLineBreaks && r.CharAfterStart == '\n')
                    break;
                if (r.Start.iChar < tb.GetLineLength(r.Start.iLine))
                if ((tb[r.Start].style & mask) == 0)
                {
                    r.GoRightThroughFolded();
                    break;
                }
            }
            Place startFragment = r.Start;

            r.Start = Start;
            //go right, check style
            do
            {
                if (!allowLineBreaks && r.CharAfterStart == '\n')
                    break;
                if (r.Start.iChar < tb.GetLineLength(r.Start.iLine))
                if ((tb[r.Start].style & mask) == 0)
                    break;
            } while (r.GoRightThroughFolded());
            Place endFragment = r.Start;

            return new Range(tb, startFragment, endFragment);
        }

        /// <summary>
        /// Get fragment of text around Start place. Returns maximal mathed to pattern fragment.
        /// </summary>
        /// <param name="allowedSymbolsPattern">Allowed chars pattern for fragment</param>
        /// <returns>Range of found fragment</returns>
        public Range GetFragment(string allowedSymbolsPattern, RegexOptions options)
        {
            Range r = new Range(tb);
            r.Start = Start;
            Regex regex = new Regex(allowedSymbolsPattern, options);
            //go left, check symbols
            while (r.GoLeftThroughFolded())
            {
                if (!regex.IsMatch(r.CharAfterStart.ToString()))
                {
                    r.GoRightThroughFolded();
                    break;
                }
            }
            Place startFragment = r.Start;

            r.Start = Start;
            //go right, check symbols
            do
            {
                if (!regex.IsMatch(r.CharAfterStart.ToString()))
                    break;
            } while (r.GoRightThroughFolded()) ;
            Place endFragment = r.Start;

            return new Range(tb, startFragment, endFragment);
        }

        bool IsIdentifierChar(char c)
        {
            return char.IsLetterOrDigit(c) || c == '_';
        }

        bool IsSpaceChar(char c)
        {
            return c == ' ' || c == '\t';
        }

        public void GoWordLeft(bool shift)
        {
            ColumnSelectionMode = false;

            if (!shift && start > end)
            {
                Start = End;
                return;
            }

            Range range = this.Clone();//to OnSelectionChanged disable
            bool wasSpace = false;
            while (IsSpaceChar(range.CharBeforeStart))
            {
                wasSpace = true;
                range.GoLeft(shift);
            }
            bool wasIdentifier = false;
            while (IsIdentifierChar(range.CharBeforeStart))
            {
                wasIdentifier = true;
                range.GoLeft(shift);
            }
            if (!wasIdentifier && (!wasSpace || range.CharBeforeStart != '\n'))
                range.GoLeft(shift);
            this.Start = range.Start;
            this.End = range.End;

            if (tb.LineInfos[Start.iLine].VisibleState != VisibleState.Visible)
                GoRight(shift);
        }

        public void GoWordRight(bool shift, bool goToStartOfNextWord = false)
        {
            ColumnSelectionMode = false;

            if (!shift && start < end)
            {
                Start = End;
                return;
            }

            Range range = this.Clone();//to OnSelectionChanged disable

            bool wasNewLine = false;


            if (range.CharAfterStart == '\n')
            {
                range.GoRight(shift);
                wasNewLine = true;
            }

            bool wasSpace = false;
            while (IsSpaceChar(range.CharAfterStart))
            {
                wasSpace = true;
                range.GoRight(shift);
            }

            if (!((wasSpace || wasNewLine) && goToStartOfNextWord))
            {

                bool wasIdentifier = false;
                while (IsIdentifierChar(range.CharAfterStart))
                {
                    wasIdentifier = true;
                    range.GoRight(shift);
                }

                if (!wasIdentifier)
                    range.GoRight(shift);

                if (goToStartOfNextWord && !wasSpace)
                    while (IsSpaceChar(range.CharAfterStart))
                        range.GoRight(shift);
            }

            this.Start = range.Start;
            this.End = range.End;

            if (tb.LineInfos[Start.iLine].VisibleState != VisibleState.Visible)
                GoLeft(shift);
        }

        internal void GoFirst(bool shift)
        {
            ColumnSelectionMode = false;

            start = new Place(0, 0);
            if (tb.LineInfos[Start.iLine].VisibleState != VisibleState.Visible)
                tb.ExpandBlock(Start.iLine);

            if(!shift)
                end = start;

            OnSelectionChanged();
        }

        internal void GoLast(bool shift)
        {
            ColumnSelectionMode = false;

            start = new Place(tb[tb.LinesCount - 1].Count, tb.LinesCount-1);
            if (tb.LineInfos[Start.iLine].VisibleState != VisibleState.Visible)
                tb.ExpandBlock(Start.iLine);

            if (!shift)
                end = start;

            OnSelectionChanged();
        }

        public static StyleIndex ToStyleIndex(int i)
        {
            return (StyleIndex)(1 << i);
        }

        public RangeRect Bounds
        {
            get
            {
                int minX = Math.Min(Start.iChar, End.iChar);
                int minY = Math.Min(Start.iLine, End.iLine);
                int maxX = Math.Max(Start.iChar, End.iChar);
                int maxY = Math.Max(Start.iLine, End.iLine);
                return new RangeRect(minY, minX, maxY, maxX);
            }
        }

        public IEnumerable<Range> GetSubRanges(bool includeEmpty)
        {
            if (!ColumnSelectionMode)
            {
                yield return this;
                yield break;
            }

            var rect = Bounds;
            for (int y = rect.iStartLine; y <= rect.iEndLine; y++)
            {
                if (rect.iStartChar > tb[y].Count && !includeEmpty)
                    continue;

                var r = new Range(tb, rect.iStartChar, y, Math.Min(rect.iEndChar, tb[y].Count), y);
                yield return r;
            }
        }

        /// <summary>
        /// Range is readonly?
        /// This property return True if any char of the range contains ReadOnlyStyle.
        /// Set this property to True/False to mark chars of the range as Readonly/Writable.
        /// </summary>
        public bool ReadOnly 
        {
            get
            {
                if (tb.ReadOnly) return true;

                ReadOnlyStyle readonlyStyle = null;
                foreach (var style in tb.Styles)
                    if (style is ReadOnlyStyle)
                    {
                        readonlyStyle = (ReadOnlyStyle)style;
                        break;
                    }

                if (readonlyStyle != null)
                {
                    var si = ToStyleIndex(tb.GetStyleIndex(readonlyStyle));

                    if (IsEmpty)
                    {
                        //check previous and next chars
                        var line = tb[start.iLine];
                        if (columnSelectionMode)
                        {
                            foreach (var sr in GetSubRanges(false))
                            {
                                line = tb[sr.start.iLine];
                                if (sr.start.iChar < line.Count && sr.start.iChar > 0)
                                {
                                    var left = line[sr.start.iChar - 1];
                                    var right = line[sr.start.iChar];
                                    if ((left.style & si) != 0 &&
                                        (right.style & si) != 0) return true;//we are between readonly chars
                                }
                            }
                        }else
                        if (start.iChar < line.Count && start.iChar > 0)
                        {
                            var left = line[start.iChar - 1];
                            var right = line[start.iChar];
                            if ((left.style & si) != 0 &&
                                (right.style & si) != 0) return true;//we are between readonly chars
                        }
                    }
                    else
                    foreach (Char c in Chars)
                        if ((c.style & si) != 0)//found char with ReadonlyStyle
                            return true;
                }

                return false;
            }

            set 
            {
                //find exists ReadOnlyStyle of style buffer
                ReadOnlyStyle readonlyStyle = null;
                foreach (var style in tb.Styles)
                    if (style is ReadOnlyStyle)
                    {
                        readonlyStyle = (ReadOnlyStyle)style;
                        break;
                    }

                //create ReadOnlyStyle
                if(readonlyStyle == null)
                    readonlyStyle = new ReadOnlyStyle();

                //set/clear style
                if (value)
                    SetStyle(readonlyStyle);
                else
                    ClearStyle(readonlyStyle);
            }
        }

        /// <summary>
        /// Is char before range readonly
        /// </summary>
        /// <returns></returns>
        public bool IsReadOnlyLeftChar()
        {
            if (tb.ReadOnly) return true;

            var r = Clone();

            r.Normalize();
            if (r.start.iChar == 0) return false;
            if (ColumnSelectionMode)
                r.GoLeft_ColumnSelectionMode();
            else
                r.GoLeft(true);

            return r.ReadOnly;
        }

        /// <summary>
        /// Is char after range readonly
        /// </summary>
        /// <returns></returns>
        public bool IsReadOnlyRightChar()
        {
            if (tb.ReadOnly) return true;

            var r = Clone();

            r.Normalize();
            if (r.end.iChar >= tb[end.iLine].Count) return false;
            if (ColumnSelectionMode)
                r.GoRight_ColumnSelectionMode();
            else
                r.GoRight(true);

            return r.ReadOnly;
        }

        public IEnumerable<Place> GetPlacesCyclic(Place startPlace, bool backward = false)
        {
            if (backward)
            {
                var r = new Range(this.tb, startPlace, startPlace);
                while (r.GoLeft() && r.start >= Start)
                {
                    if (r.Start.iChar < tb[r.Start.iLine].Count)
                        yield return r.Start;
                }

                r = new Range(this.tb, End, End);
                while (r.GoLeft() && r.start >= startPlace)
                {
                    if (r.Start.iChar < tb[r.Start.iLine].Count)
                        yield return r.Start;
                }
            }
            else
            {
                var r = new Range(this.tb, startPlace, startPlace);
                if (startPlace < End)
                    do
                    {
                        if (r.Start.iChar < tb[r.Start.iLine].Count)
                            yield return r.Start;
                    } while (r.GoRight());

                r = new Range(this.tb, Start, Start);
                if (r.Start < startPlace)
                    do
                    {
                        if (r.Start.iChar < tb[r.Start.iLine].Count)
                            yield return r.Start;
                    } while (r.GoRight() && r.Start < startPlace);
            }
        }

        #region ColumnSelectionMode

        private Range GetIntersectionWith_ColumnSelectionMode(Range range)
        {
            if (range.Start.iLine != range.End.iLine)
                return new Range(tb, Start, Start);
            var rect = Bounds;
            if (range.Start.iLine < rect.iStartLine || range.Start.iLine > rect.iEndLine)
                return new Range(tb, Start, Start);

            return new Range(tb, rect.iStartChar, range.Start.iLine, rect.iEndChar, range.Start.iLine).GetIntersectionWith(range);
        }

        private bool GoRightThroughFolded_ColumnSelectionMode()
        {
            var boundes = Bounds;
            var endOfLines = true;
            for (int iLine = boundes.iStartLine; iLine <= boundes.iEndLine; iLine++)
                if(boundes.iEndChar < tb[iLine].Count)
                {
                    endOfLines = false;
                    break;
                }

            if (endOfLines)
                return false;

            var start = Start;
            var end = End;
            start.Offset(1, 0);
            end.Offset(1, 0);
            BeginUpdate();
            Start = start;
            End = end;
            EndUpdate();

            return true;
        }

        private IEnumerable<Place> GetEnumerator_ColumnSelectionMode()
        {
            var bounds = Bounds;
            if (bounds.iStartLine < 0) yield break;
            //
            for (int y = bounds.iStartLine; y <= bounds.iEndLine; y++)
            {
                for (int x = bounds.iStartChar; x < bounds.iEndChar; x++)
                {
                    if (x < tb[y].Count)
                        yield return new Place(x, y);
                }
            }
        }

        private string Text_ColumnSelectionMode
        {
            get
            {
                StringBuilder sb = new StringBuilder();
                var bounds = Bounds;
                if (bounds.iStartLine < 0) return "";
                //
                for (int y = bounds.iStartLine; y <= bounds.iEndLine; y++)
                {
                    for (int x = bounds.iStartChar; x < bounds.iEndChar; x++)
                    {
                        if (x < tb[y].Count)
                            sb.Append(tb[y][x].c);
                    }
                    if (bounds.iEndLine != bounds.iStartLine && y != bounds.iEndLine)
                        sb.AppendLine();
                }

                return sb.ToString();
            }
        }

        internal void GoDown_ColumnSelectionMode()
        {
            var iLine = tb.FindNextVisibleLine(End.iLine);
            End = new Place(End.iChar, iLine);
        }

        internal void GoUp_ColumnSelectionMode()
        {
            var iLine = tb.FindPrevVisibleLine(End.iLine);
            End = new Place(End.iChar, iLine);
        }

        internal void GoRight_ColumnSelectionMode()
        {
            End = new Place(End.iChar + 1, End.iLine);
        }

        internal void GoLeft_ColumnSelectionMode()
        {
            if (End.iChar > 0)
                End = new Place(End.iChar - 1, End.iLine);
        }

        #endregion
    }

    public struct RangeRect
    {
        public RangeRect(int iStartLine, int iStartChar, int iEndLine, int iEndChar)
        {
            this.iStartLine = iStartLine;
            this.iStartChar = iStartChar;
            this.iEndLine = iEndLine;
            this.iEndChar = iEndChar;
        }

        public int iStartLine;
        public int iStartChar;
        public int iEndLine;
        public int iEndChar;
    }
}
