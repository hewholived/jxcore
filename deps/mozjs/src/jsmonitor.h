#include <sqlite3.h>

#ifndef JSMONITOR_H_
#define JSMONITOR_H_

namespace js {
  class JSMonitor {
    
    static int callback(void *NotUsed, int argc, char **argv, char **azColName){
      int i;
      for(i = 0; i < argc; i++){
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      }
      printf("\n");
      return 0;
    }

    public:
      sqlite3 *monitorDb;
      
      JSMonitor() {
        char *sql;
        int rc = 0;
        char *zErrMsg = 0;

        if (sqlite3_open("monitor.db::memory:", &monitorDb) != 0) {
          printf("Monitor: Db init error.\n");
          return;
        }

        sql = "CREATE TABLE HOTFUNCS("  \
              "NAME           CHAR(50)    NOT NULL," \
              "LINENO         INT         NOT NULL," \
              "COLUMNNO       INT         NOT NULL," \
              "PRIMARY KEY(NAME, LINENO, COLUMNNO));";

        rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
          return;
        }else{
          fprintf(stdout, "Table created successfully\n");
        }

        sql = "CREATE TABLE TYPES("  \
              "NAME           CHAR(50)    NOT NULL," \
              "LINENO         INT         NOT NULL," \
              "COLUMNNO       INT         NOT NULL," \
              "PCOFFSET       INT         NOT NULL," \
              "TYPE           INT         NOT NULL," \
              "OPCODE         INT         NOT NULL," \
              "PRIMARY KEY(NAME, LINENO, COLUMNNO));";

        rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
          return;
        }else{
          fprintf(stdout, "Table created successfully\n");
        }

        sql = "CREATE TABLE SHAPES("  \
              "NAME           CHAR(50)    NOT NULL," \
              "LINENO         INT         NOT NULL," \
              "COLUMNNO       INT         NOT NULL," \
              "PCOFFSET       INT         NOT NULL," \
              "TYPE           INT         NOT NULL," \
              "OPCODE         INT         NOT NULL," \
              "PRIMARY KEY(NAME, LINENO, COLUMNNO));";

        rc = sqlite3_exec(monitorDb, sql, callback, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
          fprintf(stderr, "SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
          return;
        }else{
          fprintf(stdout, "Table created successfully\n");
        }

      }

      JSMonitor() {
        sqlite3_close(monitorDb);
      }
  }
}
