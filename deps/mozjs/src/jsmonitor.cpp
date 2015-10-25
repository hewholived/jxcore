#include "jsmonitor.h"

js::JSMonitor::JSMonitor() {
	char *sql;
	int rc = 0;
	char *zErrMsg = 0;

	if (sqlite3_open("monitor.db", &monitorDb) != 0) {
	  printf("Monitor: Db init error.\n");
	  return;
	}

	sql = "CREATE TABLE HOTFUNCS("  \
		  "NAME           CHAR(50)    NOT NULL," \
		  "LINENO         INT         NOT NULL," \
		  "COLUMNNO       INT         NOT NULL," \
		  "PRIMARY KEY(NAME, LINENO, COLUMNNO));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	  return;
	}else{
	  fprintf(stdout, "Table created successfully\n");
	}

	sql = "CREATE TABLE TYPES("  \
		  "NAME           CHAR(50)    NOT NULL," \
		  "LINENO         INT         NOT NULL," \
		  "COLUMNNO       INT         NOT NULL," \
		  "PCOFFSET       INT         NOT NULL," \
		  "TYPE           INT         NOT NULL," \
		  "UNIQUE (NAME, LINENO, COLUMNNO, PCOFFSET, TYPE));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	  return;
	}else{
	  fprintf(stdout, "Table created successfully\n");
	}

	sql = "CREATE TABLE SHAPES("  \
		  "NAME           CHAR(50)    NOT NULL," \
		  "LINENO         INT         NOT NULL," \
		  "COLUMNNO       INT         NOT NULL," \
		  "PCOFFSET       INT         NOT NULL," \
		  "TYPE           INT         NOT NULL," \
		  "OPCODE         INT         NOT NULL," \
		  "PRIMARY KEY(NAME, LINENO, COLUMNNO));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
	  fprintf(stderr, "SQL error: %s\n", zErrMsg);
	  sqlite3_free(zErrMsg);
	  return;
	}else{
	  fprintf(stdout, "Table created successfully\n");
	}

  }

  void
  js::JSMonitor::updateBytecodeType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type) {
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	sprintf(buff,"INSERT INTO TYPES (NAME, LINENO, COLUMNNO, PCOFFSET, TYPE) VALUES ('%s', '%d', '%d', '%d', '%d')", fileName,lineNo, column, pc, type);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		  //fprintf(stderr, "SQL error: %s\n", zErrMsg);
		  sqlite3_free(zErrMsg);
		  return;
		}else{
		  //fprintf(stdout, "Value Inserted successfully\n");
		}
  }

  js::JSMonitor::~JSMonitor() {
	sqlite3_close(monitorDb);
  }
