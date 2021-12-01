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
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/dc_string.h>

#define NAME       "Arturo Crespo"
#define PHONE_NO   "723-9273"
#define DB_NAME    "phones"


//void Read_Write_dbm(struct dc_posix_env *env, struct dc_error *err, char *type, char *data, int fd) {
//    DBM *db;
//    datum name = {NAME, sizeof(NAME)};
//    datum put_phone_no = {PHONE_NO, sizeof(PHONE_NO)};
//    datum datamm = {data, sizeof(data)};
//    printf(" size of data: %zu\n", sizeof(data));
//    printf("Data %s\n", data);
//    datum get_phone_no;
//
//    // Open the database and store the record
//    db = dc_dbm_open(env, err, DB_NAME, DC_O_RDWR | DC_O_CREAT, 0600);
//    if (strcmp(type, "POST") == 0) {
//        dc_dbm_store(env, err, db, name, datamm, DBM_REPLACE);
//    }
//
//        // Retrieve the record
//    else {
//        get_phone_no = dc_dbm_fetch(env, err, db, name);
//        /////////////////////////////////////////////////////////////////////////////
//        size_t size = strlen((char *) name.dptr) + strlen((char *) datamm.dptr) + 1;// +1 for the null-terminator
//        char *result = dc_malloc(env, err, size);
//        // in real code you would check for errors in malloc here
//        strcpy(result, (char *) name.dptr);
//        strcat(result, (char *) datamm.dptr);
//        dc_write(env, err, fd, result, size);
//        /////////////////////////////////////////////////////////////////////////////////
//        printf("Name: %s, Phone Number: %s\n", (char *) name.dptr, (char *) datamm.dptr);
//        dc_free(env, result, size);
//    }
//
//
//    // Close the database
//    dc_dbm_close(env, err, db);
//}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void
Write_dbm(struct dc_posix_env *env, struct dc_error *err, char *major, char *minor, char *latitude, char *longitude) {
    DBM *db;

    //-----------------
    char *newKey = (char *) malloc(strlen(major) + strlen(minor) + 1);

    char delim[] = "-";
    char *delimPtr = delim;
    strcpy(newKey, major);
    strcat(newKey, delimPtr);
    strcat(newKey, minor);
    printf("TEST NEW KEY = %s\n", newKey);

    char *newVal = (char *) malloc(strlen(latitude) + strlen(longitude) + 1);

    strcpy(newVal, latitude);
    strcat(newVal, delimPtr);
    strcat(newVal, longitude);
    printf("TEST NEW VALUE = %s\n", newVal);

    datum dataVal = {newVal, (int) strlen(newVal)};
    datum dataKey = {newKey, (int) strlen(newKey)};
    // -----------------------


    // Open the database and store the record
    db = dc_dbm_open(env, err, DB_NAME, DC_O_RDWR | DC_O_CREAT, 0600);
    dc_dbm_store(env, err, db, dataKey, dataVal, DBM_REPLACE);

    // Close the database
    dc_dbm_close(env, err, db);

}

char *Read_dbm(struct dc_posix_env *env, struct dc_error *err, int fd) {
    DBM *db;
    datum key;
    datum get_maj;

    char response_GET_first[] = "Beacons in Data base\n";
    // TO DELETE LATER (these 2 lines)
//    datum name = {NAME, sizeof(NAME)};
//    datum datamm = {data, sizeof(data)};



    printf("\nget all data from database:\n");
    db = dc_dbm_open(env, err, DB_NAME, O_RDONLY, 0600);

    for (key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {
        get_maj = dc_dbm_fetch(env, err, db, key);
        printf("\ngetting data: %s, %s\n", key.dptr, get_maj.dptr);
        dc_strcat(env, response_GET_first, key.dptr);
        dc_strcat(env, response_GET_first, "\r\n");
        dc_strcat(env, response_GET_first, get_maj.dptr);
    }

    printf("\nfinal database: %s\n", response_GET_first);
    char *response = calloc(strlen(response_GET_first), sizeof(char));
    strcat(response, response_GET_first);
    response[strlen(response)] = '\0';


    // TO DELETE LATER
    /////////////////////////////////////////////////////////////////////////////
//    size_t size = strlen((char *) name.dptr) + strlen((char *) datamm.dptr) + 1;// +1 for the null-terminator
//    char *result = dc_malloc(env, err, size);
//    // in real code you would check for errors in malloc here
//    strcpy(result, (char *) name.dptr);
//    strcat(result, (char *) datamm.dptr);
//    dc_write(env, err, fd, result, size);
//    /////////////////////////////////////////////////////////////////////////////////
//
//    dc_free(env, result, size);
    // Close the database
    dc_dbm_close(env, err, db);
    return response;
}

