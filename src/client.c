#include <stdio.h>
#include <stdlib.h>
#include <dc_posix/dc_string.h>
#include <dc_posix/dc_netdb.h>
#include <dc_posix/dc_posix_env.h>
#include <dc_posix/dc_unistd.h>
#include <dc_posix/dc_signal.h>
#include <dc_posix/sys/dc_socket.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <dc_posix/dc_stdlib.h>

#define SIZE 1024

static void error_reporter(const struct dc_error *err);

static void trace_reporter(const struct dc_posix_env *env, const char *file_name,
                           const char *function_name, size_t line_number);

static void quit_handler(int sig_num);


static volatile sig_atomic_t exit_flag;

/**
 * main method runs the program.
 * acts as a client and sends a get request to server.
 * uses ncurses as a text based gui to show data
 * received from server.
 *
 * @return
 */
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
    dc_memset(&env, &hints, 0, sizeof(hints));
    hints.ai_family = PF_INET; // PF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    dc_getaddrinfo(&env, &err, host_name, NULL, &hints, &result);

    if (dc_error_has_no_error(&err)) {
        int socket_fd;

        socket_fd = dc_socket(&env, &err, result->ai_family, result->ai_socktype, result->ai_protocol);

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

            /**
             * Connects to the server.
             */
            if (dc_error_has_no_error(&err)) {
                dc_connect(&env, &err, socket_fd, sockaddr, sockaddr_size);

                if (dc_error_has_no_error(&err)) {
                    struct sigaction old_action;

                    dc_sigaction(&env, &err, SIGINT, NULL, &old_action);

                    if (old_action.sa_handler != SIG_IGN) {
                        struct sigaction new_action;
                        char data[1024];

                        exit_flag = 0;
                        new_action.sa_handler = quit_handler;
                        sigemptyset(&new_action.sa_mask);
                        new_action.sa_flags = 0;
                        dc_sigaction(&env, &err, SIGINT, &new_action, NULL);
                        strcpy(data, "GET / HTTP/1.0\r\n\r\n");

                        if (dc_error_has_no_error(&err)) {

                            /**
                             * starts ncurses window
                             * gets input from user
                             * based on input do a get request to the server.
                             * servers responds back and display the data
                             * on ncurses window.
                             */
                            char mesg[] = "Enter 1 to get data: ";         /* message to be appeared on the screen */
                            char str[80];
                            int row, col;                            /* to store the number of rows and *
                                                 * the number of colums of the screen */
                            initscr();                              /* start the curses mode */
                            getmaxyx(stdscr, row, col);               /* get the number of rows and columns */
                            mvprintw(row / 2, (col - strlen(mesg)) / 2, "%s", mesg);


                            strcpy(data,
                                   "GET / HTTP/1.1\nHost: 127.0.0.1:8081\nUser-Agent: curl/7.68.0\nAccept: /\r\n\r\n");
                            dc_write(&env, &err, socket_fd, data, strlen(data));
                            printw("\n\nBecans(major-minor,lat-ln)");

                            char *comingdata = dc_malloc(&env, &err, SIZE); //I should change the size

                            dc_read(&env, &err, socket_fd, comingdata, SIZE);

                            printw("\n%s\n", comingdata);

                            dc_free(&env, comingdata, SIZE);
                            getch();
                            endwin();


                        }
                    }
                }
            }
        }

        if (dc_error_has_no_error(&err)) {
            dc_dc_close(&env, &err, socket_fd);
        }
    }
    return EXIT_SUCCESS;
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
    ////////////////////main branch
}
