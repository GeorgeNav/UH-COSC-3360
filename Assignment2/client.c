#include <stdlib.h>
// Please note this is a C program
// It compiles without warnings with gcc
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>

void error(char*);
void communicate(int, char**, int*, const int, int);
const int children = 3;

int main(int argc, char *argv[]) {
    const int m_size = 4;
    int matrix[m_size][m_size];
    matrix[0][0] = 0; matrix[0][1] = 0; matrix[0][2] = 0; matrix[0][3] = 0;
    matrix[1][0] = 0; matrix[1][1] = 1; matrix[1][2] = 0; matrix[1][3] = 1;
    matrix[2][0] = 0; matrix[2][1] = 0; matrix[2][2] = 1; matrix[2][3] = 1;
    matrix[3][0] = 0; matrix[3][1] = 1; matrix[3][2] = 1; matrix[3][3] = 0;
    // Communication stuff
    if (argc < 3) { // if program is ran with 2 or less arguments
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    char input[100];
    int f[3][2] = {{0,0},{0,0},{0,0}};
    int r = 0;
    while(gets(input)) {
        f[r][0] = atoi(&input[0]);
        f[r][1] = atoi(&input[2]);
        // printf("%d %d\n", f[r][0], f[r][1]);
        r++;
    }

    // Parent and children stuff
    pid_t pid = 0;

    int child;
    for(child = 1; child <= children; child++) // Child creation
        if((pid = fork()) != 0) { // wait
            int * w;
            w = malloc(sizeof(int) * (m_size+2));
            for(int i = 0; i < m_size; i++)
                w[i] = matrix[child][i];
            w[4] = f[child-1][0];
            w[5] = f[child-1][1];
            printf("Child %d, sending value: %d to child process %d\n", child, w[5], w[4]);
            communicate(argc, argv, w, m_size+2, child);
            break;
        }
    return 1;
}

void error(char *msg) { // Called when system call fails
    perror(msg); // display error
    exit(0); // abort program
}

void communicate(int argc, char **argv, int *w, const int size, int child) {
    int matrix[4][4];
    matrix[0][0] = 0; matrix[0][1] = 0; matrix[0][2] = 0; matrix[0][3] = 0;
    matrix[1][0] = 0; matrix[1][1] = 1; matrix[1][2] = 0; matrix[1][3] = 1;
    matrix[2][0] = 0; matrix[2][1] = 0; matrix[2][2] = 1; matrix[2][3] = 1;
    matrix[3][0] = 0; matrix[3][1] = 1; matrix[3][2] = 1; matrix[3][3] = 0;
    const int w_size = 256;
    char w_chars[w_size];
    bzero(w_chars, w_size);
    for(int i = 0; i < size; i++)
        w_chars[i] = w[i] + '0';
    // printf("%s\n", w_chars);
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    const int buffer_size = 256;
    char buffer[buffer_size];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    // Get encoded message from server
    n = write(sockfd,w_chars,strlen(w_chars));
    if (n < 0) 
        error("ERROR writing to socket");
    bzero(buffer, buffer_size);
    n = read(sockfd, buffer, buffer_size-1);
    // printf("Server response: \'%s\'\n", buffer);

    // Decode message
    // Convert ints from string
    int message[12];
    int sender;
    int num_count = 0;
    int neg = 0;
    for(int i = 0; i < buffer_size; i++) {
        if(isdigit(buffer[i]) && neg == 0) {
            if(num_count < 12)
                message[num_count] = buffer[i] - '0';
            else
                sender = buffer[i] - '0';
            num_count++;
        } else if(isdigit(buffer[i]) && neg == 1) {
            int val = buffer[i] - '0';
            message[num_count] = -1 * val;
            neg = 0;
            num_count++;
        }
        else if(buffer[i] == '-')
            neg = 1;
        else if(buffer[i] != ' ')
            break;
    }

    sleep(child);
    printf("\nChild: %d\n", child);
    printf("Signal: ");
    for(int i = 0; i < 12; i++)
        printf("%d ", message[i]);
    printf("\n");
    for(int i = 0; i < 12; i++)
        if(matrix[sender][i%4] == 0)
            message[i] = message[i] * -1;
/*     for(int i = 0; i < 12; i++)
        printf("%d ", message[i]);
    printf("\n"); */

    int hidden_message[3] = {0, 0, 0};
    int index = 0;
    int h_index = -1;
    while(index < 12) {
        if(index%4 == 0)
            h_index++;
        hidden_message[h_index] += message[index];
        index++;
    }

    // printf("Decoded message in binary: ");
    printf("Code: -1 ");
    for(int i = 0; i < 3; i++) {
        hidden_message[i] /= 4;
        printf("%d ", hidden_message[i]);
    }
    printf("\n");

    int value = 0;
    for(int i = 2; i >= 0; i--) {
        if(hidden_message[i] == 1 && i == 0)
            value += 4;
        else if(hidden_message[i] == 1 && i == 1)
            value += 2;
        else if(hidden_message[i] == 1 && i == 2)
            value += 1;
    }
    printf("Received value: %d", value);
    printf("\n");
    sleep(children - child);
}