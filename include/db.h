//
// Created by fadi on 2021-11-09.
//

#ifndef PROJECT1_DB_H
#define PROJECT1_DB_H

Write_dbm(struct dc_posix_env *env, struct dc_error *err, char *major, char *minor, char *latitude, char *longitude);

char *Read_dbm(struct dc_posix_env *env, struct dc_error *err, int fd);

#endif //PROJECT1_DB_H
