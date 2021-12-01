#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_stdlib.h>
#include <dc_posix/sys/dc_socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "db.h"
#include <ctype.h>

static void error_reporter(const struct dc_error *err);

static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number);

static void quit_handler(int sig_num);

int receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size, char **response);

static volatile sig_atomic_t exit_flag;

int main(void) {
    dc_error_reporter reporter;
    dc_posix_tracer tracer;
    struct dc_error err;
    struct dc_posix_env env;
    const char *host_name;
    struct addrinfo hints;
    struct addrinfo *result;

    reporter = error_reporter;
    tracer = trace_reporter;
    tracer = NULL;
    dc_error_init(&err, reporter);
    dc_posix_env_init(&env, tracer);

    host_name = "192.168.0.18";
    //host_name = "localhost";
    dc_memset(&env, &hints, 0, sizeof(hints));
    hints.ai_family = PF_INET; // PF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;


    dc_getaddrinfo(&env, &err, host_name, NULL, &hints, &result);

    if (dc_error_has_no_error(&err)) {
        int server_socket_fd;

        server_socket_fd = dc_socket(&env, &err, result->ai_family, result->ai_socktype, result->ai_protocol);

        if (dc_error_has_no_error(&err)) {
            struct sockaddr *sockaddr;
            in_port_t port;
            in_port_t converted_port;
            socklen_t sockaddr_size;

            sockaddr = result->ai_addr;
            port = 8081;
            converted_port = htons(port);

            if (sockaddr->sa_family == AF_INET) {
                struct sockaddr_in *addr_in;

                addr_in = (struct sockaddr_in *) sockaddr;
                addr_in->sin_port = converted_port;
                sockaddr_size = sizeof(struct sockaddr_in);
            } else {
                if (sockaddr->sa_family == AF_INET6) {
                    struct sockaddr_in6 *addr_in;

                    addr_in = (struct sockaddr_in6 *) sockaddr;
                    addr_in->sin6_port = converted_port;
                    sockaddr_size = sizeof(struct sockaddr_in6);
                } else {
                    DC_ERROR_RAISE_USER(&err, "sockaddr->sa_family is invalid", -1);
                    sockaddr_size = 0;
                }
            }

            if (dc_error_has_no_error(&err)) {
                dc_bind(&env, &err, server_socket_fd, sockaddr, sockaddr_size);

                if (dc_error_has_no_error(&err)) {
                    int backlog;

                    backlog = 5;
                    dc_listen(&env, &err, server_socket_fd, backlog);

                    if (dc_error_has_no_error(&err)) {
                        struct sigaction old_action;

                        dc_sigaction(&env, &err, SIGINT, NULL, &old_action);

                        if (old_action.sa_handler != SIG_IGN) {
                            struct sigaction new_action;

                            exit_flag = 0;
                            new_action.sa_handler = quit_handler;
                            sigemptyset(&new_action.sa_mask);
                            new_action.sa_flags = 0;
                            dc_sigaction(&env, &err, SIGINT, &new_action, NULL);

                            while (!(exit_flag) && dc_error_has_no_error(&err)) {
                                int client_socket_fd;
                                char *response = NULL;
                                client_socket_fd = dc_accept(&env, &err, server_socket_fd, NULL, NULL);

                                if (dc_error_has_no_error(&err)) {

                                    int receive_status = receive_data(&env, &err, client_socket_fd, BUFSIZ, &response);

//                                    switch (receive_status) {
//                                        default: {
//                                            // 500 response
//                                            char response_500[] = "HTTP/1.0 200 OK\r\n"
//                                                                  "Date: Monday, 24-Apr-95 12:04:12 GMT\r\n"
//                                                                  "Content-type: text/html\r\n"
//                                                                  "\r\n"
//
//                                                                  "<!Doctype html>\r\n"
//                                                                  "<html>\r\n"
//                                                                  "<head>\r\n"
//                                                                  "<title>500</title>\r\n"
//                                                                  "</head>\r\n"
//                                                                  "<body>\r\n"
//                                                                  "<div>\r\n"
//                                                                  "<h1>500</h1>\r\n"
//                                                                  "</div>\r\n"
//                                                                  "</body>"
//                                                                  "</html>\r\n\r\n";
//                                            response = malloc(strlen(response_500) + 1);
//                                            strcpy(response, response_500);
//                                        }
//                                            break;
//                                    }


                                    printf("\nResponse\n%s", response);
                                    dc_send(&env, &err, client_socket_fd, response, strlen(response), 0);
                                    free(response);
                                    dc_dc_close(&env, &err, client_socket_fd);
                                    exit_flag = 0;
                                } else {
                                    if (err.type == DC_ERROR_ERRNO && err.errno_code == EINTR) {
                                        dc_error_reset(&err);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (dc_error_has_no_error(&err)) {
            dc_dc_close(&env, &err, server_socket_fd);
        }
    }

    return EXIT_SUCCESS;
}

int receive_data(struct dc_posix_env *env, struct dc_error *err, int fd, size_t size, char **response) {
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
                printf("\nRead_dbm:%s\n", data_from_DB);
                //free(data_from_DB);
                // response = malloc(strlen(respose_PUT) + 1);
                char response_GET_first[] = "HTTP/1.0 200 OK\r\n"
                                            "Date: Monday, 24-Apr-95 12:04:12 GMT\r\n"
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

                dc_strcat(env, response_GET_first, data_from_DB);
                dc_strcat(env, response_GET_first, response_GET_second);
                *response = realloc(*response, strlen(response_GET_first) + 1);


                // response = malloc(strlen(respose_PUT) + 1);
                dc_strcpy(env, *response, response_GET_first);

                printf("\nNcurses get\n");

            } else if (strcmp(file_name, "read_database") == 0) {
                printf("\nRead from database\n");


            } else {
                printf("\nRead from file\n");
                int num;
                FILE *fptr;

                if ((fptr = fopen("fadi.txt", "r")) == NULL) {
                    printf("Error! opening file");

                    // Program exits if the file pointer returns NULL.
                    exit(1);
                }
                fseek(fptr, 0, SEEK_END);
                long fsize = ftell(fptr);
                fseek(fptr, 0, SEEK_SET);  /* same as rewind(f); */

                char *string = malloc((unsigned long) (fsize + 1));
                fread(string, 1, (unsigned long) fsize, fptr);
                printf("file contets is: %s", string);


                char response_GET_first[] = "HTTP/1.0 200 OK\r\n"
                                            "Date: Monday, 24-Apr-95 12:04:12 GMT\r\n"
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
                                 "Date: Monday, 24-Apr-95 12:04:12 GMT\r\n"
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

        //PUT response

        //404

        //500

        while (token) {
            printf("token: %s %lu\n", token, strlen(token));
            token = strtok_r(data, "\r\n", &data);
            lastToken = token;
        }

        printf("\nlast token: %s\n", lastToken);


        exit_flag = 1;
    }

    // dc_free(env, data, size);
    return return_value;
}


static void quit_handler(int sig_num) {
    exit_flag = 1;
}

static void error_reporter(const struct dc_error *err) {
    fprintf(stderr, "Error: \"%s\" - %s : %s : %d @ %zu\n", err->message, err->file_name, err->function_name,
            err->errno_code, err->line_number);
}

static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number) {
    fprintf(stderr, "Entering: %s : %s @ %zu\n", file_name, function_name, line_number);
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