using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Data.SQLite;
using System.IO;
using NPlot;
using System.Drawing.Imaging;

namespace DataCollator
{
    public partial class Form2 : Form
    {
        const int MAX_PLAYER_LEVEL = 25;
        const int MAX_MODULE_LEVEL = 10;
        const int MAX_WEAPON_UPGRADE_LEVEL = 10;

        static string[] IndependentVariables =
        {
            "player level",
            "module level", //[specific module must be chosen]
            "game mode",
            "player faction",
            "map",
        };

        static string[] DependentVariables =
        {
            "number of players",
            "exp per minute",
            "player kills per minute",
            "monster kills per minute",
            "player deaths per minute",
            "monster deaths per minute",
            "total kills per minute",
            "total deaths per minute",
            //"(projected) total time to complete level",
            "player kills : deaths ratio",
            "monster kills : deaths ratio",
            "total kills : deaths ratio",
        };

        // key is command name, value is display name
        static SortedList<string, string> Modules = new SortedList<string,string>();
        static SortedList<int, string> GameModes = new SortedList<int, string>();
        static SortedList<int, string> Factions = new SortedList<int, string>();
        static List<string> MapList = new List<string>();

// some method of showing the number of players on a faction relative to the other factions in a game, and the overall score rankings, would be good...
// can't think how to formulate that right now

        public static string moduleListFilePath;
        public Form2()
        {
            InitializeComponent();
            Hide();

            Form f = new Form1();
            f.ShowDialog(this);

            if (moduleListFilePath == null)
            {
                Application.Exit();
                return;
            }

            foreach ( string s in IndependentVariables )
                lstIndependentVariable.Items.Add(s);
            foreach (string s in DependentVariables)
                lstDependentVariable.Items.Add(s);

            #region game data list population
            string[] moduleLines = File.ReadAllLines(moduleListFilePath);
            foreach (string l in moduleLines)
            {
                if ( l.Trim().Length == 0 )
                    continue;

                string[] split = l.Split(new char[] { ' ' }, 2);
                Modules.Add(split[1], split[0]); // english name, command name
            }

            GameModes.Add(1, "Deathmatch");
            GameModes.Add(2, "PvM");
            GameModes.Add(3, "FFA");
            GameModes.Add(4, "Team DM");
            //GameModes.Add(5, "Team FFA");

            Factions.Add(1, "Combine");
            Factions.Add(2, "Resistance");
            Factions.Add(3, "Aperture");
            #endregion

            popLstModules.Items.Add("All modules", true);
            foreach (string s in Modules.Keys)
                popLstModules.Items.Add(s);

            foreach (string gameMode in GameModes.Values)
                popLstGameMode.Items.Add(gameMode, true);

            ddlSplitBy.SelectedIndex = 0;
            lstDateOlderNewer.SelectedIndex = 0;
        }

        protected override void OnShown(EventArgs e)
        {
            base.OnShown(e);
            if (moduleListFilePath == null)
                return;
            dataGridView1.DataSource = SqlInterface.RunQuery("select name as Tables from sqlite_master where type = 'table' order by name", SqlInterface.db);

            // populate Version list
            DataTable versions = SqlInterface.RunQuery("select distinct Version from game order by Version desc", SqlInterface.db);
            if (versions == null)
            {
                MessageBox.Show("No data loaded", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            foreach ( DataRow dr in versions.Rows )
                popLstVersion.Items.Add(dr[0]);
            if (popLstVersion.Items.Count > 1)
                popLstVersion.SetItemChecked(1, true);

            // populate map list
            DataTable maps = SqlInterface.RunQuery("select distinct mapname from game order by mapname", SqlInterface.db);
            popLstMap.Items.Add("All maps", true);
            foreach (DataRow dr in maps.Rows)
            {// list all maps used. For 2.0.3 and older where map name isn't stored, don't show the blank value as a search option
                if (dr[0] == null || dr[0].ToString() == string.Empty)
                    continue;
                popLstMap.Items.Add(dr[0], false);
                MapList.Add(dr[0].ToString());
            }
        }

        protected override bool ProcessDialogKey(Keys keyData)
        {
            if (keyData == Keys.F5)
            {
                if (btnQuery.Visible)
                    btnQuery_Click(this, null);
                return true;
            }
            else if (keyData == Keys.F6)
            {
                if (btnUpdate.Visible)
                    btnUpdate_Click(this, null);
                return true;
            }

            return false;
        }

        private string QueryText
        {
            get
            {
                if (txtQuery.SelectionLength > 0)
                    return txtQuery.SelectedText;
                return txtQuery.Text;
            }
        }

        private void btnQuery_Click(object sender, EventArgs e)
        {
            dataGridView1.DataSource = SqlInterface.RunQuery(QueryText, SqlInterface.db);
        }

        private void btnUpdate_Click(object sender, EventArgs e)
        {
            SqlInterface.RunCommand(QueryText, SqlInterface.db);
        }

        private void lstIndependentVariable_SelectedIndexChanged(object sender, EventArgs e)
        {
            string selectedItem = lstIndependentVariable.SelectedItem as string;
            popLstModules.Enabled = selectedItem == "module level";
            popLstGameMode.Enabled = selectedItem != "game mode";
        }

        static Color[] colorOrder = new Color[] { Color.Red, Color.Blue, Color.Green, Color.Orange, Color.LightBlue, Color.Purple, Color.Fuchsia, Color.LightGreen, Color.Brown, Color.Gray };

        private void RenderGraph()
        {
            string[] labels;
            string[] baseQueries;

            plotSurface2D1.Clear();
            bool splitByGameMode = ddlSplitBy.SelectedItem.ToString() == "Game Mode" && popLstGameMode.CheckedIndices.Count != 0;
            bool splitByModule = ddlSplitBy.SelectedItem.ToString() == "Module" && popLstModules.CheckedIndices.Count != 0;
            bool splitByVersion = ddlSplitBy.SelectedItem.ToString() == "Version" && popLstVersion.CheckedIndices.Count != 0;
            bool splitByMap = ddlSplitBy.SelectedItem.ToString() == "Map" && popLstMap.CheckedIndices.Count != 0;

            // work out what we're selecting from, based on independent variable ... also work out x-axis labels
            switch (lstIndependentVariable.SelectedItem as string)
            {
                case "player level":
                    labels = new string[MAX_PLAYER_LEVEL];
                    baseQueries = new string[MAX_PLAYER_LEVEL];
                    for (int i = 0; i < MAX_PLAYER_LEVEL; i++)
                    {
                        labels[i] = (i+1).ToString();
                        baseQueries[i] = " from gameplayer where plevel = " + (i + 1);
                    }
                    break;
                case "module level":
                    if (popLstModules.CheckedItems.Count == 0)
                        return;
                    labels = new string[MAX_MODULE_LEVEL + 1];
                    baseQueries = new string[MAX_MODULE_LEVEL + 1];
                    
                    labels[0] = "0";
                    for (int i = 1; i <= MAX_MODULE_LEVEL; i++)
                        labels[i] = i.ToString();
                    break;
                case "game mode":
                    labels = new string[GameModes.Count];
                    baseQueries = new string[GameModes.Count];
                    for (int i = 1; i <= GameModes.Count; i++)
                    {
                        labels[i-1] = GameModes[i];
                        baseQueries[i - 1] = " from gameplayer where gameid in (select id from game where gamemode = " + i + ")";
                    }
                    break;
                case "player faction":
                    labels = new string[Factions.Count];
                    baseQueries = new string[Factions.Count];
                    for (int i = 1; i <= Factions.Count; i++)
                    {
                        labels[i] = Factions[i];
                        baseQueries[i] = " from gameplayer where faction = " + i;
                    }
                    break;
                case "map":
                    labels = new string[MapList.Count];
                    baseQueries = new string[MapList.Count];
                    for (int i = 0; i < MapList.Count; i++)
                    {
                        labels[i] = MapList[i];
                        baseQueries[i] = " from gameplayer where gameid in (select id from game where mapname = '" + MapList[i] + "')";
                    }
                    break;
                default:
                    return;
            }
            LabelAxis xAxis = new LabelAxis();
            //xAxis.AutoScaleTicks = false;
            xAxis.TickTextNextToAxis = false;
            for (int i = 0; i < labels.Length; i++)
                xAxis.AddLabel(labels[i], i);
            xAxis.WorldMin = -1;
            xAxis.WorldMax = labels.Length;
            xAxis.TicksCrossAxis = true;
            xAxis.Label = lstIndependentVariable.SelectedItem as string;
            plotSurface2D1.XAxis1 = xAxis;

            LinearAxis yAxis = new LinearAxis();
            yAxis.Label = lstDependentVariable.SelectedItem as string;
            plotSurface2D1.YAxis1 = yAxis;

            // now work out what we're selecting, based on independent variable
            string queryStart;
            switch (lstDependentVariable.SelectedItem as string)
            {
                case "number of players":
                    queryStart = "select count(gameid)";
                    break;
                case "exp per minute":
                    queryStart = "select gameexp * 60 / duration";
                    break;
                case "player kills per minute":
                    queryStart = "select playerkills * 60 / duration";
                    break;
                case "monster kills per minute":
                    queryStart = "select monsterkills * 60 / duration";
                    break;
                case "player deaths per minute":
                    queryStart = "select playerdeaths * 60 / duration";
                    break;
                case "monster deaths per minute":
                    queryStart = "select monsterdeaths * 60 / duration";
                    break;
                case "total kills per minute":
                    queryStart = "select (playerkills + monsterkills) * 60 / duration";
                    break;
                case "total deaths per minute":
                    queryStart = "select (playerdeaths + monsterdeaths) * 60 / duration";
                    break;
                /*case "(projected) total time to complete level":
                    queryStart = "select gameexp / duration"; // incomplete... need to incorporate total exp needed for each level, somehow
                    break;*/
                case "player kills : deaths ratio":
                    queryStart = "select (playerkills+1) / (playerdeaths+1)";
                    break;
                case "monster kills : deaths ratio":
                    queryStart = "select (monsterkills+1) / (monsterdeaths+1)";
                    break;
                case "total kills : deaths ratio":
                    queryStart = "select (playerkills + monsterkills + 1) / (playerdeaths + monsterdeaths + 1)";
                    break;
                default:
                    return;
            }

            string[] extraWhere, lineNames; string gameWhere = string.Empty;
            if ( splitByGameMode ) // plotting game modes separatey!
            {
                extraWhere = new string[popLstGameMode.CheckedIndices.Count];
                extraWhere[0] = string.Empty;
            }
            else if (splitByModule)
            {
                extraWhere = new string[popLstModules.CheckedIndices.Count];
                extraWhere[0] = string.Empty;
            }
            else if (splitByVersion)
            {
                extraWhere = new string[popLstVersion.CheckedIndices.Count];
                extraWhere[0] = string.Empty;
            }
            else if (splitByMap)
            {
                extraWhere = new string[popLstMap.CheckedIndices.Count];
                extraWhere[0] = string.Empty;
            }
            else
                extraWhere = new string[] { string.Empty };

            if (checkNeedKillsOrDeaths.Checked)
                extraWhere[0] += " AND (gameexp <> 0 or playerdeaths <> 0 or monsterdeaths <> 0)";
            if ( checkNeedsDuration.Checked )
            {
                double d;
                if ( double.TryParse(txtMinDuration.Text, out d) )
                    extraWhere[0] += " AND Duration > " + d;
                else
                    checkNeedsDuration.Checked = false;
            }
            if (checkDateRange.Checked)
            {
                string comparison = lstDateOlderNewer.SelectedItem.ToString() == "newer than" ? ">" : "<";
                long timeStamp = (long)(dateTimePicker1.Value - new DateTime(1970, 1, 1, 0, 0, 0)).TotalSeconds;
                extraWhere[0] += " AND gameid " + comparison + " 'Stats_" + timeStamp + "_'";
            }
            if (chkNotCheats.Checked)
            {
                gameWhere += " AND cheatsenabled <> 1";
            }
            if (chkOnlyDefaults.Checked)
            {
                gameWhere += " AND defaultconvars <> 0";
            }

            // if all game modes are checked, don't bother adjusting the query
            if (!splitByGameMode && popLstGameMode.Enabled && popLstGameMode.CheckedIndices.Count != popLstGameMode.Items.Count) 
            {
                if (popLstGameMode.CheckedIndices.Count == 0)
                    gameWhere += " AND gamemode = 0"; // select none!
                else
                {
                    gameWhere += " AND gamemode in (" + (popLstGameMode.CheckedIndices[0] + 1);
                    for (int i = 1; i < popLstGameMode.CheckedIndices.Count; i++)
                        gameWhere += ", " + (popLstGameMode.CheckedIndices[i] + 1);
                    gameWhere += ")";
                }
            }
            if (!splitByModule && popLstModules.CheckedIndices.Count != popLstModules.Items.Count)
            {
                if (lstIndependentVariable.SelectedItem.ToString() == "module level")
                {
                    for (int i = 0; i <= MAX_MODULE_LEVEL; i++)
                        baseQueries[i] = " from gameplayer where (";

                    bool firstItem = true;
                    for (int i = 0; i < popLstModules.CheckedItems.Count; i++)
                    {
                        if (firstItem)
                            firstItem = false;
                        else
                            for (int j = 0; j <= MAX_MODULE_LEVEL; j++)
                                baseQueries[j] += " OR ";

                        string moduleName = popLstModules.CheckedItems[i].ToString();
                        if (moduleName == "All modules")
                            moduleName = string.Empty;
                        else
                            moduleName = Modules[moduleName];
                        
                        baseQueries[0] += string.Format("modules like '%{0};0;%' OR modules not like '%{0};%' ", moduleName);
                        for (int j = 1; j <= MAX_MODULE_LEVEL; j++)
                            baseQueries[j] += string.Format("modules like '%{0};" + j + ";%'", moduleName);
                    }

                    for (int i = 0; i <= MAX_MODULE_LEVEL; i++)
                        baseQueries[i] += ")";
                }
            }
            if (!splitByVersion && popLstVersion.CheckedIndices.Count != popLstVersion.Items.Count)
            {
                if (popLstVersion.CheckedIndices.Count == 0)
                    gameWhere += " AND version = null";
                else
                {
                    gameWhere += " AND version in ('" + popLstVersion.CheckedItems[0].ToString();
                    for (int i = 1; i < popLstVersion.CheckedIndices.Count; i++)
                        gameWhere += "', '" + popLstVersion.CheckedItems[i].ToString();
                    gameWhere += "')";
                }
            }
            if (!splitByMap && popLstMap.CheckedIndices.Count != popLstMap.Items.Count)
            {
                if (popLstMap.CheckedIndices.Count == 0)
                    gameWhere += " AND version = null";
                else if ( !popLstMap.CheckedItems.Contains("All maps") ) // if "All maps" is checked, and we're not splitting, do no map filter
                {
                    gameWhere += " AND mapname in ('" + popLstMap.CheckedItems[0].ToString();
                    for (int i = 1; i < popLstMap.CheckedIndices.Count; i++)
                        gameWhere += "', '" + popLstMap.CheckedItems[i].ToString();
                    gameWhere += "')";
                }
            }
            
            string[] modules = new string[0];
            if (splitByGameMode)
            {
                lineNames = new string[extraWhere.Length];

                extraWhere[0] += " AND gameid in (select id from game where gamemode = ";
                for (int i = extraWhere.Length - 1; i >= 0; i--)
                {
                    extraWhere[i] = extraWhere[0] + (popLstGameMode.CheckedIndices[i] + 1) + ")";
                    lineNames[i] = GameModes[popLstGameMode.CheckedIndices[i] + 1];
                }
            }
            else if (splitByModule)
            {
                baseQueries[0] = " from gameplayer where (modules like '%{0};0;%' OR modules not like '%{0};%')";
                for (int i = 1; i <= MAX_MODULE_LEVEL; i++)
                {
                    labels[i] = i.ToString();
                    baseQueries[i] = " from gameplayer where modules like '%{0};" + i + ";%'";
                }

                modules = new string[popLstModules.CheckedItems.Count];
                lineNames = new string[modules.Length];


                for (int i = extraWhere.Length - 1; i >= 0; i--)
                {
                    lineNames[i] = popLstModules.CheckedItems[i] as string;
                    if (lineNames[i] == "All modules")
                        modules[i] = string.Empty;
                    else
                        modules[i] = Modules[lineNames[i]];

                    extraWhere[i] = extraWhere[0];
                }
            }
            else if (splitByVersion)
            {
                lineNames = new string[popLstVersion.CheckedItems.Count];
                extraWhere[0] += " AND gameid in (select id from game where version = '";
                for (int i = extraWhere.Length - 1; i >= 0; i--)
                {
                    extraWhere[i] = extraWhere[0] + popLstVersion.CheckedItems[i].ToString() + "')";
                    lineNames[i] = popLstVersion.CheckedItems[i].ToString();
                }
            }
            else if (splitByMap)
            {
                // needs to handle "All maps"

                lineNames = new string[popLstMap.CheckedItems.Count];
                string extra = " AND gameid in (select id from game where mapname = '";
                for (int i = extraWhere.Length - 1; i >= 0; i--)
                {
                    string itemName = popLstMap.CheckedItems[i].ToString();
                    if ( itemName != "All maps" )
                        extraWhere[i] = extraWhere[0] + extra + itemName + "')";
                    lineNames[i] = itemName;
                }
            }
            else
            {
                lineNames = new string[] { "Plot" };
            }


            if (gameWhere != string.Empty)
                for (int i = 0; i < extraWhere.Length; i++)
                    extraWhere[i] += " AND gameid in (select id from game where" + gameWhere.Substring(4) + ")"; // chop first " AND" off of gamewhere

            // this is where we should loop if we're to split by something (game mode, module, version)!
            for (int plot = 0; plot < lineNames.Length; plot++)
            {
                string[] queries = new string[baseQueries.Length];
                if (splitByModule) // when splitting by modules, we need to insert module name
                    for (int i = 0; i < queries.Length; i++)
                        queries[i] = queryStart + string.Format(baseQueries[i], modules[plot]) + extraWhere[0];
                else // when splitting by game mode, version, or not splitting, we just use all the extra where clauses
                    for (int i = 0; i < queries.Length; i++)
                        queries[i] = queryStart + baseQueries[i] + extraWhere[plot];

                // having built a single query for each data column, get the data
                ValueSet[] graphData = new ValueSet[queries.Length];
                for (int i = 0; i < queries.Length; i++)
                    graphData[i] = new ValueSet(queries[i]);

                int c = plot;
                while (c > colorOrder.Length) // loop through the color list if we have too many lines
                    c -= colorOrder.Length;
                Color itemColor = colorOrder[c];

                if (checkQuartiles.Checked)
                {
                    float[] values = new float[queries.Length];
                    for (int i = 0; i < values.Length; i++)
                        values[i] = graphData[i].GetUpperQuartile();
                    LinePlot line1 = new LinePlot(values);

                    values = new float[queries.Length];
                    for (int i = 0; i < values.Length; i++)
                        values[i] = graphData[i].GetLowerQuartile();
                    LinePlot line2 = new LinePlot(values);

                    if (checkFillIQR.Checked)
                    {
                        FilledRegion fr = new FilledRegion(line1, line2);
                        fr.Brush = new SolidBrush(Color.FromArgb(48, itemColor));
                        plotSurface2D1.Add(fr);
                    }
                    else
                    {
                        line1.Label = lineNames[plot] + " (upper quartile)";
                        line2.Label = lineNames[plot] + " (lower quartile)"; // unless we give this a seperate label, it shows on the legend as "Series X" (where X is a #)
                        line1.Color = line2.Color = itemColor;
                        plotSurface2D1.Add(line1);
                        plotSurface2D1.Add(line2);
                    }
                }
                if (checkMean.Checked)
                {
                    float[] values = new float[queries.Length];
                    for (int i = 0; i < values.Length; i++)
                        values[i] = graphData[i].GetMean();

                    LinePlot line = new LinePlot(values);
                    line.Label = lineNames[plot] + " (mean)";
                    line.Color = itemColor;
                    line.Pen.Width = 2;
                    plotSurface2D1.Add(line);
                }
                if (checkMedian.Checked || (checkQuartiles.Checked && checkFillIQR.Checked && !checkMean.Checked))
                {
                    float[] values = new float[queries.Length];
                    for (int i = 0; i < values.Length; i++)
                        values[i] = graphData[i].GetMedian();

                    LinePlot line = new LinePlot(values);
                    line.Label = lineNames[plot] + " (median)";
                    line.Color = itemColor;
                    //line.Pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
                    line.Pen.Width = 1;
                    plotSurface2D1.Add(line);
                }

                string[] columnLabels = new string[queries.Length];
                if (checkNumPoints.Checked)
                {
                    for (int i = 0; i < queries.Length; i++)
                        columnLabels[i] += "Num: " + graphData[i].Count + Environment.NewLine;
                }
                if (checkStandardDeviation.Checked)
                {
                    for (int i = 0; i < queries.Length; i++)
                        columnLabels[i] += "SD:    " + graphData[i].GetStandardDeviation() + Environment.NewLine;
                }
                /*if (checkLabelIQR.Checked)
                {
                    for (int i = 0; i < queries.Length; i++)
                        columnLabels[i] += "IQR:  " + graphData[i].GetInterQuartileRange();
                }*/

                Font labelFont = new Font(this.Font.FontFamily, 6, FontStyle.Regular);
                for (int i = 0; i < columnLabels.Length; i++)
                    if (!string.IsNullOrEmpty(columnLabels[i]))
                    {
                        TextItem ti = new TextItem(new PointD(i - 0.2, plotSurface2D1.YAxis1.WorldMax), Environment.NewLine + columnLabels[i]);
                        ti.TextFont = labelFont;
                        plotSurface2D1.Add(ti);
                    }
            }

            //if (lineNames.Length > 1)
            if ( checkLegend.Checked )
                plotSurface2D1.Legend = new Legend();
            else
                plotSurface2D1.Legend = null;
        }

        private bool yMinIsAuto = true, yMaxIsAuto = true;

        private void btnGo_Click(object sender, EventArgs e)
        {
            RenderGraph();

            bool setYminAuto = yMinIsAuto, setYmaxAuto = yMaxIsAuto;
            double yMin, yMax;
            if (yMinIsAuto || (!double.TryParse(txtYmin.Text, out yMin) && plotSurface2D1.YAxis1 != null))
            {
                yMin = plotSurface2D1.YAxis1.WorldMin;
                setYminAuto = true;
            }

            if (yMaxIsAuto || (!double.TryParse(txtYmax.Text, out yMax) && plotSurface2D1.YAxis1 != null))
            {
                yMax = plotSurface2D1.YAxis1.WorldMax;
                setYmaxAuto = true;
            }

            if (yMin > yMax)
            {
                double temp = yMin;
                yMin = yMax;
                yMax = temp;
            }
            if (yMin < yMax && plotSurface2D1.YAxis1 != null)
            {
                plotSurface2D1.YAxis1.WorldMin = yMin;
                plotSurface2D1.YAxis1.WorldMax = yMax;
            }

            plotSurface2D1.Refresh();

            if (setYminAuto)
            {
                txtYmin.Text = yMin.ToString();
                yMinIsAuto = true;
            }
            if (setYmaxAuto)
            {
                txtYmax.Text = yMax.ToString();
                yMaxIsAuto = true;
            }
        }

        private void txtYmin_TextChanged(object sender, EventArgs e)
        {
            yMinIsAuto = false;
        }

        private void txtYmax_TextChanged(object sender, EventArgs e)
        {
            yMaxIsAuto = false;
        }

        private void lnkSave_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "Png files (*.png)|*.png|All files (*.*)|*.*";
            sfd.Title = "Enter filename to save";
            //sfd.InitialDirectory = ; // look up steam directory?
            sfd.CheckFileExists = false;
            if (sfd.ShowDialog() != DialogResult.OK)
                return;

            Bitmap bitmap = new Bitmap(plotSurface2D1.Width, plotSurface2D1.Height, PixelFormat.Format24bppRgb);
            plotSurface2D1.DrawToBitmap(bitmap, new Rectangle(0,0,plotSurface2D1.Width, plotSurface2D1.Height));
            bitmap.Save(sfd.FileName, ImageFormat.Png);
        }

        private void checkQuartiles_CheckedChanged(object sender, EventArgs e)
        {
            checkFillIQR.Enabled = checkQuartiles.Checked;
        }
    }
}
