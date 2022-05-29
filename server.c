#include "server.h"

interpreter_state current_interpreter_status = I_WAITING;
char file_buff[file_buffer_size];
char *p;
int file_size, client_socket; // total file size

/**
 * @brief checks the client request and determines, if client can go on and execute the action
 *
 * @param client_request
 * @return int if request ok -> CLIENT_REQUEST_IS_OK
 *             else, -> corresponding error code
 */
int check_client_request(int client_request)
{
    int response = DENY_REQUEST;
    switch (client_request)
    {
    case CLIENT_WANTS_TO_SEND_FILE:
        // check if interpreter status correlates with receiving a file
        if (current_interpreter_status == I_WAITING)
        {
            response = CLIENT_REQUEST_IS_OK;
        }
        break;
    case CLIENT_WANTS_TO_SEND_FILE_AND_RUN_INTERPRETER:
        if (current_interpreter_status == I_WAITING || current_interpreter_status == I_READY)
        {
            response = CLIENT_REQUEST_IS_OK;
        }
        break;
    case CLIENT_WANTS_TO_GET_INTERPRETER_STATE:
        // get interpreter state
        response = current_interpreter_status;
        break;
    case CLIENT_WANTS_TO_STOP_INTERPRETER:
        if (current_interpreter_status == I_RUNNING)
        {
            response = CLIENT_REQUEST_IS_OK;
        }
        break;
    case CLIENT_WANTS_TO_RUN_INTERPRETER:
        if (current_interpreter_status == I_READY)
        {
            response = CLIENT_REQUEST_IS_OK;
        }
        break;
    default:
        response = COMMAND_CODE_COULD_NOT_BE_INTERPRETED;
        break;
    }
    return htonl(response); // return response in network-byte-order
}

/**
 * @brief used to recieve a file from the clientclient
 *
 * @param client_socket
 */
void recv_file()
{
    file_size = 0;
    while (1)
    {
        p = file_buff + file_size;
        int bytes_recv = recv(client_socket, p, sizeof(file_buff), 0);
        file_size += bytes_recv;

        if (bytes_recv == 0)
        {
            break;
        }
    }
    current_interpreter_status = I_READY;
}

/**
 * @brief cleanes up, after the interpreter finished running or if an error occured
 *
 */
void cleanup()
{
    file_size = 0;
    current_interpreter_status = I_WAITING;
}

/**
 * @brief provides the interpreter with a pointer to the beginning of the file buffer containing the received xml
 *
 */
void run()
{
    // testing purposes only -> confirm that file was properly received
    // later give interpreter pointer to file_buff
    FILE *f;
    f = fopen("test_c.xml", "w");
    fwrite(file_buff, sizeof(char), file_size, f);
    fclose(f);
    //
    current_interpreter_status = I_EXECUTION_FINISHED;
    cleanup();
}

int main(int argc, char **argv)
{
    char msg_buff[message_buffer_size]; // stores the client message
    char *s;
    int cmd;      // interger representing a command which is provided via the command header
    int response; // response to client
    // create a socket
    int server_socket, bytes_recvd, optval;
    while (1)
    {
        server_socket = socket(AF_INET, SOCK_STREAM, 0);

        // prevent OS from blocking the creation of a listening socket at the same port after the previous socket has closed
        optval = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

        // specify an address for the socket
        struct sockaddr_in server_address;
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(9002);
        server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

        // bind the socket to specified IP and port
        bind(server_socket, (struct sockaddr *)&server_address,
             sizeof(server_address));
        listen(server_socket, 1);
        client_socket = accept(server_socket, NULL, NULL);

        // receive client request
        recv(client_socket, msg_buff, sizeof(msg_buff), 0);
        s = strstr(msg_buff, "<\cmd>"); // check if file_buff contains substring "<\cmd>"
        if (s != NULL)
        {
            // isolate request code
            if (sscanf(s, "%*[^0123456789]%d", &cmd))
            {
                response = check_client_request(cmd);
                if (cmd == CLIENT_WANTS_TO_SEND_FILE)
                {
                    if (response == htonl(CLIENT_REQUEST_IS_OK))
                    {
                        send(client_socket, &response, sizeof(int), 0);
                        recv_file(client_socket);
                    }
                    else // cant receive file
                    {
                        send(client_socket, &response, sizeof(int), 0);
                    }
                }
                else if (cmd == CLIENT_WANTS_TO_RUN_INTERPRETER)
                {
                    if (response == htonl(CLIENT_REQUEST_IS_OK))
                    {
                        send(client_socket, &response, sizeof(int), 0);
                        run();
                    }
                    else // request denied
                    {
                        send(client_socket, &response, sizeof(int), 0);
                    }
                }
                else if (cmd == CLIENT_WANTS_TO_SEND_FILE_AND_RUN_INTERPRETER)
                {
                    if (response == htonl(CLIENT_REQUEST_IS_OK))
                    {
                        send(client_socket, &response, sizeof(int), 0);
                        recv_file(client_socket);
                        run();
                    }
                    else // request denied
                    {
                        send(client_socket, &response, sizeof(int), 0);
                    }
                }
                else // other request
                {
                    send(client_socket, &response, sizeof(int), 0);
                }
            }
            else // no command code was received via the header
            {
                response = htonl(NO_COMMAND_CODE_WAS_RECEIVED);
                send(client_socket, &response, sizeof(int), 0);
            }
        }
        else
        {
            response = htonl(NO_COMMAND_HEADER_WAS_RECEIVED);
            send(client_socket, &response, sizeof(int), 0);
        }
        close(server_socket);
        close(client_socket);
    }

    return EXIT_SUCCESS;
}