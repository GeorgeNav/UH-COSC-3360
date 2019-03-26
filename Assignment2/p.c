#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int main()
{
    const int buffer_size = 256;
    char buffer[buffer_size] = "-1 2 2 3 -1 2 2 3 -1 2 2 3 6";
    int message[12];
    int sender = 0;
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

    for(int i = 0; i < 12; i++)
        printf("%d ", message[i]);
    printf(" | %d", sender);
    printf("\n");
}