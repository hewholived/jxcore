#include "jsmonitor.h"

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

void
js::JSMonitor::Init(int threadID)
{
	char *sql;
	int rc = 0;
	char *zErrMsg = 0;
	char filename[100];

	sprintf(filename, "monitor%d.db", threadID);

	if (sqlite3_open(filename, &monitorDb) != 0) {
		printf("Monitor: Db init error.\n");
		return;
	}

	sql = "CREATE TABLE HOTFUNCS("  \
			"NAME           CHAR(100)    NOT NULL," \
			"LINENO         INT         NOT NULL," \
			"COLUMNNO       INT         NOT NULL," \
			"TSCOUNT		INT			NOT NULL," \
			"PRIMARY KEY(NAME, LINENO, COLUMNNO));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE TYPES("  \
			"NAME           CHAR(100)    NOT NULL," \
			"LINENO         INT         NOT NULL," \
			"COLUMNNO       INT         NOT NULL," \
			"PCOFFSET       INT         NOT NULL," \
			"TYPE           INT         NOT NULL," \
			"UNIQUE (NAME, LINENO, COLUMNNO, PCOFFSET, TYPE));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE SHAPESDEOPT("  \
			"NAME           CHAR(100)    NOT NULL," \
			"LINENO         INT         NOT NULL," \
			"COLUMNNO       INT         NOT NULL," \
			"PCOFFSET       INT         NOT NULL," \
			"UNIQUE(NAME, LINENO, COLUMNNO, PCOFFSET));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE OBJECTTYPES("  \
			"NAME           CHAR(100)    NOT NULL," \
			"LINENO         INT         NOT NULL," \
			"COLUMNNO       INT         NOT NULL," \
			"INVOCCOUNT     INT         NOT NULL," \
			"UNIQUE (NAME, LINENO, COLUMNNO));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sql = "CREATE TABLE INSPECTORTYPES("  \
			"NAME           CHAR(100)    NOT NULL," \
			"LINENO         INT         NOT NULL," \
			"COLUMNNO       INT         NOT NULL," \
			"PCOFFSET       INT         NOT NULL," \
			"TYPE           INT         NOT NULL," \
			"UNIQUE (NAME, LINENO, COLUMNNO, PCOFFSET, TYPE));";

	rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
}

sqlite3*
js::JSMonitor::getMonitorDb()
{
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

	sprintf(buff,"INSERT INTO TYPES (NAME, LINENO, COLUMNNO, PCOFFSET, TYPE) VALUES ('%s', %d, %d, %d, %d)", fileName,lineNo, column, pc, type);
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

	sprintf(buff,"INSERT INTO SHAPESDEOPT (NAME, LINENO, COLUMNNO, PCOFFSET) VALUES ('%s', %d, %d, %d)", fileName,lineNo, column, pc);
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

	sprintf(buff,"INSERT INTO HOTFUNCS (NAME, LINENO, COLUMNNO, TSCOUNT) VALUES ('%s', %d, %d, %d)", fileName,lineNo, column, tsCount);
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
			"COALESCE((SELECT INVOCCOUNT FROM OBJECTTYPES WHERE NAME='%s' AND LINENO=%d AND COLUMNNO=%d AND INVOCCOUNT>%d), %d))",
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
	sprintf(buff,"INSERT INTO INSPECTORTYPES (NAME, LINENO, COLUMNNO, PCOFFSET, TYPE) VALUES ('%s', %d, %d, %d, %d)", fileName,lineNo, column, pc, type);
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
