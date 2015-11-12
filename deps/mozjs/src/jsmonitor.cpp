#include "jsmonitor.h"
#include <signal.h>

js::JSMonitor::JSMonitor(long int id) {
	int rc = 0;
	char *zErrMsg = 0;

	rc = sqlite3_enable_shared_cache(1);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: Could not enable shared cache %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	index = id;
	Init(id);
}

int
js::JSMonitor::backupDb(){
	int rc;                     /* Function return code */
	sqlite3 *pFile;             /* Database connection opened on zFilename */
	sqlite3_backup *pBackup;    /* Backup handle used to copy data */
	sqlite3 *pDb;               /* Database to back up */
	char zFilename[100];      /* Name of file to back up to */

	sprintf(zFilename, "monitor%d.db", index);
	pDb = monitorDb;
	/* Open the database file identified by zFilename. */
	rc = sqlite3_open(zFilename, &pFile);
	if( rc==SQLITE_OK ){

		/* Open the sqlite3_backup object used to accomplish the transfer */
		pBackup = sqlite3_backup_init(pFile, "main", pDb, "main");
		if( pBackup ){

			/* Each iteration of this loop copies 5 database pages from database
			 ** pDb to the backup database. If the return value of backup_step()
			 ** indicates that there are still further pages to copy, sleep for
			 ** 250 ms before repeating. */
			do {
				rc = sqlite3_backup_step(pBackup, 5);
				if( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
					sqlite3_sleep(25);
				}
			} while( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED );

			/* Release resources allocated by backup_init(). */
			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pFile);
	}

	printf("Done backing up the database%s\n", zFilename);

	/* Close the database connection opened on database file zFilename
	 ** and return the result of this function. */
	(void)sqlite3_close(pFile);
	return rc;
}

void
js::JSMonitor::Init(int threadID)
{
	char *sql;
	int rc = 0;
	char *zErrMsg = 0;
	char filename[100];

	sprintf(filename, ":memory:");

	if (sqlite3_open(filename, &monitorDb) != 0) {
		printf("Monitor: Db init error.\n");
		return;
	}

	sql = "CREATE TABLE HOTFUNCS("  \
			"NAME           CHAR(100)    NOT NULL," \
			"TSCOUNT		INT			NOT NULL," \
			"PRIMARY KEY(NAME));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE TYPES("  \
			"NAME           CHAR(100)    NOT NULL," \
			"PCOFFSET		INT			NOT NULL,"
			"TYPE           INT         NOT NULL," \
			"UNIQUE (NAME, PCOFFSET, TYPE));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE SHAPESDEOPT("  \
			"NAME           CHAR(100)    NOT NULL," \
			"PCOFFSET		INT			NOT NULL," \
			"UNIQUE(NAME, PCOFFSET));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE OBJECTTYPES("  \
			"NAME           CHAR(100)    NOT NULL," \
			"INVOCCOUNT     INT         NOT NULL," \
			"UNIQUE (NAME));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE INSPECTORTYPES("  \
			"NAME           CHAR(100)    NOT NULL," \
			"PCOFFSET		INT			NOT NULL," \
			"TYPE           INT         NOT NULL," \
			"UNIQUE (NAME, PCOFFSET, TYPE));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE BAILOUTS("  \
			"NAME           CHAR(100)    NOT NULL," \
			"PCOFFSET		INT			NOT NULL," \
			"TYPE           INT         NOT NULL," \
			"UNIQUE (NAME, PCOFFSET, TYPE));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	time (&start);
}

sqlite3*
js::JSMonitor::getMonitorDb()
{
	time_t end;

	time (&end);
	double dif = difftime (end,start);
	if (dif > 5) {
		backupDb();
		time(&start);
	}
	return monitorDb;
}

void
js::JSMonitor::updateBytecodeType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type) {
	if (type == 8)
		return;

	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];

	sqlite3 *monitorDb = getMonitorDb();
	if (monitorDb == nullptr)
		return;

	sprintf(buff,"INSERT INTO TYPES (NAME, PCOFFSET, TYPE) VALUES ('%s:%d:%d', %d, %d);", fileName, lineNo, column, pc, type);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);
	if( rc != SQLITE_CONSTRAINT && rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}
}

void
js::JSMonitor::recordShapeDeopt(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc)
{
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	sqlite3 *monitorDb = getMonitorDb();

	if (monitorDb == nullptr)
		return;

	sprintf(buff,"INSERT INTO SHAPESDEOPT (NAME, PCOFFSET) VALUES ('%s:%d:%d', %d);", fileName, lineNo, column, pc);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);
	if( rc != SQLITE_CONSTRAINT && rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: SHAPESDEOPT %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}
}

void
js::JSMonitor::recordHotFunc(const char* fileName, long unsigned int lineNo, long unsigned int column, int tsCount)
{
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	sqlite3 *monitorDb = getMonitorDb();

	if (monitorDb == nullptr)
		return;

	//printf("Recording hot function\n");
	sprintf(buff,"INSERT INTO HOTFUNCS (NAME, TSCOUNT) VALUES ('%s:%d:%d', %d);", fileName, lineNo, column, tsCount);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);

	if( rc != SQLITE_CONSTRAINT && rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: HOTFUNCS %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}
}

void
js::JSMonitor::updateObjectTypeCount(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int invocCount)
{
	int rc = 0;
	char *zErrMsg = 0;
	char buff[1000];
	sqlite3 *monitorDb = getMonitorDb();

	if (monitorDb == nullptr)
		return;

	sprintf(buff,"INSERT OR REPLACE INTO OBJECTTYPES (NAME, LINENO, COLUMNNO, INVOCCOUNT) VALUES ('%s', %d, %d, "\
			"COALESCE((SELECT INVOCCOUNT FROM OBJECTTYPES WHERE NAME='%s' AND LINENO=%d AND COLUMNNO=%d AND INVOCCOUNT>%d), %d));",
			fileName, lineNo, column, fileName, lineNo, column, invocCount, invocCount);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);
	if( rc != SQLITE_CONSTRAINT && rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: OBJECTTYPES %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}
}
void
js::JSMonitor::setInspectorResultType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type)
{
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	sqlite3 *monitorDb = getMonitorDb();

	if (monitorDb == nullptr)
		return;
	sprintf(buff,"INSERT INTO INSPECTORTYPES (NAME, PCOFFSET, TYPE) VALUES ('%s:%d:%d', %d, %d);", fileName, lineNo, column, pc, type);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);
	if( rc != SQLITE_CONSTRAINT && rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}
}

void
js::JSMonitor::recordBailout(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int bailoutkind)
{
	if (bailoutkind == 1)
		return;

	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	sqlite3 *monitorDb = getMonitorDb();

	if (monitorDb == nullptr)
		return;
	sprintf(buff,"INSERT INTO BAILOUTS (NAME, PCOFFSET, TYPE) VALUES ('%s:%d:%d', %d, %d);", fileName, lineNo, column, pc, bailoutkind);
	rc = sqlite3_exec(monitorDb, buff, callback, 0, &zErrMsg);
	if( rc != SQLITE_CONSTRAINT && rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}

}

void
js::JSMonitor::setCompareType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type)
{
	setInspectorResultType(fileName, lineNo, column, pc, type);
}
void
js::JSMonitor::setNonNativeGetElem(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value)
{
	setInspectorResultType(fileName, lineNo, column, pc, (int)value);
}
void
js::JSMonitor::setSeenNonString(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value)
{
	setInspectorResultType(fileName, lineNo, column, pc, int(value));
}
void
js::JSMonitor::setSeenDouble(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value)
{
	setInspectorResultType(fileName, lineNo, column, pc, int(value));
}
void
js::JSMonitor::setBinaryType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type)
{
	setInspectorResultType(fileName, lineNo, column, pc, type);
}
void
js::JSMonitor::setSeenAccessedGetter(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value)
{
	setInspectorResultType(fileName, lineNo, column, pc, int(value));
}
js::JSMonitor::~JSMonitor() {
	sqlite3_close(monitorDb);
}
