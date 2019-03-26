// A simple server in the internet domain using TCP
// The port nu1mber is passed as an argument

// Please note this is a C program
// It compiles without warnings with gcc

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    const int buffer_size = 256;
    char buffer[buffer_size];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    const int num_clients = 3;
    int client = 1;
    int sockets[num_clients];
    const int c_size = 6;
    const int c_num = 4;
    const int num = 5;
    int client_messages[num_clients][c_size];
    int client_order[3];
    while(client <= 3) {
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        sockets[client-1] = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&clilen);
        if (sockets[client-1] < 0)
            error("ERROR on accept");

        bzero(buffer, buffer_size);
        n = read(sockets[client-1], buffer, buffer_size-1);
        if (n < 0)
            error("ERROR reading from socket");

        // printf("Encoding: ");
        for(int b = 0; b < buffer_size; b++)
            if(buffer[b] == '0' || buffer[b] == '1') {
                for(int i = 0; i < 6; i++) {
                    if(buffer[b+i] == '0')
                        client_messages[client-1][i] = -1;
                    else
                        client_messages[client-1][i] = buffer[b+i] - '0';
                    // printf("%d ", client_messages[client-1][i]);
                }
                // printf("\n");
                break;
            }
        client++;
    }

    client = 1;
    const int m_size = 12;
    int encoded_messages[num_clients][m_size];
    int encoded_message[m_size];
    do {
        int val = client_messages[client-1][num];
        // printf("(%d)base10 -> ", val);
        int binary[3] = {-1, -1, -1};
        int index = 2;
        while(val != 0) {
            if(val%2 != 0)
                binary[index] = 1;
            val = val/2;
            index--;
        }
        // printf("(%d %d %d)base3\n", binary[0], binary[1], binary[2]);
        int m_index = 0;
        int b_index = -1;
        while(m_index < m_size) {
            if(m_index%4 == 0)
                b_index++;
            encoded_messages[client-1][m_index] = binary[b_index]*client_messages[client-1][m_index%4];
            // printf("%d ", encoded_messages[client-1][m_index]);
            m_index++;
        }
        // printf("\n");
        client++;
    } while(client <= num_clients);

    int m_index = 0;
    while(m_index < m_size) {
        encoded_message[m_index] =
            encoded_messages[0][m_index]
            + encoded_messages[1][m_index]
            + encoded_messages[2][m_index];
        m_index++;
    }

/*     printf("Encoded message: [");
    for(m_index = 0; m_index < m_size; m_index++) {
        printf("%d", encoded_message[m_index]);
        if(m_index+1 != m_size)
            printf(", ");
    }
    printf("]\n"); */

    // Send message back to each child
    for(client = 1; client <= num_clients; client++) {
        bzero(buffer, buffer_size);
        n = sprintf(buffer, "%d %d %d %d %d %d %d %d %d %d %d %d %d",
            encoded_message[0],
            encoded_message[1],
            encoded_message[2],
            encoded_message[3],
            encoded_message[4],
            encoded_message[5],
            encoded_message[6],
            encoded_message[7],
            encoded_message[8],
            encoded_message[9],
            encoded_message[10],
            encoded_message[11],
            client
        );

        printf("Here is message from child %d: Value = %d, Destination = %d\n", client, client_messages[client_messages[client-1][4]-1][num], client_messages[client_messages[client-1][4]-1][c_num]);
        // printf("Sending client %d\'s message from %d: \'%s\'\n", client_messages[client-1][c_num], client, buffer);
        n = write(sockets[client_messages[client-1][4]-1], buffer, sizeof(buffer));
        if (n < 0)
            error("ERROR writing to socket");
    }
    
    return 0;
}