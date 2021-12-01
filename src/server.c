#include <dc_application/command_line.h>
#include <dc_application/config.h>
#include <dc_application/options.h>
#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/sys/dc_socket.h>
#include <dc_posix/dc_fcntl.h>
#include <dc_posix/dc_ndbm.h>
#include <dc_network/common.h>
#include <dc_network/options.h>
#include <dc_network/server.h>
#include <dc_util/streams.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <malloc.h>
#include "common.h"
#include "db.h"


struct application_settings {
    struct dc_opt_settings opts;
    struct dc_setting_bool *verbose;
    struct dc_setting_string *hostname;
    struct dc_setting_regex *ip_version;
    struct dc_setting_uint16 *port;
    struct dc_setting_bool *reuse_address;
    struct addrinfo *address;
    int server_socket_fd;
};

struct dc_application_settings *create_settings(const struct dc_posix_env *env, struct dc_error *err);

static int
destroy_settings(const struct dc_posix_env *env, struct dc_error *err, struct dc_application_settings **psettings);

int run(const struct dc_posix_env *env, struct dc_error *err, struct dc_application_settings *settings);

void signal_handler(int signnum);

void do_create_settings(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void do_create_socket(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void do_set_sockopts(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void do_bind(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void do_listen(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void do_setup(const struct dc_posix_env *env, struct dc_error *err, void *arg);

bool do_accept(const struct dc_posix_env *env, struct dc_error *err, int *client_socket_fd, void *arg);

void do_shutdown(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void do_destroy_settings(const struct dc_posix_env *env, struct dc_error *err, void *arg);

void error_reporter(const struct dc_error *err);

void trace(const struct dc_posix_env *env, const char *file_name, const char *function_name, size_t line_number);

void write_displayer(const struct dc_posix_env *env, struct dc_error *err, const uint8_t *data, size_t count,
                     size_t file_position, void *arg);

void read_displayer(const struct dc_posix_env *env, struct dc_error *err, const uint8_t *data, size_t count,
                    size_t file_position, void *arg);

int receive_data(const struct dc_posix_env *env, struct dc_error *err, int fd, size_t size, char **response);


static volatile sig_atomic_t exit_flag;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static volatile sig_atomic_t exit_signal = 0;

void trace_reporter(__attribute__((unused)) const struct dc_posix_env *env,
                    const char *file_name,
                    const char *function_name,
                    size_t line_number);


int main(int argc, char *argv[]) {
    dc_error_reporter reporter;
    dc_posix_tracer tracer;
    struct dc_posix_env env;
    struct dc_error err;
    struct dc_application_info *info;
    int ret_val;
    struct sigaction sa;

    reporter = error_reporter;
    tracer = trace_reporter;
    tracer = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);
    dc_memset(&env, &sa, 0, sizeof(sa));
    sa.sa_handler = &signal_handler;
    dc_sigaction(&env, &err, SIGINT, &sa, NULL);
    dc_sigaction(&env, &err, SIGTERM, &sa, NULL);

    info = dc_application_info_create(&env, &err, "Test Application");
    ret_val = dc_application_run(&env, &err, info, create_settings, destroy_settings, run, dc_default_create_lifecycle,
                                 dc_default_destroy_lifecycle,
                                 "~/.dcecho.conf",
                                 argc, argv);
    dc_application_info_destroy(&env, &info);
    dc_error_reset(&err);

    return ret_val;
}


struct dc_application_settings *create_settings(const struct dc_posix_env *env, struct dc_error *err) {
    static const bool default_verbose = false;
    static const char *default_hostname = "localhost";
    static const char *default_ip = "IPv4";
    static const uint16_t default_port = 8080;
    static const bool default_reuse = false;
    struct application_settings *settings;

    settings = dc_malloc(env, err, sizeof(struct application_settings));

    if (settings == NULL) {
        return NULL;
    }

    settings->opts.parent.config_path = dc_setting_path_create(env, err);
    settings->verbose = dc_setting_bool_create(env, err);
    settings->hostname = dc_setting_string_create(env, err);
    settings->ip_version = dc_setting_regex_create(env, err, "^IPv[4|6]");
    settings->port = dc_setting_uint16_create(env, err);
    settings->reuse_address = dc_setting_bool_create(env, err);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
    struct options opts[] =
            {
                    {(struct dc_setting *) settings->opts.parent.config_path, dc_options_set_path,   "config",  required_argument, 'c', "CONFIG",        dc_string_from_string, NULL,            dc_string_from_config, NULL},
                    {(struct dc_setting *) settings->verbose,                 dc_options_set_bool,   "verbose", no_argument,       'v', "VERBOSE",       dc_flag_from_string,   "verbose",       dc_flag_from_config,   &default_verbose},
                    {(struct dc_setting *) settings->hostname,                dc_options_set_string, "host",    required_argument, 'h', "HOST",          dc_string_from_string, "host",          dc_string_from_config, default_hostname},
                    {(struct dc_setting *) settings->ip_version,              dc_options_set_regex,  "ip",      required_argument, 'i', "IP",            dc_string_from_string, "ip",            dc_string_from_config, default_ip},
                    {(struct dc_setting *) settings->port,                    dc_options_set_uint16, "port",    required_argument, 'p', "PORT",          dc_uint16_from_string, "port",          dc_uint16_from_config, &default_port},
                    {(struct dc_setting *) settings->reuse_address,           dc_options_set_bool,   "force",   no_argument,       'f', "REUSE_ADDRESS", dc_flag_from_string,   "reuse_address", dc_flag_from_config,   &default_reuse},
            };
#pragma GCC diagnostic pop

    // note the trick here - we use calloc and add 1 to ensure the last line is all 0/NULL
    settings->opts.opts = dc_calloc(env, err, (sizeof(opts) / sizeof(struct options)) + 1, sizeof(struct options));
    dc_memcpy(env, settings->opts.opts, opts, sizeof(opts));
    settings->opts.flags = "c:vh:i:p:f";
    settings->opts.env_prefix = "DC_ECHO_";

    return (struct dc_application_settings *) settings;
}

int destroy_settings(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
                     struct dc_application_settings **psettings) {
    struct application_settings *app_settings;

    app_settings = (struct application_settings *) *psettings;
    dc_setting_bool_destroy(env, &app_settings->verbose);
    dc_setting_string_destroy(env, &app_settings->hostname);
    dc_setting_uint16_destroy(env, &app_settings->port);
    dc_free(env, app_settings->opts.opts, app_settings->opts.opts_size);
    dc_free(env, app_settings, sizeof(struct application_settings));

    if (env->null_free) {
        *psettings = NULL;
    }

    return 0;
}


static struct dc_server_lifecycle *create_server_lifecycle(const struct dc_posix_env *env, struct dc_error *err) {
    struct dc_server_lifecycle *lifecycle;

    lifecycle = dc_server_lifecycle_create(env, err);
    dc_server_lifecycle_set_create_settings(env, lifecycle, do_create_settings);
    dc_server_lifecycle_set_create_socket(env, lifecycle, do_create_socket);
    dc_server_lifecycle_set_set_sockopts(env, lifecycle, do_set_sockopts);
    dc_server_lifecycle_set_bind(env, lifecycle, do_bind);
    dc_server_lifecycle_set_listen(env, lifecycle, do_listen);
    dc_server_lifecycle_set_setup(env, lifecycle, do_setup);
    dc_server_lifecycle_set_accept(env, lifecycle, do_accept);
    dc_server_lifecycle_set_shutdown(env, lifecycle, do_shutdown);
    dc_server_lifecycle_set_destroy_settings(env, lifecycle, do_destroy_settings);

    return lifecycle;
}

static void destroy_server_lifecycle(const struct dc_posix_env *env, struct dc_server_lifecycle **plifecycle) {
    DC_TRACE(env);
    dc_server_lifecycle_destroy(env, plifecycle);
}


int run(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
        struct dc_application_settings *settings) {
    int ret_val;
    struct dc_server_info *info;

    info = dc_server_info_create(env, err, "Echo Server", NULL, settings);

    if (dc_error_has_no_error(err)) {
        dc_server_run(env, err, info, create_server_lifecycle, destroy_server_lifecycle);
        dc_server_info_destroy(env, &info);
    }

    if (dc_error_has_no_error(err)) {
        ret_val = 0;
    } else {
        ret_val = -1;
    }

    return ret_val;
}


void signal_handler(__attribute__ ((unused)) int signnum) {
    printf("caught\n");
    exit_signal = 1;
}

void do_create_settings(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct application_settings *app_settings;
    const char *ip_version;
    int family;

    DC_TRACE(env);
    app_settings = arg;
    ip_version = dc_setting_regex_get(env, app_settings->ip_version);

    if (dc_strcmp(env, ip_version, "IPv4") == 0) {
        family = PF_INET;
    } else {
        if (dc_strcmp(env, ip_version, "IPv6") == 0) {
            family = PF_INET6;
        } else {
            DC_ERROR_RAISE_USER(err, "Invalid ip_version", -1);
            family = 0;
        }
    }

    if (dc_error_has_no_error(err)) {
        const char *hostname;

        hostname = dc_setting_string_get(env, app_settings->hostname);
        dc_network_get_addresses(env, err, family, SOCK_STREAM, hostname, &app_settings->address);
    }
}

void do_create_socket(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct application_settings *app_settings;
    int socket_fd;

    DC_TRACE(env);
    app_settings = arg;
    socket_fd = dc_network_create_socket(env, err, app_settings->address);

    if (dc_error_has_no_error(err)) {
        app_settings = arg;
        app_settings->server_socket_fd = socket_fd;
    } else {
        socket_fd = -1;
    }
}

void do_set_sockopts(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct application_settings *app_settings;
    bool reuse_address;

    DC_TRACE(env);
    app_settings = arg;
    reuse_address = dc_setting_bool_get(env, app_settings->reuse_address);
    dc_network_opt_ip_so_reuse_addr(env, err, app_settings->server_socket_fd, reuse_address);
}

void do_bind(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct application_settings *app_settings;
    uint16_t port;

    DC_TRACE(env);
    app_settings = arg;
    port = dc_setting_uint16_get(env, app_settings->port);

    dc_network_bind(env,
                    err,
                    app_settings->server_socket_fd,
                    app_settings->address->ai_addr,
                    port);
}

void do_listen(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct application_settings *app_settings;
    int backlog;

    DC_TRACE(env);
    app_settings = arg;
    backlog = 5;    // TODO: make this a setting
    dc_network_listen(env, err, app_settings->server_socket_fd, backlog);
}

void do_setup(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
              __attribute__ ((unused)) void *arg) {
    DC_TRACE(env);
}

bool do_accept(const struct dc_posix_env *env, struct dc_error *err, int *client_socket_fd, void *arg) {
    struct application_settings *app_settings;
    bool ret_val;

    DC_TRACE(env);
    app_settings = arg;
    ret_val = false;
    *client_socket_fd = dc_network_accept(env, err, app_settings->server_socket_fd);

    if (dc_error_has_error(err)) {
        if (exit_signal == true && dc_error_is_errno(err, EINTR)) {
            ret_val = true;
        }
    } else {

        ///////////////////////////////////////////////////////////////////


        char *response = NULL;

        int receive_status = receive_data(env, err, *client_socket_fd, BUFSIZ, &response);
        printf("\nResponse\n%s", response);
        dc_send(env, err, *client_socket_fd, response, strlen(response), 0);
        free(response);
        dc_dc_close(env, err, *client_socket_fd);
        exit_flag = 0;



//        while (!(exit_flag) && dc_error_has_no_error(&err)) {
//            char *response = NULL;
//            if (dc_error_has_no_error(&err)) {
//
//                int receive_status = receive_data(&env, &err, client_socket_fd, BUFSIZ, &response);
//                printf("\nResponse\n%s", response);
//                dc_send(&env, &err, client_socket_fd, response, strlen(response), 0);
//                free(response);
//                dc_dc_close(&env, &err, client_socket_fd);
//                exit_flag = 0;
//
//
//            } else {
//                if (err.type == DC_ERROR_ERRNO && err.errno_code == EINTR) {
//                    dc_error_reset(&err);
//                }
//            }
//        }
    }

    return ret_val;
}

int receive_data(const struct dc_posix_env *env, struct dc_error *err, int fd, size_t size, char **response) {
    // more efficient would be to allocate the buffer in the caller (main) so we don't have to keep
    // mallocing and freeing the same data over and over again.
    char *data;
    ssize_t count;
    int return_value;
    data = dc_malloc(env, err, size);
    memset(data, 0, size);
    while (!(exit_flag) && (count = dc_read(env, err, fd, data, size)) > 0 && dc_error_has_no_error(err)) {
        dc_write(env, err, STDOUT_FILENO, data, (size_t) count);
        //  &data[length - count]
        char *token;
        char *tokenBody;
        char *lastToken;
        char *ptr;

        char buff[50];


        //----------------test area-------------------



        //----------------test area-------------------

        //1-check if it's GET or PUT
        // walk through tokens
        token = strtok_r(data, "\r\n", &data);
        printf("\nfirst token: %s\n", token);

        char *firstWord;
        int inputLength = strlen(token);
        char *inputCopy = (char *) calloc(inputLength + 1, sizeof(char));
        strncpy(inputCopy, token, inputLength);
        firstWord = strtok(inputCopy, " ");
        printf("\nfirst world: %s\n", firstWord);

        if (strcmp(firstWord, "GET") == 0) {
            char *file_name = strtok(inputCopy + strlen(firstWord) + 2, " ");
            printf("\nBody is:%s\n", file_name);
            if (strncmp(file_name, "HTTP", 4) == 0) {

                char *data_from_DB = Read_dbm(env, err, 1);

                char response_GET_first[] = "HTTP/1.0 200 OK\r\n"
                                            "Date: Monday, 24-Apr-95 12:04:12 GMT\r\n"
                                            "Content-type: text/html\r\n\r\n";

                *response = malloc((strlen(response_GET_first) + strlen(data_from_DB)));

                dc_strcpy(env, *response, response_GET_first);
                dc_strcat(env, *response, data_from_DB);

                printf("\nNcurses get: %s\n", *response);

            } else if (strcmp(file_name, "read_database") == 0) {

                char *data_from_DB = Read_dbm(env, err, 1);
                char response_GET_first[] = "HTTP/1.0 200 OK\r\n"
                                            "Content-type: text/html\r\n\r\n"
                                            "<!Doctype html>\r\n"
                                            "<html>\r\n"
                                            "<head>\r\n"
                                            "<title>GET from file</title>\r\n"
                                            "</head>\r\n"
                                            "<body>\r\n"
                                            "<div>\r\n"
                                            "<h1>";
                char response_GET_second[] = "</h1>\r\n"
                                             "</div>\r\n"
                                             "</body>"
                                             "</html>\r\n\r\n";

                *response = malloc((strlen(response_GET_first) + strlen(response_GET_second) + strlen(data_from_DB)));

                dc_strcpy(env, *response, response_GET_first);
                dc_strcat(env, *response, data_from_DB);
                dc_strcat(env, *response, response_GET_second);

                printf("\nRead from database\n");

            } else {
                printf("\nRead from file\n");
                int num;
                FILE *fptr;

                if ((fptr = fopen("fadi.txt", "r")) == NULL) {
                    printf("Error! opening file");

                    // Program exits if the file pointer returns NULL.
//                    exit(1);
                }
                fseek(fptr, 0, SEEK_END);
                long fsize = ftell(fptr);
                fseek(fptr, 0, SEEK_SET);  /* same as rewind(f); */

                char *string = malloc((unsigned long) (fsize + 1));
                fread(string, 1, (unsigned long) fsize, fptr);
                printf("file contets is: %s", string);


                char response_GET_first[] = "HTTP/1.0 200 OK\r\n"
                                            "Content-type: text/html\r\n"
                                            "\r\n"

                                            "<!Doctype html>\r\n"
                                            "<html>\r\n"
                                            "<head>\r\n"
                                            "<title>GET from file</title>\r\n"
                                            "</head>\r\n"
                                            "<body>\r\n"
                                            "<div>\r\n"
                                            "<h1>";
                char response_GET_second[] = "</h1>\r\n"
                                             "</div>\r\n"
                                             "</body>"
                                             "</html>\r\n\r\n";

                dc_strcat(env, response_GET_first, string);
                dc_strcat(env, response_GET_first, response_GET_second);
                *response = realloc(*response, strlen(response_GET_first) + 1);


                // response = malloc(strlen(respose_PUT) + 1);
                dc_strcpy(env, *response, response_GET_first);

                free(string);
                fclose(fptr);
            }
            return_value = 1;

            //PUT response
        } else if (strcmp(firstWord, "PUT") == 0) {
            // 2- getting the size of body
            char *sub;
            sub = strstr(data, "Content-Length: ");
            if (sub != NULL) {
                // printf("\nSubstring is: %s", sub + 17);
                char *begin = strchr(sub, ' ');
                char *end = strchr(begin, '\r');

//                memcpy(buff, begin, (unsigned long) (end - begin));
//                printf("\nContent-Length is: %s", buff);

                //3-Tokenize the body----------------------
                char *body;
                body = strstr(data, "\r\n\r\n");
                printf("\nBody is: %s\n", body + 4);


                if (body != NULL) {
                    char *modBody = body + 4;
                    char *major = strtok(modBody, "=");
                    char *minor = strtok(NULL, "&");
                    char *longitude = strtok(NULL, "=");
                    char *latitude = strtok(NULL, "&");
                    printf("\nmajor: %s\n", major);
                    printf("\nminor: %s\n", minor);
                    printf("\nlongitude: %s\n", longitude);
                    printf("\nlatitude: %s\n", latitude);

                    Write_dbm(env, err, major, minor, latitude, longitude);
                }
            }
            char respose_PUT[] = "HTTP/1.0 200 OK\r\n"
                                 "Content-type: text/html\r\n"
                                 "\r\n"

                                 "<!Doctype html>\r\n"
                                 "<html>\r\n"
                                 "<head>\r\n"
                                 "<title>PUT response</title>\r\n"
                                 "</head>\r\n"
                                 "<body>\r\n"
                                 "<div>\r\n"
                                 "<h1>PUT response</h1>\r\n"
                                 "</div>\r\n"
                                 "</body>"
                                 "</html>\r\n\r\n";
            *response = realloc(*response, strlen(respose_PUT) + 1);

            // response = malloc(strlen(respose_PUT) + 1);
            strcpy(*response, respose_PUT);
            return_value = 2;
        }



        //404

        //500

//        while (token) {
//            printf("token: %s %lu\n", token, strlen(token));
//            token = strtok_r(data, "\r\n", &data);
//            lastToken = token;
//        }
//
//        printf("\nlast token: %s\n", lastToken);


        exit_flag = 1;
    }

    // dc_free(env, data, size);
    return return_value;
}


void do_shutdown(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err,
                 __attribute__ ((unused)) void *arg) {
    DC_TRACE(env);
}

void
do_destroy_settings(const struct dc_posix_env *env, __attribute__ ((unused)) struct dc_error *err, void *arg) {
    struct application_settings *app_settings;

    DC_TRACE(env);
    app_settings = arg;
    dc_freeaddrinfo(env, app_settings->address);
}

void error_reporter(const struct dc_error *err) {
    if (err->type == DC_ERROR_ERRNO) {
        fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number,
                err->errno_code);
    } else {
        fprintf(stderr, "ERROR: %s : %s : @ %zu : %d\n", err->file_name, err->function_name, err->line_number,
                err->err_code);
    }

    fprintf(stderr, "ERROR: %s\n", err->message);
}


void trace_reporter(__attribute__((unused)) const struct dc_posix_env *env,
                    const char *file_name,
                    const char *function_name,
                    size_t line_number) {
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}

__attribute__ ((unused)) void
trace(__attribute__ ((unused)) const struct dc_posix_env *env, const char *file_name, const char *function_name,
      size_t line_number) {
    fprintf(stdout, "TRACE: %s : %s : @ %zu\n", file_name, function_name, line_number);
}


//                sub = strstr(body + 4, "latitude=");
////               // tokenBody = strtok_r(body + 4, "-", &body);
//                char *begin_lat = strchr(sub, '=');
//                char *end_lat = strchr(begin_lat, '&');
//                char longitude_number[end_lat - begin_lat-1];
//                printf("\nlengthhhhhhhhhh: %ld\n", end_lat - begin_lat);
//                strncpy(longitude_number, begin_lat, (unsigned long) (end_lat - begin_lat));
//                printf("\nlatitude: %s\n", longitude_number);




//connect
//read
//write
//close


//
//    char response[] = "HTTP/1.0 200 OK\r\n"
//                      "Date: Monday, 24-Apr-95 12:04:12 GMT\r\n"
//                      "Content-type: text/html\r\n"
//                      "\r\n"
//                      "message body"
//                      "\r\n\r\n";
//    printf("Greeting message: %s\n", response);
//
//    dc_send(env, err, fd, &response, strlen(response), 0);
/*
 *
int strncmp(const char *str1, const char *str2, size_t n)

Parameters

str1 − This is the first string to be compared.

str2 − This is the second string to be compared.

n − The maximum number of characters to be compared.

Return Value

This function return values that are as follows −

if Return value < 0 then it indicates str1 is less than str2.

if Return value > 0 then it indicates str2 is less than str1.

if Return value = 0 then it indicates str1 is equal to str2.
*/


//The C library function strcspn() calculates the length of the number of characters before the 1st occurrence of character present in both the string.


/*
strcmp vs strncmp
* bothe of them compare from the first char, not string contain
strcmp compares both the strings till null-character of either string comes, whereas strncmp compares at most num characters of both strings.
 But if num is equal to the length of either string than strncmp behaves similar to strcmp.
Problem with strcmp function is that if both of the strings passed in the argument is not terminated by null-character, then comparison of
 characters continues till the system crashes. But with strncmp function we can limit the comparison with num parameter.
*/