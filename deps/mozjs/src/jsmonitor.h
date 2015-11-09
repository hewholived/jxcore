#include <sqlite3.h>
#include <stdio.h>
#include <map>
#include "jstypes.h"

#ifndef JSMONITOR_H_
#define JSMONITOR_H_

namespace js {
  class JSMonitor {

	  static int callback(void *NotUsed, int argc, char **argv, char **azColName){
		  int i;
		  for(i = 0; i < argc; i++){
		  }
		  return 0;
	  }
	  sqlite3 *monitorDb;
	  int index;
  public:
	  JSMonitor(long int id);
	  void updateBytecodeType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
	  void recordShapeDeopt(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc);
	  void recordHotFunc(const char* fileName, long unsigned int lineNo, long unsigned int column, int tsCount);
	  void updateObjectTypeCount(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int invocCount);
	  void setInspectorResultType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
	  void setCompareType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
	  void setNonNativeGetElem(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
	  void setSeenNonString(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
	  void setSeenDouble(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
	  void setBinaryType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
	  void setSeenAccessedGetter(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
	  sqlite3 *getMonitorDb();
	  void Init(int index);

	  ~JSMonitor();
  };
}

#endif
