using System;
using System.Collections.Generic;
using System.Text;
using System.Data.SQLite;
using System.Data;
using System.Windows.Forms;

namespace DataCollator
{
    static class SqlInterface
    {
        public const string filename = "collated.db";
        public static SQLiteConnection db;
        static SQLiteCommand cmd;
        static SQLiteDataAdapter adapter;
        static DataSet ds;

        public static DataTable RunQuery(string q, SQLiteConnection sql)
        {
            sql.Open();
            ds = new DataSet();
            cmd = sql.CreateCommand();
            cmd.CommandText = q;
            adapter = new SQLiteDataAdapter(cmd);
            DataTable retVal;
            try
            {
                adapter.Fill(ds);
                retVal = ds.Tables[0];
            }
            catch ( Exception )
            {
                retVal = null;
            }
            sql.Close();
            return retVal;
        }

        public static void RunCommand(string c, SQLiteConnection sql)
        {
            if (c.Trim() == string.Empty)
                return;

            //dataGridViewQuery.DataSource = null;
            sql.Open();

            // run the query
            cmd = sql.CreateCommand();
            cmd.CommandText = c;

            try
            {
                int affected = cmd.ExecuteNonQuery();
                //MessageBox.Show(affected + " rows affected");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "SQLite Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            sql.Close();
        }
    }
}
