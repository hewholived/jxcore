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

	db = nullptr;
	index = id;
	Init(id);
}

void
js::Oracle::Init(int id)
{
	char filename[100];

	sprintf(filename, "monitor%d.db", id);

	if (sqlite3_open_v2(filename, &db, SQLITE_OPEN_READONLY, nullptr) != 0) {
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

	//printf("in callback %p:", retval);
	if (argc == 0 || argc > 1) {
		retval[0] = -1;
	} else {
		retval[0] = argv[0] ? atoi(argv[0]) : -1;
		//printf("%d\n", atoi(argv[0]));
	}

	return 0;
}

int
js::Oracle::getHotnessThreshold(const char* fileName, long unsigned int lineNo, long unsigned int column)
{
	if (db == nullptr)
		return -1;

	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	int* value = (int *)malloc(1);
	value[0] = -1;

	sprintf(buff,"SELECT TSCOUNT FROM HOTFUNCS WHERE NAME='%s:%d:%d';", fileName, lineNo, column);
	//printf("Query:%s\n", buff);
	rc = sqlite3_exec(db, buff, hotFunCallback, (void *)value, &zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}

	return value[0];
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
	if (db == nullptr)
		return;

	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];
	unsigned long int *data = (unsigned long int *)malloc(2);
	data[0] = (unsigned long int)context;
	data[1] = (unsigned long int)script;

	sprintf(buff,"SELECT PCOFFSET,TYPE FROM TYPES WHERE NAME='%s:%d:%d';", script->filename(), script->lineno(), script->column());
	rc = sqlite3_exec(db, buff, typesCallback, (void *)data, &zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return;
	}

}

static int
inspectorBoolCallback(void *passPtr, int argc, char **argv, char **azColName)
{
	bool *retVal = (bool *) passPtr;

	if (argv[0])
		*retVal = (*retVal) || (bool)atoi(argv[0]);

	return 0;
}

bool
js::Oracle::inspectorBoolData(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc)
{
	if (db == nullptr)
		return false;

	bool *retVal = (bool*)malloc(1);
	*retVal = false;
	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];


	sprintf(buff,"SELECT TYPE FROM INSPECTORTYPES WHERE NAME='%s:%d:%d' AND PCOFFSET=%d;", fileName, lineNo, column, pc);
	//printf("Query:%s\n", buff);
	rc = sqlite3_exec(db, buff, inspectorBoolCallback, (void *)retVal, &zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return false;
	}

	return *retVal;
}

static js::jit::MIRType
moveUpTheLattice(js::jit::MIRType type1, js::jit::MIRType type2)
{
	if (type1 == js::jit::MIRType_None || type2 == js::jit::MIRType_None)
		return js::jit::MIRType_None;

	if (type1 == js::jit::MIRType_Int32) {
		if (type2 == js::jit::MIRType_Int32)
			return js::jit::MIRType_Int32;
		else if (type2 == js::jit::MIRType_Double)
			return js::jit::MIRType_Double;
		else
			return js::jit::MIRType_None;
	}
	if (type1 == js::jit::MIRType_Double) {
		if (type2 == js::jit::MIRType_Int32)
			return js::jit::MIRType_Double;
		else if (type2 == js::jit::MIRType_Double)
			return js::jit::MIRType_Double;
		else
			return js::jit::MIRType_None;
	}
	if (type1 == js::jit::MIRType_String) {
		if (type2 == js::jit::MIRType_String)
			return js::jit::MIRType_String;
		else
			return js::jit::MIRType_None;
	}
	return js::jit::MIRType_None;
}

static int
inspectorTypeCallback(void *passPtr, int argc, char **argv, char **azColName)
{
	js::jit::MIRType *retVal = (js::jit::MIRType *) passPtr;

	if (argv[0])
		*retVal = moveUpTheLattice(*retVal, (js::jit::MIRType)atoi(argv[0]));

	return 0;
}

js::jit::MIRType
js::Oracle::inspectorTypeData(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, js::jit::MIRType curType)
{
	if (db == nullptr)
		return curType;

	js::jit::MIRType *retVal = (js::jit::MIRType *) malloc(1);
	*retVal = curType;

	int rc = 0;
	char *zErrMsg = 0;
	char buff[500];

	sprintf(buff,"SELECT TYPE FROM INSPECTORTYPES WHERE NAME='%s:%d:%d' AND PCOFFSET=%d;", fileName, lineNo, column, pc);
	//printf("Query:%s\n", buff);
	rc = sqlite3_exec(db, buff, inspectorTypeCallback, (void *)retVal, &zErrMsg);
	if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: types %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return curType;
	}

	return *retVal;
}
