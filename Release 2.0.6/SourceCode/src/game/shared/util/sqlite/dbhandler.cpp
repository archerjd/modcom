#include "cbase.h"
#ifndef CLIENT_DLL
#include <string>
#endif
#include "dbhandler.h"
#include "tier0/icommandline.h"
#include "filesystem.h"
//#undef time
#include <time.h>

bool g_bDebugging = false;
dbHandler*	g_pDB = NULL;

#ifndef CLIENT_DLL
dbHandler*	g_pDB2 = NULL; // used for storing game statistics only
#define VarArgs UTIL_VarArgs

#ifndef CLIENT_DLL
CUtlVector<std::string> commandLog;
bool hasSavedErrorLog = false;
#endif

void DebugDatabase_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue );
ConVar debug_database( "debug_database", "0", FCVAR_CHEAT, "Show all database commands in the server console", true, 0, true, 1, DebugDatabase_ChangeCallback );
void DebugDatabase_ChangeCallback( IConVar *pConVar, char const *pOldString, float flOldValue )
{
	g_bDebugging = debug_database.GetBool();
}
#endif
	
clock_t startTime;
#define DEBUG_DURATION_START() startTime = clock()
#define DEBUG_DURATION_END(actionName) Msg(VarArgs("Database " actionName " took %.4f seconds\n", (float)((clock()-startTime)/CLOCKS_PER_SEC)))

dbHandler::dbHandler(const char *filename)
{
	m_iInTransaction = 0;
	char *fullFilePath = VarArgs("%s\\%s",CommandLine()->ParmValue( "-game", "hl2" ),filename);
	V_FixSlashes(fullFilePath);
	int rc = sqlite3_open(fullFilePath, Reference());
	if( rc && g_bDebugging )
		Msg("Failed to open database: %s\n", sqlite3_errmsg(Instance()));
}

dbHandler::~dbHandler()
{
	sqlite3_close(db);
	if ( g_bDebugging )
		Msg("database connection closed\n");
}

void dbHandler::ShowError(int returnCode, const char *action, const char *command, const char *customError)
{
	Warning(VarArgs("Database error #%i when %s command:\n%s\nError message: %s\n", returnCode, action, command, customError == NULL ? sqlite3_errmsg(db) : customError));
#ifndef CLIENT_DLL
	if ( g_bDebugging && this == g_pDB && !hasSavedErrorLog )
	{
		commandLog[commandLog.AddToTail()] = UTIL_VarArgs("error %s command!", action);

		// now need to write commandLog out to text file
		char dataDump[32562];
		Q_snprintf(dataDump, sizeof(dataDump), "");
		for ( int i=0; i<commandLog.Count(); i++ )
			Q_strcat(dataDump, UTIL_VarArgs("%s\r\n", commandLog[i].c_str()), sizeof(dataDump));
		
		FileHandle_t fh;
		const char *filename = "dberrorlog.cfg";
	   
		fh = filesystem->Open( filename, "w", "MOD");
		if (fh)
		{
			filesystem->FPrintf(fh, dataDump);
			filesystem->Close(fh);
			hasSavedErrorLog = true;
		}
	}
#endif
}

#ifndef CLIENT_DLL
ConVar mc_dev_disable_database_transactions("mc_dev_disable_database_transactions", "0", FCVAR_NOTIFY, "Disables all database transactions, for debugging purposes", true, 0, true, 1);
#endif
void dbHandler::BeginTransaction()
{
#ifndef CLIENT_DLL
	if ( mc_dev_disable_database_transactions.GetInt() )
		return;
#endif
	m_iInTransaction ++;
	if ( m_iInTransaction == 1 )
	{
		bool bDebugging = g_bDebugging;
		g_bDebugging = false; // don't write out BEGIN / COMMIT commands
		Command("BEGIN");
		g_bDebugging = bDebugging;
	}
}

void dbHandler::CommitTransaction()
{
#ifndef CLIENT_DLL
	if ( mc_dev_disable_database_transactions.GetInt() )
		return;
#endif
	m_iInTransaction = max(m_iInTransaction-1,0);
	if ( m_iInTransaction == 0 )
	{
		if ( g_bDebugging )
			DEBUG_DURATION_START();
			
		bool bDebugging = g_bDebugging;
		g_bDebugging = false; // don't write out BEGIN / COMMIT commands
		Command("COMMIT");
		g_bDebugging = bDebugging;

		if ( g_bDebugging )
			DEBUG_DURATION_END("transaction");
	}
}

// execute a command that outputs no data, returns true on success or false on failure
bool dbHandler::Command(const char *cmd, ...)
{
	if ( g_bDebugging && m_iInTransaction == 0 )
		DEBUG_DURATION_START();

	va_list marker;
	char msg[4096];
	va_start(marker, cmd);
	Q_vsnprintf(msg, sizeof(msg), cmd, marker);
	va_end(marker);
	//const char *command = VarArgs( msg );
	
	if ( g_bDebugging )
	{
		Msg("%s\n",msg);
#ifndef CLIENT_DLL
		if ( this == g_pDB )
			commandLog[commandLog.AddToTail()] = UTIL_VarArgs("command: %s", msg);		
#endif
	}
	bool retVal = true;
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, msg, -1, &stmt, 0);
	if( rc )
	{
		ShowError(rc, "preparing", msg);
		retVal = false;
	}
	else
	{
		rc = sqlite3_step(stmt);
		switch( rc )
		{
			case SQLITE_DONE:
			case SQLITE_OK:
#ifndef CLIENT_DLL
				if ( g_bDebugging && this == g_pDB )
					commandLog[commandLog.AddToTail()] = "ok";
#endif
				break;
			default:
				ShowError(rc, "processing", msg);
				retVal = false;
				break;
		}
		
		// finalize the statement to release resources
		rc = sqlite3_finalize(stmt);
		if( rc != SQLITE_OK)
			ShowError(rc, "finalising", msg);
	}
	
	if ( g_bDebugging && m_iInTransaction == 0 )
		DEBUG_DURATION_END("command");

	return retVal;
}

const char* dbHandler::ReadString(const char *cmd, ...)
{
	va_list marker;
	char msg[4096];
	va_start(marker, cmd);
	Q_vsnprintf(msg, sizeof(msg), cmd, marker);
	va_end(marker);
	//const char *command = VarArgs( msg );

	if ( g_bDebugging )
	{
		Msg("%s\n",msg);
		DEBUG_DURATION_START();
#ifndef CLIENT_DLL
		if ( this == g_pDB )
			commandLog[commandLog.AddToTail()] = UTIL_VarArgs("read string: ", msg);
#endif	
	}
	bool isnull = true;
	static char retVal[MAX_DB_STRING];

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, msg, -1, &stmt, 0);
	if( rc )
		ShowError(rc, "preparing", msg);
	else
	{
		//int cols = sqlite3_column_count(stmt);
		do
		{
			rc = sqlite3_step(stmt);
			switch( rc )
			{
				case SQLITE_DONE:
				case SQLITE_OK:
					break;
				case SQLITE_ROW:
					// print results for this row, the only row (hopefully)
					Q_snprintf(retVal, sizeof(retVal), (const char*)sqlite3_column_text(stmt, 0));
					isnull = false;
					break;
				default:
					ShowError(rc, "processing", msg);
					break;
			}
		} while( rc==SQLITE_ROW );
		// finalize the statement to release resources
		rc = sqlite3_finalize(stmt);
		if( rc != SQLITE_OK)
			ShowError(rc, "finalising", msg);
	}
	
	if ( g_bDebugging )
	{
		DEBUG_DURATION_END("read");
		Msg("returning: %s\n", isnull ? "null" : retVal);
	}
	return isnull ? NULL : retVal;
}

int dbHandler::ReadInt(const char *cmd, ...)
{
	va_list marker;
	char msg[4096];
	va_start(marker, cmd);
	Q_vsnprintf(msg, sizeof(msg), cmd, marker);
	va_end(marker);
	//const char *command = VarArgs( msg );
	
	if ( g_bDebugging )
	{
		Msg("%s\n",msg);
		DEBUG_DURATION_START();

#ifndef CLIENT_DLL	
		if ( this == g_pDB )
			commandLog[commandLog.AddToTail()] = UTIL_VarArgs("read int: ", msg);
#endif	
	}
	int retVal = -1;
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, msg, -1, &stmt, 0);
	if( rc != SQLITE_OK)
		ShowError(rc, "preparing", msg);
	else
	{
		//int cols = sqlite3_column_count(stmt);
		do
		{
			rc = sqlite3_step(stmt);
			switch( rc )
			{
				case SQLITE_DONE:
				case SQLITE_OK:
					break;
				case SQLITE_ROW:
					// print results for this row, the only row (hopefully)
					//retVal = sqlite3_column_int64(stmt, 0); - primary key values are int64, I think?
					retVal = sqlite3_column_int(stmt, 0);
					break;
				default:
					ShowError(rc, "processing", msg);
					break;
			}
		} while( rc==SQLITE_ROW );
		// finalize the statement to release resources
		rc = sqlite3_finalize(stmt);
		if( rc != SQLITE_OK)
			ShowError(rc, "finalising", msg);
	}
	
	if ( g_bDebugging )
	{
		DEBUG_DURATION_END("read");
		Msg(VarArgs("returning: %i\n",retVal));
	}
	return retVal;
}

dbReadResult* dbHandler::ReadMultiple(const char *cmd, ...)
{
	va_list marker;
	char msg[4096];
	va_start(marker, cmd);
	Q_vsnprintf(msg, sizeof(msg), cmd, marker);
	va_end(marker);
	//const char *command = VarArgs( msg );
	
	if ( g_bDebugging )
	{
		Msg("%s\n",msg);
		DEBUG_DURATION_START();
#ifndef CLIENT_DLL
		if ( this == g_pDB )
			commandLog[commandLog.AddToTail()] = UTIL_VarArgs("read: ", msg);
#endif	
	}
	dbReadResult* output = new dbReadResult();
	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, msg, -1, &stmt, 0);
	if( rc )
	{
		ShowError(rc, "preparing", msg);
	}
	else
	{
		int cols = sqlite3_column_count(stmt);
		// execute the statement
		do
		{
			rc = sqlite3_step(stmt);
			switch( rc )
			{
				case SQLITE_DONE:
				case SQLITE_OK:
					break;
				case SQLITE_ROW:
					for( int col=0; col<cols; col++)
						switch ( sqlite3_column_type(stmt, col) )
						{
							case SQLITE_INTEGER:
								output->AddToTail(dbValue(sqlite3_column_int(stmt, col))); break;
							case SQLITE_TEXT:
								output->AddToTail(dbValue((const char*)sqlite3_column_text(stmt, col))); break;
							case SQLITE_FLOAT:
								output->AddToTail(dbValue(sqlite3_column_double(stmt, col))); break;
							case SQLITE_BLOB:
							case SQLITE_NULL:
							default:
								ShowError(rc, "processing", msg, "Data type is not supported; must be SQLITE_INTEGER, SQLITE_FLOAT or SQLITE_TEXT!"); break;
						}
					break;
				default:
					ShowError(rc, "processing", msg);
					break;
			}
		} while( rc==SQLITE_ROW );
		// finalize the statement to release resources
		rc = sqlite3_finalize(stmt);
		if( rc != SQLITE_OK)
			ShowError(rc, "finalising", msg);
	}

	if ( g_bDebugging )
	{
		DEBUG_DURATION_END("read");
		Msg("returning: ");
		int num = output->Count();
		for (int i=0; i<num; i++ )
		{
			if ( i > 0 )
				Msg(", ");
			switch ( output->Element(i).type )
			{
			case INTEGER:
				Msg(VarArgs("%i",output->Element(i).integer)); break;
			case TEXT:
				Msg(VarArgs("%s",output->Element(i).text)); break;
			case FLOATING:
				Msg(VarArgs("%f",output->Element(i).floating)); break;
			default:
				Msg("<INVALID>"); break;
			}
		}
		Msg("\n");
	}
	return output;
}

int dbHandler::LastInsertID()
{
	return sqlite3_last_insert_rowid(db);
}