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


/**
 * Gets the major, minor, latitude, longitude from server
 * opens the database and stores the record for the beacon.
 * @param env
 * @param err
 * @param major
 * @param minor
 * @param latitude
 * @param longitude
 */
void Write_dbm(const struct dc_posix_env *env, struct dc_error *err, char *major, char *minor, char *latitude, char *longitude) {
    DBM *db;

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

    // Open the database and store the record
    db = dc_dbm_open(env, err, DB_NAME, DC_O_RDWR | DC_O_CREAT, 0600);
    dc_dbm_store(env, err, db, dataKey, dataVal, DBM_REPLACE);

    // Close the database
    dc_dbm_close(env, err, db);

}

/**
 * Reads all data from database
 * do string parsing and returns the data to the server.
 * @param env
 * @param err
 * @param fd
 * @return char*
 */
char *Read_dbm(struct dc_posix_env *env, struct dc_error *err, int fd) {
    DBM *db;
    datum key;
    datum get_maj;
    size_t size = 1;

    char *response;
    char delim2[] = "\n";
    char *delimPtr2 = delim2;
    char delimComma[] = ",";
    char *delimCommaPtr = delimComma;


    db = dc_dbm_open(env, err, DB_NAME, O_RDONLY, 0600);

    char header[] = "Beacons in Data base: ";
    size += strlen(header);
    for (key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {

        get_maj = dc_dbm_fetch(env, err, db, key);
        size += strlen((char *) key.dptr) + strlen((char *) delimCommaPtr) + strlen((char *) get_maj.dptr) +
                strlen((char *) delimPtr2);// +1 for the null-terminator
    }
    char *result = dc_malloc(env, err, size);
    strcpy(result, header);


    for (key = dc_dbm_firstkey(env, err, db); key.dptr != NULL; key = dc_dbm_nextkey(env, err, db)) {

        get_maj = dc_dbm_fetch(env, err, db, key);
        printf("data: %s, %s\n", key.dptr, get_maj.dptr);

        // in real code you would check for errors in malloc here
        strcat(result, (char *) key.dptr);
        strcat(result, (char *) delimCommaPtr);
        strcat(result, (char *) get_maj.dptr);
        strcat(result, (char *) delimPtr2);
    }

    printf("\nstrlen(response) + 1: %lu\n", strlen(result) + 1);

    // Close the database
    dc_dbm_close(env, err, db);

    printf("\nfinal database: %s\n", result);
    return result;
}

