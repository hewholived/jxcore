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
      
      JSMonitor();
      void updateBytecodeType(const char* fileName, long unsigned int lineNo, long unsigned int column, long unsigned int pc, int type);
      ~JSMonitor();
  };
}

#endif
