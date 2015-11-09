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
	int getHotnessThreshold(const char* fileName, long unsigned int lineNo, long unsigned int column, int *value);
};
}



#endif /* SHELL_ORACLE_H_ */
