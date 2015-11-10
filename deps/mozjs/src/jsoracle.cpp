#include "jsoracle.h"
#include "jsinfer.h"
#include "jsinferinlines.h"


js::Oracle::Oracle(int id)
{
	index = id;
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
js::Oracle::Init(int id)
{
	char filename[100];

	sprintf(filename, "monitor%d.db", id);

	if (sqlite3_open(filename, &db) != 0) {
		printf("Monitor: Db init error.\n");
		return;
	}
}

js::Oracle::~Oracle() {
	sqlite3_close(db);
}

static int
hotFunCallback(void *passPtr, int argc, char **argv, char **azColName)
{
	int *retval = (int *) passPtr;

	if (argc == 0 || argc > 1)
		*retval = -1;
	else
		*retval = argv[0] ? -1 : atoi(argv[0]);

	return 0;
}

int
js::Oracle::getHotnessThreshold(const char* fileName, long unsigned int lineNo, long unsigned int column, int *value)
{
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];

	sprintf(buff,"SELECT TSCOUNT FROM HOTFUNCS WHERE NAME='%s:%d:%d", fileName, lineNo, column);
	rc = sqlite3_exec(db, buff, hotFunCallback, (void *)value, &zErrMsg);
	if( rc != SQLITE_OK ){
		//fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return 0;
}

static int
typesCallback(void *passPtr, int argc, char **argv, char **azColName)
{
	unsigned long int *data = (unsigned long int *) passPtr;
	JSContext *context = (JSContext *)data[0];
	JSScript *script = (JSScript *)data[1];

	if (argc != 2)
		return -1;

	js::types::TypeScript::Monitor(context, script, script->offsetToPC(atoi(argv[0])), (js::types::Type)atoi(argv[1]));
	//printf("Updating types %s:%d:%d:%d:%d\n", script->filename(), script->lineno(), script->column(), atoi(argv[0]), atoi(argv[1]));

	return 0;
}

void
js::Oracle::getTypeInfos(JSContext *context, JSScript *script)
{
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	unsigned long int *data = (unsigned long int *)malloc(2);
	data[0] = (unsigned long int)context;
	data[1] = (unsigned long int)script;

	sprintf(buff,"SELECT PCOFFSET,TYPE FROM TYPES WHERE NAME='%s:%d:%d'", script->filename(), script->lineno(), script->column());
	rc = sqlite3_exec(db, buff, typesCallback, (void *)data, &zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}

}

