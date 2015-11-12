/*
 * Oracle.h
 *
 *  Created on: Jan 23, 2015
 *      Author: ******
 */

#include <list>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>

//#include "jscntxt.h"
#include "jsscript.h"
#include "jit/IonTypes.h"
//#include "jsinfer.h"
//#include "jstypes.h"


#ifndef SHELL_ORACLE_H_
#define SHELL_ORACLE_H_

namespace js {

class Oracle {
	int index;
	sqlite3* db;
public:
	Oracle(int id);
	~Oracle();
	void Init(int id);
	int getHotnessThreshold(const char* fileName, long unsigned int lineNo, long unsigned int column);
	void getTypeInfos(JSContext *context, JSScript *script);
	bool inspectorBoolData(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc);
	js::jit::MIRType inspectorTypeData(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, js::jit::MIRType curType);
};
}



#endif /* SHELL_ORACLE_H_ */
