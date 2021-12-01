////
//// Created by fadi on 2021-11-09.
////
//#include <dc_posix/dc_fcntl.h>
//#include <dc_posix/dc_ndbm.h>
//#include <dc_posix/dc_posix_env.h>
//#include <dc_posix/dc_unistd.h>
//#include <stdio.h>
//#include <fcntl.h>
//#include <string.h>
//#include <malloc.h>
//#include <time.h>
//#include <dc_posix/dc_stdlib.h>
//
//
//#define NAME       "Arturo Crespo"
//#define PHONE_NO   "723-9273"
//#define DB_NAME    "phones"
//
//
//char* get_data_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db, int fd);
//void delete_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db);
//
//void Read_Write_dbm(struct dc_posix_env *env, struct dc_error *err, char *type, char *data, int fd, char *mjr, char *mnr, char *lt, char *ln) {
//    DBM *db;
//
//    // TO DELETE LATER (these 2 lines)
//    datum name = {NAME, sizeof(NAME)};
//    datum datamm = {data, sizeof(data)};
//
//
//    //-----------------
//    char * newKey = (char *)malloc(strlen(mjr)  + strlen(mnr) + 1);
//
//    char delim[] = "-";
//    char *delimPtr = delim;
//    strcpy(newKey, mjr);
//    strcat(newKey, delimPtr);
//    strcat(newKey, mnr);
//    printf("TEST NEW KEY = %s\n", newKey);
//
//    char * newVal = (char *)malloc(strlen(lt)  + strlen(ln) + 1);
//
//    strcpy(newVal, lt);
//    strcat(newVal, delimPtr);
//    strcat(newVal, ln);
//    printf("TEST NEW VALUE = %s\n", newVal);
//
//    datum dataVal = {newVal, sizeof(newVal)};
//    datum dataKey = {newKey, sizeof(newKey)};
//    // -----------------------
//
//
//    // Open the database and store the record
//    db = dc_dbm_open(env, err, DB_NAME, DC_O_RDWR | DC_O_CREAT, 0600);
//    if (strcmp(type, "POST") == 0) {
//        dc_dbm_store(env, err, db, dataKey, dataVal, DBM_REPLACE);
//    }
//
//        // Retrieve the record
//    else {
//        dc_dbm_store(env, err, db, dataKey, dataVal, DBM_REPLACE);
//
////        get_data_from_database(env, err, db);
////        delete_from_database(env, err, db);
//  //      char *res = get_data_from_database(env, err, db, fd);
//  //      printf("FINAL TEST = %s", res);
//
//
//        datum key = dc_dbm_firstkey(env, err, db);
//        datum get_maj = dc_dbm_fetch(env, err, db, key);
//        printf("got data = %s, %s\n", key.dptr, get_maj.dptr);
//
//        printf("hi there");
//        // TO DELETE LATER
//        /////////////////////////////////////////////////////////////////////////////
//        size_t size = strlen((char *) name.dptr) + strlen((char *) datamm.dptr) + 1;// +1 for the null-terminator
//        char *result = dc_malloc(env, err, size);
//        // in real code you would check for errors in malloc here
//        strcpy(result, (char *) name.dptr);
//        strcat(result, (char *) datamm.dptr);
//        dc_write(env, err, fd, result, size);
//        /////////////////////////////////////////////////////////////////////////////////
//
////        dc_free(env, result, size);
//    }
//
//    // Close the database
//    dc_dbm_close(env, err, db);
//
//}
//
//char* get_data_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db, int fd) {
//    datum key;
//    datum get_maj;
//    printf("\nget all data from database:\n");
//
//    char delim[] = "|";
//    char *delimPtr = delim;
//    size_t size = 1;
//
////    for(key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {
////
////
////        get_maj = dc_dbm_fetch(env, err, db, key);
////    //    printf("getting data: %s, %s\n", key.dptr, get_maj.dptr);
////
////        size += strlen((char *)delimPtr) + strlen((char *) key.dptr) + strlen((char *) get_maj.dptr);
////
////    }
//
//    size = 1024;
//    printf("size = %zu", size);
//
//    char *result = dc_malloc(env, err, size);
//    strcpy(result, delimPtr);
//    for(key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {
//
//        get_maj = dc_dbm_fetch(env, err, db, key);
//        printf("getting data: %s, %s\n", key.dptr, get_maj.dptr);
//
////        strcat(result, (char *) key.dptr);
////        strcat(result, (char *) get_maj.dptr);
////        strcat(result, (char *) delimPtr);
//    //    printf("\nresss = %s", result);
//    }
//
//    printf("my testtt");
// //   printf("hellll = %s", result);
//
////    dc_write(env, err, fd, result, size);
////
////
////    dc_free(env, result, size);
//
//    return result;
//}
//
//void delete_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db) {
//    datum key;
//    printf("\ndelete from database called\n");
//    for(key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_firstkey(env, err, db)) {
//        printf("deleting key: %s\n", key.dptr);
//        dc_dbm_delete(env, err, db, key);
//    }
//}







//
// Created by fadi on 2021-11-09.
//
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_ndbm.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <dc_posix/dc_stdlib.h>


#define NAME       "Arturo Crespo"
#define PHONE_NO   "723-9273"
#define DB_NAME    "phones"


void get_data_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db, int fd);
void delete_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db);

void Read_Write_dbm(struct dc_posix_env *env, struct dc_error *err, char *type, char *data, int fd, char *mjr, char *mnr, char *lt, char *ln) {
    DBM *db;

    // TO DELETE LATER (these 2 lines)
    datum name = {NAME, sizeof(NAME)};
    datum datamm = {data, sizeof(data)};


    //-----------------
    char * newKey = (char *)malloc(strlen(mjr)  + strlen(mnr) + 1);

    char delim[] = "-";
    char *delimPtr = delim;
    strcpy(newKey, mjr);
    strcat(newKey, delimPtr);
    strcat(newKey, mnr);
    printf("TEST NEW KEY = %s\n", newKey);

    char * newVal = (char *)malloc(strlen(lt)  + strlen(ln) + 1);

    strcpy(newVal, lt);
    strcat(newVal, delimPtr);
    strcat(newVal, ln);
    printf("TEST NEW VALUE = %s\n", newVal);

    datum dataVal = {newVal, sizeof(newVal)};
    datum dataKey = {newKey, sizeof(newKey)};
    // -----------------------


    // Open the database and store the record
    db = dc_dbm_open(env, err, DB_NAME, DC_O_RDWR | DC_O_CREAT, 0600);
    if (strcmp(type, "POST") == 0) {
        dc_dbm_store(env, err, db, dataKey, dataVal, DBM_REPLACE);
    }

        // Retrieve the record
    else {
        dc_dbm_store(env, err, db, dataKey, dataVal, DBM_REPLACE);

//        get_data_from_database(env, err, db);
  //      delete_from_database(env, err, db);
        get_data_from_database(env, err, db, fd);

    }

    // Close the database
    dc_dbm_close(env, err, db);

}

void get_data_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db, int fd) {
    char delim2[] = "|";
    char *delimPtr2 = delim2;
    char delimComma[] = ",";
    char *delimCommaPtr = delimComma;
    datum key;
    datum get_maj;
    size_t size = 1;

    for(key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {

        get_maj = dc_dbm_fetch(env, err, db, key);
        size += strlen((char *) key.dptr) + strlen((char *) delimCommaPtr) + strlen((char *) get_maj.dptr) + strlen((char *) delimPtr2);// +1 for the null-terminator
    }
    char *result = dc_malloc(env, err, size);
    strcpy(result, (char *) delimPtr2);

    for(key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {

        get_maj = dc_dbm_fetch(env, err, db, key);
        printf("data: %s, %s\n", key.dptr, get_maj.dptr);

        // in real code you would check for errors in malloc here
        strcat(result, (char *) key.dptr);
        strcat(result, (char *) delimCommaPtr);
        strcat(result, (char *) get_maj.dptr);
        strcat(result, (char *) delimPtr2);
    }

    dc_write(env, err, fd, result, size);

    dc_free(env, result, size);
}

void delete_from_database(struct dc_posix_env *env, struct dc_error *err, DBM *db) {
    datum key;
    printf("\ndelete from database called\n");
    for(key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_firstkey(env, err, db)) {
        printf("deleting key: %s\n", key.dptr);
        dc_dbm_delete(env, err, db, key);
    }
}