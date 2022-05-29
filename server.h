#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>

typedef enum{
    I_WAITING = 10,
    I_READY,
    I_RUNNING,
    I_INTERRUPTED,
    I_EXECUTION_FINISHED,
    I_ERROR
}interpreter_state;

//Global Defs
#define file_buffer_size 100000
#define message_buffer_size 20
//request by client
#define CLIENT_WANTS_TO_SEND_FILE 2
#define CLIENT_WANTS_TO_SEND_FILE_AND_RUN_INTERPRETER 3
#define CLIENT_WANTS_TO_GET_INTERPRETER_STATE 4
#define CLIENT_WANTS_TO_STOP_INTERPRETER 5
#define CLIENT_WANTS_TO_RUN_INTERPRETER 6

//responses to clients requents
#define CLIENT_REQUEST_IS_OK 1


//
//deny request
#define DENY_REQUEST -1
#define CANT_RECIEVE_FILE -2
#define CANT_RECEIVE_FILE_AND_RUN_INTERPRETER -3
#define NO_COMMAND_CODE_WAS_RECEIVED -4
#define NO_COMMAND_HEADER_WAS_RECEIVED -5
#define COMMAND_CODE_COULD_NOT_BE_INTERPRETED -6
#define UNSPECIFIED_ERROR -99

int check_client_request(int client_request);
void recv_file();
void cleanup();
void run();

#endif /* SERVER_H_ */
