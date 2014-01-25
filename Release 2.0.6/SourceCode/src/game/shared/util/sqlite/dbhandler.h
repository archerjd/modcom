#ifndef DBHANDLER_H
#define DBHANDLER_H
#pragma once

#include "sqlite3.h"
#include "utlvector.h"

// set this as you see fit
#define MAX_DB_STRING	4096					// max length of a string returned from database

enum ValueType
{
	ISNULL = 0,
	INTEGER,
	FLOATING,
	TEXT,
};

struct dbValue
{
public:
	dbValue(bool isNull)
	{
		type = ISNULL;
	}

	dbValue(int value)
	{
		type = INTEGER;
		integer = value;
	}
	
	dbValue(double value)
	{
		type = FLOATING;
		floating = value;
	}
	
	dbValue(const char* value)
	{
		type = TEXT;
		Q_snprintf(text, sizeof(text), value);
	}
	
	ValueType type;
	int integer;
	double floating;
	char text[MAX_DB_STRING];
};

typedef CUtlVector<dbValue> dbReadResult;

class dbHandler
{
public:
	dbHandler(const char *filename);
	~dbHandler();

	// calling a series of commands between BeginTransaction and EndTransaction will significantly increase write speed...
	// see http://www.sqlite.org/faq.html#q19 for more information. These functions ensure that nested calls will cause no harm
	void BeginTransaction();
	void CommitTransaction();

	bool Command(const char *cmd, ...);
	const char* ReadString(const char *cmd, ...);
	int ReadInt(const char *cmd, ...);
	dbReadResult* ReadMultiple(const char *cmd, ...);

	void ShowError(int returnCode, const char *action, const char *command, const char *customError=NULL);
	sqlite3 *Instance() { return db; }
	sqlite3 **Reference() { return &db; }
	int	LastInsertID();
private:
	sqlite3 *db;
	int m_iInTransaction;
};

extern dbHandler *g_pDB;

#ifndef CLIENT_DLL
extern dbHandler *g_pDB2; //  used for storing game statistics only
#endif

#endif