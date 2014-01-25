using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Data.SQLite;

namespace MCDBBrowser
{
    public partial class Form1 : Form
    {
        SQLiteConnection db;
        SQLiteCommand cmd;
        SQLiteDataAdapter adapter;
        DataSet ds;
        public Form1()
        {
            InitializeComponent();
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.CheckFileExists = false;
            ofd.Filter = "Database files|*.db";
            ofd.Title = "Select a database file";
            if (ofd.ShowDialog() != DialogResult.OK)
            {
                Application.Exit();
                return;
            }

            // load database: ofd.FileName;
            db = new SQLiteConnection(string.Format("Data Source={0};Version=3;", ofd.FileName));
            cmd = db.CreateCommand();

            try
            {
                RunQuery("select name as Tables from sqlite_master where type = 'table' order by name", dataGridViewQuery);
            }
            catch (SQLiteException e)
            {
                MessageBox.Show(e.Message, "Database error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Application.Exit();
            }

            ReadAccounts();
            ReadCharacters();
            ReadModules();
            ReadWeapons();

            BringToFront();
        }

        private void RunQuery(string q, DataGridView destination)
        {
            // now load all accounts and characters
            db.Open();
            ds = new DataSet();
            cmd.CommandText = q;
            adapter = new SQLiteDataAdapter(cmd);
            try
            {
                adapter.Fill(ds);
                destination.DataSource = ds.Tables[0];
            }
            catch
            {
                destination.DataSource = null;
            }
            db.Close();
        }

        private void ReadAccounts()
        {
            RunQuery("select * from Accounts", dataGridViewAccounts);
        }

        private void ReadCharacters()
        {
            RunQuery("select * from Characters", dataGridViewCharacters);
        }

        private void ReadModules()
        {
            RunQuery("select * from CharacterModules order by CharacterID, Module", dataGridViewModules);
        }

        private void ReadWeapons()
        {
            RunQuery("select * from CharacterWeaponUpgrades order by CharacterID, Upgrade", dataGridViewWeapons);
        }

        ~Form1()
        {
            if ( db != null && db.State != ConnectionState.Closed )
                db.Close();
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
            if (QueryText.Trim() == string.Empty)
                return;

            ds = new DataSet();
            db.Open();

            // run the query
            cmd.CommandText = QueryText;
            adapter = new SQLiteDataAdapter(cmd);
            try
            {
                adapter.Fill(ds);
                dataGridViewQuery.DataSource = null;
                dataGridViewQuery.DataSource = ds.Tables[0];
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SQLite Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            db.Close();
        }

        private void btnUpdate_Click(object sender, EventArgs e)
        {
            if (QueryText.Trim() == string.Empty)
                return;

            dataGridViewQuery.DataSource = null;
            db.Open();

            // run the query
            cmd.CommandText = QueryText;

            try
            {
                int affected = cmd.ExecuteNonQuery();
                MessageBox.Show(affected + " rows affected");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SQLite Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            db.Close();
        }

        private void txtQuery_DoubleClick(object sender, EventArgs e)
        {
            txtQuery.SelectAll();
        }

        protected override bool ProcessDialogKey(Keys keyData)
        {
            if (keyData == Keys.F5)
            {
                if (tabControl1.SelectedTab == tabQuery)
                    btnQuery_Click(this, null);
                else if (tabControl1.SelectedTab == tabAccounts)
                    ReadAccounts();
                else if (tabControl1.SelectedTab == tabChars)
                    ReadCharacters();
                else if (tabControl1.SelectedTab == tabModules)
                    ReadModules();
                else if (tabControl1.SelectedTab == tabWeapons)
                    ReadWeapons();
                return true;
            }
            else if (keyData == Keys.F6 && tabControl1.SelectedTab == tabQuery)
            {
                btnUpdate_Click(this, null);
                return true;
            }

            return false;
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            string query;
            switch (comboBox1.SelectedItem as string)
            {
                case "Characters, sorted by experience":
                    query = "select name, (select name from accounts where id = accountid) as Account, exp, level from characters order by exp desc";
                    break;
                case "Characters, sorted by best spree":
                    query = "select name, (select name from accounts where id = accountid) as Account, bestspree from characters order by bestspree desc";
                    break;
                case "Characters, sorted by player kill / death ratio":
                    query = "select name, (select name from accounts where id = accountid) as Account, playerkills, playerdeaths, (cast(playerkills as floating)/cast(playerdeaths as floating)) as Ratio from characters order by ratio desc";
                    break;
                case "Characters, sorted by monster kill / death ratio":
                    query = "select name, (select name from accounts where id = accountid) as Account, monsterkills, monsterdeaths, (cast(monsterkills as floating)/cast(monsterdeaths as floating)) as Ratio from characters order by ratio desc";
                    break;
                case "Characters, sorted by time played":
                    query = "select name, (select name from accounts where id = accountid) as Account, timeplayed from characters order by timeplayed desc";
                    break;
                case "Accounts, sorted by time played":
                    query = "select Name, (select sum(timeplayed) from characters where accountid = accounts.id) as time from accounts order by time desc";
                    break;
                case "Factions, character distribution":
                    query = "select (select count(id) from characters where faction = 1) as Combine, (select count(id) from characters where faction = 2) as Resistance, (select count(id) from characters where faction = 3) as Aperture";
                    break;
                case "Default weapons, character distribution":
                    query = "select (select count(id) from characters where defaultweapon = 'weapon_smg1') as SMG, (select count(id) from characters where defaultweapon = 'weapon_357') as m357, (select count(id) from characters where defaultweapon = 'weapon_ar2') as AR2, (select count(id) from characters where defaultweapon = 'weapon_shotgun') as Shotgun";
                    break;
                default:
                    MessageBox.Show("Unknown preset selected");
                    return;
            }

            RunQuery(query, dataGridViewPresets);
        }
    }
}
