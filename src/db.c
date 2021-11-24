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


#define NAME       "Arturo Crespo"
#define PHONE_NO   "723-9273"
#define DB_NAME    "phones"


void Read_Write_dbm(struct dc_posix_env *env, struct dc_error *err, char *type, char *data, int fd) {
    char *major = "1";
    char *minor = "2";
    DBM *db;
    datum name = {NAME, sizeof(NAME)};
    datum dataMaj = {major, sizeof(major)};

    datum put_phone_no = {PHONE_NO, sizeof(PHONE_NO)};
    datum datamm = {data, sizeof(data)};
    datum dataMin = {minor, sizeof(minor)};


    printf(" size of data: %zu\n", sizeof(data));
    printf("Data %s\n", data);
    datum get_maj;
    datum get_min;

    char maj[5] = "major";
    char min[5] = "minor";
    datum majKey = {(void *)maj, strlen(maj) + 1};
    datum minKey = {(void *)min, strlen(min) + 1};

    // Open the database and store the record
    db = dc_dbm_open(env, err, DB_NAME, DC_O_RDWR | DC_O_CREAT, 0600);
    if (strcmp(type, "POST") == 0) {
        dc_dbm_store(env, err, db, majKey, dataMaj, DBM_REPLACE);
        dc_dbm_store(env, err, db, minKey, dataMin, DBM_REPLACE);
    }

        // Retrieve the record
    else {
        dc_dbm_store(env, err, db, majKey, dataMaj, DBM_REPLACE);
        dc_dbm_store(env, err, db, minKey, dataMin, DBM_REPLACE);

        get_maj = dc_dbm_fetch(env, err, db, majKey);
        printf("\nTest get Major: %s\n", get_maj.dptr);
        get_min = dc_dbm_fetch(env, err, db, minKey);
        printf("Test get Minor: %s\n\n", get_min.dptr);
        /////////////////////////////////////////////////////////////////////////////
        size_t size = strlen((char *) name.dptr) + strlen((char *) datamm.dptr) + 1;// +1 for the null-terminator
        char *result = dc_malloc(env, err, size);
        // in real code you would check for errors in malloc here
        strcpy(result, (char *) name.dptr);
        strcat(result, (char *) datamm.dptr);
        dc_write(env, err, fd, result, size);
        /////////////////////////////////////////////////////////////////////////////////
        printf("Name: %s, Phone Number: %s\n", (char *) name.dptr, (char *) datamm.dptr);
  //      printf("Major: %s, Minor: %s\n", (char *) name.dptr, (char *) datamm.dptr);
        dc_free(env, result, size);
    }


    // Close the database
    dc_dbm_close(env, err, db);
}
