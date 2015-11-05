#include <sqlite3.h>
#include <stdio.h>
#include "jstypes.h"

#ifndef JSMONITOR_H_
#define JSMONITOR_H_

namespace js {
  class JSMonitor {
    
    static int callback(void *NotUsed, int argc, char **argv, char **azColName){
      int i;
      for(i = 0; i < argc; i++){
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      }
      //printf("\n");
      return 0;
    }

    public:
      sqlite3 *monitorDb;
//      sqlite3 *hotDb;
//      sqlite3 *objTypesDb;
      
      JSMonitor();
      void updateBytecodeType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
      void recordShapeDeopt(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc);
      void recordHotFunc(const char* fileName, long unsigned int lineNo, long unsigned int column, uint32_t tsCount);
      void updateObjectTypeCount(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int invocCount);
      //void shapeUpdateCount(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int invocCount);
      void setInspectorResultType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
      void setCompareType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
      void setNonNativeGetElem(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
      void setSeenNonString(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
      void setSeenDouble(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);
      void setBinaryType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
      void setSeenAccessedGetter(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, bool value);

      ~JSMonitor();
  };
}

#endif
