/*
 * Oracle.h
 *
 *  Created on: Jan 23, 2015
 *      Author: ******
 */

#include <list>
#include <sqlite3.h>

#ifndef SHELL_ORACLE_H_
#define SHELL_ORACLE_H_

namespace js {

class OTypeInfo {
  public:
	const char* name;
	uint32_t lineNo;
	uint32_t column;
	uint32_t pc;
	uint32_t* types;
	uint32_t noTypes;

  OTypeInfo(const char* name, uint32_t lineNo, uint32_t column, uint32_t pc, 
            uint32_t* types, uint32_t noTypes) {
    this->name = name;
    this->lineNo = lineNo;
    this->column = column;
    this->pc = pc;
    this->types = types;
    this->noTypes = noTypes;
  }
};

class OFreqBailoutInfo {
public:
	const char* name;
	uint32_t lineNo;
	uint32_t column;

	OFreqBailoutInfo(const char* name, uint32_t lineNo, uint32_t column) {
		this->name = name;
		this->lineNo = lineNo;
		this->column = column;
	}
};

class OHotFunctionInfo {
public:
	const char* name;
	uint32_t lineNo;
	uint32_t column;

	OHotFunctionInfo(const char* name, uint32_t lineNo, uint32_t column) {
		this->name = name;
		this->lineNo = lineNo;
		this->column = column;
	}
};


class OShapeGuardBailoutInfo {
public:
	const char* name;
	uint32_t lineNo;
	uint32_t column;
	uint32_t pc;

	OShapeGuardBailoutInfo(const char* name, uint32_t lineNo, uint32_t column, 
                           uint32_t pc) {
		this->name = name;
		this->lineNo = lineNo;
		this->column = column;
		this->pc = pc;
	}
};

class Oracle {
  public:
	OTypeInfo** typeInfos;
	OFreqBailoutInfo** freqBailoutInfos;
	OShapeGuardBailoutInfo** shapeGuardInfos;
	//OHotFunctionInfo** hotFunctionInfos;
	std::list<OHotFunctionInfo*> hotFunctionInfos;
	uint32_t noTypeInfos;
	uint32_t noBailoutInfos;
	uint32_t noShapeGuardInfos;
	uint32_t noHotFunctions;
	uint32_t lastIndex;
    sqlite3 *db;

    Oracle() {
       noTypeInfos = lastIndex = noBailoutInfos = noShapeGuardInfos = noHotFunctions = 0;
       freqBailoutInfos = nullptr;
       typeInfos = nullptr;
       shapeGuardInfos = nullptr;
       hotFunctionInfos.clear();
    }

    ~Oracle() {
	   noTypeInfos = lastIndex = noBailoutInfos = noShapeGuardInfos = 0;
	   if (freqBailoutInfos != nullptr) {
		   //Maybe I should delete all the indifidual info objects. I dont care right now.
		   free(freqBailoutInfos);
	   }
	   if (shapeGuardInfos != nullptr) free(shapeGuardInfos);
	   if (typeInfos != nullptr) free(typeInfos);
	   //if (hotFunctionInfos != nullptr) free(hotFunctionInfos);
	}

    OTypeInfo* 
    getTypeInfo(const char* name, uint32_t lineNo, uint32_t column, 
                uint32_t pc){
    	for (uint32_t i = (lastIndex + 1) % noTypeInfos; 
                      i != lastIndex; 
                      i = (i + 1) % noTypeInfos) {
    		if(!strcmp(typeInfos[i]->name, name) && 
                lineNo == typeInfos[i]->lineNo && 
                column == typeInfos[i]->column && 
                pc == typeInfos[i]->pc) {
    			lastIndex = i;
    			return typeInfos[i];
    		}
    	}
    	return NULL;
    }

    OTypeInfo** 
    getTypeInfos(const char* name, uint32_t lineNo, uint32_t column, 
                 uint32_t *nTypeInfos) {
    	OTypeInfo** retTypeInfos = nullptr;
    	*nTypeInfos = 0;
    	for (uint32_t i = 0; i < noTypeInfos; i++) {
    	    if(!strcmp(typeInfos[i]->name, name) && 
                lineNo == typeInfos[i]->lineNo && 
                column == typeInfos[i]->column) {
    	    	if (retTypeInfos == nullptr) {
    	    		 retTypeInfos = (OTypeInfo**)malloc(sizeof(OTypeInfo *) * noTypeInfos);
    	    	}
    	    	retTypeInfos[(*nTypeInfos)++] = typeInfos[i];
    	    }
    	}

    	return retTypeInfos;
    }

    bool 
    isFreqBailedOut(const char* name, uint32_t lineNo, uint32_t column) {
    	for (uint32_t i = 0; i < noBailoutInfos; i++) {
			if(!strcmp(freqBailoutInfos[i]->name, name) && 
                lineNo == freqBailoutInfos[i]->lineNo && 
                column == freqBailoutInfos[i]->column) {
				return true;
			}
		}
    	return false;
    }

    bool 
    isHotFunction(const char* name, uint32_t lineNo, uint32_t column) {

    	if (hotFunctionInfos.empty())
    		return false;

		for(std::list<OHotFunctionInfo*>::iterator list_iter = hotFunctionInfos.begin();
		    list_iter != hotFunctionInfos.end(); list_iter++)
		{
			if(!strcmp((*list_iter)->name, name) && 
                lineNo == (*list_iter)->lineNo && 
                column == (*list_iter)->column) {
				hotFunctionInfos.remove(*list_iter);
				return true;
			}
		}

		return false;
	}

    bool hasShapeGuardFailed(const char* name, uint32_t lineNo, uint32_t column, uint32_t pc) {
		for (uint32_t i = 0; i < noShapeGuardInfos; i++) {
			//printf("Trying to find %s:%d:%d comparing with %s:%d:%d\n", name, lineNo, pc, typeInfos[i]->name, typeInfos[i]->lineNo, typeInfos[i]->pc);
			if(!strcmp(shapeGuardInfos[i]->name, name) && lineNo == shapeGuardInfos[i]->lineNo && column == shapeGuardInfos[i]->column && pc == shapeGuardInfos[i]->pc) {
				return true;
			}
		}
		return false;
	}

    static int hotFunCallback(void *oraclePtr, int argc, char **argv, char **azColName){
    	//Oracle *oracle = (Oracle *) oraclePtr;
    	int i;
    	for(i = 0; i < argc; i++){
    		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    	}
    	printf("\n");
    	return 0;
    }
    void init(const char* fname) {
    	//char* jsFile = (char *)malloc(sizeof(char) * 2051);
    	//char types[100];
    	//int lineNo, pc, offset, shapeID, isFixedSlot;
    	//int lineNo, column, pc;

    	const char *sql;
    	int rc = 0;
    	char *zErrMsg = 0;

    	if (sqlite3_open(fname, &db) != 0) {
    		//if (sqlite3_open("monitor.db", &monitorDb) != 0) {
    		printf("Monitor: Db init error.\n");
    		return;
    	}

    	//Get the hot functions
    	sql = "SELECT * HOTFUNCS;";

    	rc = sqlite3_exec(db, sql, hotFunCallback, this, &zErrMsg);
    	if( rc != SQLITE_OK ){
    		fprintf(stderr, "SQL error: Init %s\n", zErrMsg);
    		sqlite3_free(zErrMsg);
    	}else{
    		//fprintf(stdout, "Table created successfully\n");
    	}

//    	// Scan the no of bailout infos
//    	if (fscanf(fstream, "%d\n", &noBailoutInfos) == EOF)
//    		return;
//
//    	freqBailoutInfos = (OFreqBailoutInfo**)malloc(sizeof(OFreqBailoutInfo*) * noBailoutInfos);
//
//    	for (uint32_t i = 0; i < noBailoutInfos && fscanf(fstream, "%[^;];%d;%d\n", jsFile, &lineNo, &column) != EOF; i++)
//    	{
//    		OFreqBailoutInfo* bailoutInfo = new OFreqBailoutInfo(jsFile, lineNo, column);
//    		freqBailoutInfos[i] = bailoutInfo;
//    	}
//
//    	// Scan the no of bailout infos
//		if (fscanf(fstream, "%d\n", &noShapeGuardInfos) == EOF)
//			return;
//
//		shapeGuardInfos = (OShapeGuardBailoutInfo**)malloc(sizeof(OShapeGuardBailoutInfo*) * noShapeGuardInfos);
//
//		for (uint32_t i = 0; i < noShapeGuardInfos && fscanf(fstream, "%[^;];%d;%d;%d\n", jsFile, &lineNo, &column, &pc) != EOF; i++)
//		{
//			OShapeGuardBailoutInfo* shapeGuardInfo = new OShapeGuardBailoutInfo(jsFile, lineNo, column, pc);
//			shapeGuardInfos[i] = shapeGuardInfo;
//		}
//
//
//    	// Scan the no of type infos
//    	if (fscanf(fstream, "%d\n", &noTypeInfos) == EOF)
//    	    return;
//
//    	typeInfos = (OTypeInfo**)malloc(sizeof(OTypeInfo*) * noTypeInfos);
//
//    	for (uint32_t i = 0; i < noTypeInfos && fscanf(fstream, "%[^;];%d;%d;%d; %[^\t\n]\n", jsFile, &lineNo, &column, &pc, types) != EOF; i++) {
//    		/*
//    		 * tokenize the types http://stackoverflow.com/questions/1483206/how-to-tokenize-string-to-array-of-int-in-c
//    		 * Create a OTypeInfo object and add it to typeInfos
//    		 *
//    		 */
//    		char seps[] = " ";
//    		char* token;
//    		uint32_t type;
//    		uint32_t* input = (uint32_t *)malloc(sizeof(uint32_t) * 16);
//    		uint32_t j = 0;
//
//    		token = strtok (types, seps);
//    		while (token != NULL)
//    		{
//    			sscanf (token, "%d", &type);
//    			input[j++] = type;
//    			token = strtok (NULL, seps);
//    		}
//
//    		OTypeInfo *typeInfo = new OTypeInfo(jsFile, lineNo, column, pc, input, j);
//    		//printf("%s:%d:%d:%d:%d:%d:%s:%d\n", jsFile, lineNo, pc, isFixedSlot, offset, shapeID, types, i);
//    		typeInfos[i] = typeInfo;
//    	}
    }
};
}



#endif /* SHELL_ORACLE_H_ */
