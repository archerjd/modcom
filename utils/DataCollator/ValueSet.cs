using System;
using System.Collections.Generic;
using System.Text;
using System.Data;

namespace DataCollator
{
    class ValueSet
    {
        public ValueSet(string sqlQuery)
        {
            // initialise, run query
            DataTable dt = SqlInterface.RunQuery(sqlQuery, SqlInterface.db);
            if (dt == null)
                return;
            Type t = dt.Columns[0].DataType;
            foreach (DataRow row in dt.Rows)
            {
                float f;
                if (t == typeof(Int64))
                {
                    Int64 i = (Int64)row[0];
                    f = (float)i;
                }
                else
                {
                    double d = (double)row[0];
                    f = (float)d;
                }
                if (!values.ContainsKey(f)) // not sure that this is really that smart tbh
                    values.Add(f, 0);
            }
        }

        // the int is 0 in all cases, we only care about the keys
        private SortedList<float, int> values = new SortedList<float, int>();

        public int Count { get { return values.Count; } }
        private bool HasData { get { return values.Count > 0; } }

        // mean
        public float GetMin()
        {
            if ( !HasData )
                return 0;
            return values.Keys[0];
        }

        public float GetMax()
        {
            if (!HasData)
                return 0;
            return values.Keys[values.Count - 1];
        }

        public float GetMedian()
        {
            if (!HasData)
                return 0;

            // examples:
            // if 5 items, median is item 2 (max 4)
            // if 6 items, median is sum of items 2+3 (max 6)
            // 2 is (6-2)/2, 3 is 6/2
            if (values.Count % 2 == 0) // even number of items ... 
                return (values.Keys[values.Count / 2] + values.Keys[(values.Count - 2) / 2]) / 2;
            else // odd number of items ... 
                return values.Keys[(values.Count - 1) / 2];
        }

        public float GetLowerQuartile()
        {
            if (!HasData)
                return 0;

            int remainder = values.Count % 4;
            // 4n-3 pieces of data, the LQ is the n th value,
            // 4n-2 pieces of data, the LQ is the n th value,
            // 4n-1 pieces of data, the LQ is the n th value,
            // but for 4n pieces of data, the LQ is not uniquely defined, by this, it could be the n th value, or the n+1 th value.
            // It would be sensible to take the average of these two values in this case.

            switch (remainder)
            {
                case 3:
                case 1:
                case 2:
                    return values.Keys[(values.Count - remainder - 1) / 4];
                default: // 0
                    return (values.Keys[values.Count / 4 - 1] + values.Keys[values.Count / 4]) / 2;
            }
        }

        public float GetUpperQuartile()
        {
            if (!HasData)
                return 0;

            int remainder = values.Count % 4;
            switch (remainder)
            {
                case 3:
                case 1:
                case 2:
                    return values.Keys[values.Count - 1 - ((values.Count - remainder - 1) / 4)];
                default: // 0
                    return (values.Keys[values.Count - 1 - (values.Count / 4 - 1)] + values.Keys[values.Count - 1 - (values.Count / 4)]) / 2;
            }
        }

        public float GetInterQuartileRange()
        {
            return GetUpperQuartile() - GetLowerQuartile();
        }

        public float GetMean()
        {
            if (!HasData)
                return 0;
            float sum = 0;
            foreach (float f in values.Keys)
                sum += f;

            return sum / values.Count;
        }

        public float GetStandardDeviation()
        {
            if (!HasData)
                return 0;

            float mean = GetMean();
            float sum = 0;
            foreach (float f in values.Keys)
                sum += (f - mean) * (f - mean);

            sum /= values.Count; // if this is an "incomplete sample", use values.Count-1 instead

            return (float)Math.Sqrt(sum);
        }
    }
}
