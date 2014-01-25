using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;

namespace MonsterDifficultyConfig
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            btnRefresh_Click(this, new EventArgs());
        }

        decimal[] values = new decimal[26];

        enum EditMode
        {
            ScaleAll = -1,
            AddAll = -2,
            SetRange = -3,
        };

        private void Form1_Load(object sender, EventArgs e)
        {

        }

        private void lstMonsterTypes_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lstMonsterTypes.SelectedIndex >= 0)
            {
                ParseFile(lstMonsterTypes.SelectedItem.ToString() + ".txt");
                btnSave.Enabled = true;
            }
            else
                btnSave.Enabled = false;
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            lstMonsterTypes.Items.Clear();
            lstFields.Items.Clear();

            DirectoryInfo di = new DirectoryInfo(".");
            FileInfo[] rgFiles = di.GetFiles("*.txt");
            foreach (FileInfo fi in rgFiles)
                lstMonsterTypes.Items.Add(fi.Name.Substring(0, fi.Name.Length - 4));
        }

        KeyValues keyvalues;

        public void ParseFile(string filename)
        {
            lstFields.Items.Clear();

            string line;
            int pos = 0, depth = 0;
            StreamReader file = new StreamReader(filename);

            keyvalues = new KeyValues("blah");
            KeyValues currentNode = keyvalues;
            while((line = file.ReadLine()) != null)
            {
                pos++;

                line = line.Trim();
                if (line == string.Empty)
                    continue;
                if (line == "{")
                    depth++;
                else if (line == "}")
                {
                    depth--;
                    if (currentNode.Parent != null)
                        currentNode = currentNode.Parent;
                }
                else if (pos == 1)
                {
                    while ( line.StartsWith("\"") )
                        line = line.Substring(1);
                    while ( line.EndsWith("\"") )
                        line = line.Substring(0, line.Length-1);
                    currentNode.Name = line;
                }
                else
                {
                    KeyValues currentLine = KeyValues.Create(line);
                    if (currentLine != null)
                    {
                        currentNode.AddChild(currentLine);
                        if (currentLine.Type == KeyValues.KVType.Parent)
                            currentNode = currentLine;
                    }
                }
            }

            file.Close();

            if (depth != 0)
            {
                MessageBox.Show(string.Format("Error: finished file read at depth {0}, should be 0!", depth));
                return;
            }

            if ( keyvalues.Type != KeyValues.KVType.Parent )
                return; // oh dear

            // now work out what fields this keyvalues file has in it for each level
            SortedList<string, object> distinctFields = new SortedList<string, object>();
            foreach ( KeyValues level in keyvalues.Children )
                if ( level.Type == KeyValues.KVType.Parent )
                    foreach ( KeyValues kv in level.Children )
                        if ( kv.Type == KeyValues.KVType.Numeric && !distinctFields.ContainsKey(kv.Name) )
                            distinctFields.Add(kv.Name,null);
            foreach (string s in distinctFields.Keys)
                lstFields.Items.Add(s);
        }

        public void Save()
        {
            if (keyvalues != null)
            {
                TextWriter tw = new StreamWriter(lstMonsterTypes.SelectedItem.ToString() + ".txt");
                tw.Write(keyvalues.ToString());
                tw.Close();
            }
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            Save();
        }

        private void lstFields_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lstFields.SelectedIndex >= 0)
            {
                string name = lstFields.SelectedItem.ToString();
                for (int i = 0; i < 26; i++)
                    values[i] = 0;

                foreach ( KeyValues level in keyvalues.Children )
                if ( level.Type == KeyValues.KVType.Parent )
                    foreach ( KeyValues kv in level.Children )
                        if ( kv.Name == name )
                        {
                            int lvl = 0;
                            int.TryParse(level.Name, out lvl);

                            if (kv.Type == KeyValues.KVType.Numeric)
                                values[lvl] = kv.NumericValue;
                            else
                                values[lvl] = 0;
                            break;
                        }

                graphPanel1.Values.Clear();
                for (int i = 0; i < 26; i++)
                    graphPanel1.Values.Add((int)values[i]);
                graphPanel1.Invalidate();
            }
        }

        private void btnScaleAll_Click(object sender, EventArgs e)
        {
            EditValue((int)EditMode.ScaleAll);
        }

        private void btnAddAll_Click(object sender, EventArgs e)
        {
            EditValue((int)EditMode.AddAll);
        }

        private void btnAddLevel_Click(object sender, EventArgs e)
        {
            EditValue((int)EditMode.SetRange);
        }

        public void EditValue(int level)
        {
            if (lstFields.SelectedItem == null)
                return;

            // show a popup to edit the current value
            if (level >= 0)
                EntryPopup.Show(string.Format("Enter new value for level {0}:", level), values[level].ToString(), "", lstFields.SelectedItem.ToString(), level, EditComplete, this);
            else
            {
                string message, val1, val2="";
                switch (level)
                {
                    case (int)EditMode.AddAll:
                        message = "Add to each value:";
                        val1 = "0";
                        break;
                    case (int)EditMode.ScaleAll:
                        message = "Scale each value by:";
                        val1 = "1";
                        break;
                    case (int)EditMode.SetRange:
                        message = "Set range of values:";
                        val1 = values[0].ToString();
                        val2 = values[25].ToString();
                        break;
                    default:
                        return;
                }

                EntryPopup.Show(message, val1, val2, lstFields.SelectedItem.ToString(), level, EditComplete, this);
            }
        }

        public void EditComplete(object sender, EventArgs e)
        {
            graphPanel1.ClearHighlight();

            decimal value1, value2 = 0;
            if (!decimal.TryParse(EntryPopup.lastEnteredValue1, out value1))
            {
                MessageBox.Show("Bad value");
                return;
            }

            if ( EntryPopup.lastEditedLevel < -2 && !decimal.TryParse(EntryPopup.lastEnteredValue2, out value2))
            {
                MessageBox.Show("Bad value #2");
                return;
            }

            // edit KeyValues regardless of whether this is still the active field
            string levelName = EntryPopup.lastEditedLevel.ToString();
            foreach ( KeyValues level in keyvalues.Children)
                if ( (EntryPopup.lastEditedLevel < 0 || level.Name == levelName) && level.Type == KeyValues.KVType.Parent )
                {
                    foreach ( KeyValues kv in level.Children )
                        if (kv.Name == EntryPopup.lastEditedField)
                        {
                            int levelNum;
                            int.TryParse(level.Name, out levelNum);
                            switch (EntryPopup.lastEditedLevel)
                            {
                                case (int)EditMode.AddAll:
                                    kv.NumericValue += value1;
                                    break;
                                case (int)EditMode.SetRange:
                                    kv.NumericValue = Scale(levelNum,value1,value2);
                                    break;
                                case (int)EditMode.ScaleAll:
                                    kv.NumericValue *= value1;
                                    break;
                                default:
                                    kv.NumericValue = value1;
                                    break;
                            }

                            break;
                        }

                    if ( EntryPopup.lastEditedLevel >= 0 )
                        break;
                }

            if ((string)lstFields.Items[lstFields.SelectedIndex] == EntryPopup.lastEditedField)
            {
                // also update values[] and buttons[]
                if ( EntryPopup.lastEditedLevel < 0 )
                    graphPanel1.Values.Clear();
                switch (EntryPopup.lastEditedLevel)
                {
                    case (int)EditMode.AddAll:
                        for (int i = 0; i < 26; i++)
                        {
                            values[i] += value1;
                            graphPanel1.Values.Add((int)values[i]);
                        }
                        break;
                    case (int)EditMode.SetRange:
                        for (int i = 0; i < 26; i++)
                        {
                            values[i] = Scale(i, value1, value2);
                            graphPanel1.Values.Add((int)values[i]);
                        }
                        break;
                    case (int)EditMode.ScaleAll:
                        for (int i = 0; i < 26; i++)
                        {
                            values[i] *= value1;
                            graphPanel1.Values.Add((int)values[i]);
                        }
                        break;
                    default:
                        values[EntryPopup.lastEditedLevel] = value1;
                        graphPanel1.Values[EntryPopup.lastEditedLevel] = (int)value1;
                        break;
                }
                graphPanel1.Invalidate();
            }
        }

        public decimal Scale(int level, decimal v1, decimal v2)
        {
            decimal dif = v2 - v1, difPerLevel = dif / 25;
            
            return v1 + difPerLevel * level;
        }

        public decimal GetValue(int level)
        {
            if (keyvalues == null || lstFields.SelectedIndex < 0)
                return 0;

            string name = level.ToString();
            foreach (KeyValues lvl in keyvalues.Children)
                if (lvl.Name == name)
                {
                    foreach (KeyValues field in lvl.Children)
                        if ((string)field.Name == (string)lstFields.Items[lstFields.SelectedIndex])
                        {
                            if (field.Type == KeyValues.KVType.Numeric)
                                return field.NumericValue;
                            else
                                return 0;
                        }
                    break;
                }

            return 0;
        }

        public void SetValue(int level, int val)
        {
            if (keyvalues == null || lstFields.SelectedIndex < 0)
                return;

            string name = level.ToString();
            foreach (KeyValues lvl in keyvalues.Children)
                if (lvl.Name == name)
                {
                    foreach (KeyValues field in lvl.Children)
                        if (field.Name == (string)lstFields.Items[lstFields.SelectedIndex])
                        {
                            //if (field.Type == KeyValues.KVType.Numeric)
                                field.NumericValue = val;
                        }
                    break;
                }
        }

        private void graphPanel1_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button != MouseButtons.Left || graphPanel1.Values.Count == 0)
                return;

            // 0.5+1+0.25 ... 0.25+1+0.25 ...  0.25+1+0.25 ...  0.25+1+0.5


            int clickedColumn = -1;
            float testX = graphPanel1.BarWidth * 1.75f;
            if (e.X <= testX)
                clickedColumn = 0;
            else
            {
                for (int i = 1; i < graphPanel1.Values.Count - 1; i++)
                {
                    testX += graphPanel1.BarWidth * 1.5f;
                    if (e.X < testX)
                    {
                        clickedColumn = i;
                        break;
                    }
                }
                if (clickedColumn == -1)
                    clickedColumn = graphPanel1.Values.Count - 1;
            }
            graphPanel1.Highlight(clickedColumn);
            EditValue(clickedColumn);
        }
    }

    public class KeyValues
    {
        public string Name
        {
            get; set;
        }
        decimal numericVal;
        string stringVal;
        ArrayList children;

        public enum KVType
        {
            Numeric,
            String,
            Parent,
        }
        KVType myType;
        KeyValues myParent = null;

        public KeyValues(string name, int val)
        {
            Name = name; numericVal = val;
            myType = KVType.Numeric;
        }

        public KeyValues(string name, string val)
        {
            Name = name; stringVal = val;
            myType = KVType.String;
        }

        public KeyValues(string name)
        {
            Name = name;
            myType = KVType.Parent;
            children = new ArrayList();
        }

        static char[] whitespace = { ' ', '	' };
        public static KeyValues Create(string line)
        {
            if (line.StartsWith("//"))
                return new KeyValues(line);
            else if (line.IndexOfAny(whitespace) == -1)
                return new KeyValues(line.Substring(1, line.Length - 2));

            // each pair of items may be seperated by any combination of space and tab, and may or may not be in quote marks
            string name, value;
            int firstEndPos;
            if (line.StartsWith("\""))
            {// first item uses quotes, read until next quote
                firstEndPos = line.IndexOf('"', 1);
                name = line.Substring(1, firstEndPos-1);
            }
            else
            {// first item doesn't use quotes - read until first space/tab
                firstEndPos = line.IndexOfAny(whitespace);
                name = line.Substring(0, firstEndPos);
            }

            // then step past any seperator (space or tab)
            line = line.Substring(firstEndPos + 1).Trim();

            // and the value is the rest of it, remove quotes if present
            if (line.StartsWith("\"") && line.EndsWith("\""))
                value = line.Substring(1, line.Length - 2); // trim quotes off
            else
                value = line;

            /*
            // determine if we have a name and a value, or just a name
            string[] split = { "	" };
            split = line.Split(split, StringSplitOptions.RemoveEmptyEntries);
            if (split.Length == 0)
                return null;
            string name = split[0];
            if (name.StartsWith("\"") && name.EndsWith("\""))
                name = name.Substring(1, name.Length - 2); // trim quotes off

            if (split.Length == 1 || split[1].StartsWith("//"))
                return new KeyValues(name);

            // we have a value, decide if its a string or a number
            string value = split[1];
            if ( value.StartsWith("\"") && value.EndsWith("\"") )
                value = value.Substring(1,value.Length-2); // trim quotes off

            */

            int d;
            if (int.TryParse(value, out d))
                return new KeyValues(name, d);
            else
                return new KeyValues(name, value);
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();
            ToString(ref sb, 0);
            return sb.ToString();
        }

        public void ToString(ref StringBuilder sb, int depth)
        {
            for (int i = 0; i < depth; i++)
                sb.Append("	");

            if (Type == KVType.Parent)
            {
                sb.AppendFormat("\"{0}\"\r\n", Name);

                for (int i = 0; i < depth; i++)
                    sb.Append("	");
                sb.Append("{\r\n");

                foreach (KeyValues kv in children)
                    kv.ToString(ref sb, depth + 1);

                for (int i = 0; i < depth; i++)
                    sb.Append("	");
                sb.Append("}\r\n");
            }
            else
            {
                sb.AppendFormat("\"{0}\"	\"{1}\"\r\n", Name, Type == KVType.Numeric ? ((int)numericVal).ToString() : stringVal);
            }
        }

        public KVType Type
        {
            get { return myType; }
        }

        public object Value
        {
            get
            {
                switch (myType)
                {
                    case KVType.Numeric:
                        return numericVal;
                    case KVType.String:
                        return stringVal;
                    default:
                        return children;
                }
            }
        }

        public decimal NumericValue
        {
            get
            {
                if (myType != KVType.Numeric)
                    return 0;
                return numericVal;
            }

            set
            {
                if (myType != KVType.Numeric)
                    return;
                numericVal = value;
            }
        }

        public string StringValue
        {
            get
            {
                if (myType != KVType.String)
                    return "";
                return stringVal;
            }

            set
            {
                if (myType != KVType.String)
                    return;
                stringVal = value;
            }
        }

        public KeyValues[] Children
        {
            get
            {
                if (myType != KVType.Parent)
                    return null;

                return (KeyValues[])children.ToArray(typeof(KeyValues));
            }
        }

        public void AddChild(KeyValues child)
        {
            if (myType != KVType.Parent)
                return;

            child.Parent = this;
            children.Add(child);
        }

        public KeyValues Parent
        {
            get { return myParent; }
            set { myParent = value; }
        }
    }
}
