using System;

namespace FastColoredTextBoxNS
{
    /// <summary>
    /// Limited stack
    /// </summary>
    public class LimitedStack<T>
    {
        T[] items;
        int count;
        int start;

        /// <summary>
        /// Max stack length
        /// </summary>
        public int MaxItemCount
        {
            get { return items.Length; }
        }

        /// <summary>
        /// Current length of stack
        /// </summary>
        public int Count
        {
            get { return count; }
        }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="maxItemCount">Maximum length of stack</param>
        public LimitedStack(int maxItemCount)
        {
            items = new T[maxItemCount];
            count = 0;
            start = 0;
        }

        /// <summary>
        /// Pop item
        /// </summary>
        public T Pop()
        {
            if (count == 0)
                throw new Exception("Stack is empty");

            int i = LastIndex;
            T item = items[i];
            items[i] = default(T);

            count--;

            return item;
        }

        int LastIndex
        {
            get { return (start + count - 1) % items.Length; }
        }

        /// <summary>
        /// Peek item
        /// </summary>
        public T Peek()
        {
            if (count == 0)
                return default(T);

            return items[LastIndex];
        }

        /// <summary>
        /// Push item
        /// </summary>
        public void Push(T item)
        {
            if (count == items.Length)
                start = (start + 1) % items.Length;
            else
                count++;

            items[LastIndex] = item;
        }

        /// <summary>
        /// Clear stack
        /// </summary>
        public void Clear()
        {
            items = new T[items.Length];
            count = 0;
            start = 0;
        }

        public T[] ToArray()
        {
            T[] result = new T[count];
            for (int i = 0; i < count; i++)
                result[i] = items[(start + i) % items.Length];
            return result;
        }
    }
}