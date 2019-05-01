// George Navarro 1481556
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

enum { POSITIONS = 10 };
static pthread_mutex_t pos_mutexes[POSITIONS];
static pthread_cond_t pos_conditionals[POSITIONS];
static int position_in_use[POSITIONS] = {0};
struct db_user * users;
pthread_t * tid;
static int waiting_group;
static int waiting_locked;
static int last_user_of_first_group;
static int last_user_of_last_group;

struct dbms_user {
    int id;
    int group_num;
    int pos_request;
    int wait_time;
    int request_time;
};

typedef struct queue {
    int capacity;
    int size;
    int front; // front index
    int rear; // rear index
    int * data;
} queue;
static queue current_group;
static queue * pos_queues;

struct data {
    int contents;
    int position;
};
static struct data db_data[POSITIONS]; // data from database

struct db_user {
    int id;
    int group_num;
    int pos_request;
    int wait_time;
    int request_time;
};

void *thread_func(void*);
void *wait_func(void*);
queue *createQueue(int);
int queueAdd(queue*, int);
int queuePop(queue*);
int queueFront(queue*);
void printQueue(queue*);

int main(int argc, char *argv[]) {
    int num_users = 0;
    static int group_one_size;
    static int * group_one;
    static int group_two_size;
    static int * group_two;
    group_one_size = 0;
    group_two_size = 0;
    waiting_group = 0;
    waiting_locked = 0;
    last_user_of_first_group = 0;
    last_user_of_last_group = 0;

    /* Initialize database management data */
    for(int i = 0; i < POSITIONS; i++) {
        db_data[i].contents = 0;
        db_data[i].position = i+1;
        pthread_mutex_init(&pos_mutexes[i], NULL);
        pthread_cond_init(&pos_conditionals[i], NULL);
    }
    
    current_group = *createQueue(3);
    int temp;
    scanf("%d", &temp);
    scanf("%*[^\n]s");
    if(temp == 1) {
        queueAdd(&current_group, 1);
        queueAdd(&current_group, 2);
    } else {
        queueAdd(&current_group, 2);
        queueAdd(&current_group, 1);
    }

    int user = 1;
    while(scanf("%d", &temp) != -1) {
        users = realloc(users, sizeof(struct dbms_user)*(++num_users)); // add one more user
        users[user-1].id = user;
        users[user-1].group_num = temp;
        scanf("%d", &users[user-1].pos_request);
        scanf("%d", &users[user-1].wait_time);
        scanf("%d", &users[user-1].request_time);
        scanf("%*[^\n]s");
        if(users[user-1].group_num == 1) {
            group_one = realloc(group_one, sizeof(int)*(++group_one_size)); // add one more user
            group_one[group_one_size-1] = users[user-1].id;
        } else if(users[user-1].group_num == 2) {
            group_two = realloc(group_two, sizeof(int)*(++group_two_size)); // add one more user
            group_two[group_two_size-1] = users[user-1].id;
        }
        if(queueFront(&current_group) == temp)
            last_user_of_first_group = user;
        else
            last_user_of_last_group = user;
        user++;
    }

/*     printf("# of users: %d\n", num_users);
    printf("First group: %d\n", queueFront(&current_group));
    printf("id\tgroup\tpos\tstart\ttime\n");
    for(int i = 0; i < num_users; i++)
        printf("%d\t%d\t%d\t%d\t%d\n",
            users[i].id,
            users[i].group_num,
            users[i].pos_request,
            users[i].wait_time,
            users[i].request_time);
    printf("Group 1: ");
    for(int i = 0; i < group_one_size; i++)
        printf("%d ", group_one[i]);
    printf("\n\tgroup one size: %d\n", group_one_size);
    printf("Group 2: ");
    for(int i = 0; i < group_two_size; i++)
        printf("%d ", group_two[i]);
    printf("\n\tgroup two size: %d\n\n\n", group_two_size); */

    /* Make queues for each data position */
    pos_queues = malloc(sizeof(queue) * POSITIONS); // make queue for 
    for(int i = 0; i < POSITIONS; i++)
        pos_queues[i] = *createQueue(num_users+1);

    int g = queuePop(&current_group);
    int first_group = g;
    while(g != -1) {
        if(g == 1) {
            for(int i = 0; i < POSITIONS; i++)
                for(int j = 0; j < group_one_size; j++) {
                    if(users[group_one[j]-1].pos_request == i+1)
                        //printf("adding %d to queue %d\n", group_one[j], i+1);
                        queueAdd(&pos_queues[i], group_one[j]);
                    }
        } else if(g == 2) {
            for(int i = 0; i < POSITIONS; i++)
                for(int j = 0; j < group_two_size; j++)
                    if(users[group_two[j]-1].pos_request == i+1) {
                        //printf("adding %d to queue %d\n", group_two[j], i+1);
                        queueAdd(&pos_queues[i], group_two[j]);
                    }
        } 
        g = queuePop(&current_group);
    }
    if(first_group == 1) { // return group queue back to normal
        queueAdd(&current_group, 1);
        queueAdd(&current_group, 2);
    } else {
        queueAdd(&current_group, 2);
        queueAdd(&current_group, 1);
    }
/*     for(int i = 0; i < POSITIONS; i++) {
        printf("Queue %d:\t", i+1);
        printQueue(&pos_queues[i]);
    }
    printf("\nGroup queue:\t");
    printQueue(&current_group);
    printf("\n"); */

    /* Create threads for each user */
    const int NTHREADS = num_users;
 	tid = malloc(sizeof(pthread_t)*NTHREADS);
    if ((first_group == 1 && group_one_size > 0) || (first_group == 2 && group_two_size > 0)) {
        for (int i = 0; i < NTHREADS; i++) {
            if(i > 0)
                sleep(users[i-1].wait_time);

            if(users[i].group_num == queueFront(&current_group)) {
                if(pthread_create(&tid[i], NULL, thread_func, &users[i]) == -1) {
                    fprintf(stderr, "Error creating thread\n");
                    return 1;
                }
            } else {
                if(pthread_create(&tid[i], NULL, wait_func, &users[i]) == -1) {
                    fprintf(stderr, "Error creating thread\n");
                    return 1;
                }
            }
        }
    }
    for(int i = 0; i < NTHREADS; i++) // wait for first group (Group 1 or 2) to finish
        if(users[i].group_num == queueFront(&current_group))
            pthread_join(tid[i], NULL);

    printf("\nAll users from Group %d finished their execution\n", queuePop(&current_group));
    printf("The users from Group %d start their execution\n\n", queueFront(&current_group));

    for(int i = 0; i < NTHREADS; i++) {
        if(last_user_of_first_group == 0 && i > 0)
            sleep(users[i-1].wait_time);

        if(users[i].group_num == queueFront(&current_group))
            if(pthread_create(&tid[i], NULL, thread_func, &users[i]) == -1) {
                fprintf(stderr, "Error creating thread\n");
                return 1;
            }
    }
    for(int i = 0; i < NTHREADS; i++) // wait for second group (Other group) to finish
        if(users[i].group_num == queueFront(&current_group))
            pthread_join(tid[i], NULL);

    printf("All users from Group %d finished their execution\n\n", queuePop(&current_group));

    printf("Total Requests:\n");
    printf("\tGroup 1: %d\n", group_one_size);
    printf("\tGroup 2: %d\n\n", group_two_size);
    printf("Requests that waited:\n");
    printf("\tDue to it\'s group: %d\n", waiting_group);
    printf("\tDue to a locked position: %d\n\n", waiting_locked);

    return 0;
}

void *wait_func(void *void_ptr) { // last group threads access this function
    struct db_user * user = void_ptr;
    printf("User %d from Group %d arrives to the DBMS\n",
        user->id,
        user->group_num);
    sleep(user->wait_time);
    if(users[user->id-1].group_num != queueFront(&current_group)) {
        waiting_group++;
        printf("User %d is waiting due to it's group\n", user->id);
    }
    pthread_exit(NULL);
}

void *thread_func(void *void_ptr) { // threads access this function
    struct db_user * user = void_ptr;
    int n;

    if(current_group.size == 2) {
        printf("User %d from Group %d arrives to the DBMS\n",
            user->id,
            user->group_num);
        sleep(user->wait_time);
    } else {
        if(last_user_of_first_group == 0) {
            sleep(user->wait_time);
            printf("User %d from Group %d arrives to the DBMS\n",
                user->id,
                user->group_num);
        }
        pthread_join(tid[user->id-1], NULL);
    }

    if(position_in_use[user->pos_request-1] != 0) {
        waiting_locked++;
        printf("User %d is waiting: position %d of the database is being used by user %d\n",
            user->id, user->pos_request, position_in_use[user->pos_request-1]);
        if((n = pthread_cond_wait(
            &pos_conditionals[user->pos_request-1],
            &pos_mutexes[user->pos_request-1])) != 0)
            printf("WAIT ERROR: %d", n);
    } else {
        if((n = pthread_mutex_lock(&pos_mutexes[user->pos_request-1])) != 0)
            printf("USER %d LOCKING ERROR: %d\n", user->id, n);
    }

    // Critical section
    position_in_use[user->pos_request-1] = user->id;
    printf("User %d is accessing the position %d of the database for %d second(s)\n",
        user->id, user->pos_request, user->request_time);
    sleep(user->request_time);
    printf("User %d finished it\'s execution\n", user->id);
    position_in_use[user->pos_request-1] = 0;
    // End of critical section

    // Unlock mutex
    if((n = pthread_mutex_unlock(&pos_mutexes[user->pos_request-1])) != 0)
        printf("USER %d UNLOCKING ERROR: %d\n", user->id, n);

    queuePop(&pos_queues[user->pos_request-1]);
    int pos = queueFront(&pos_queues[user->pos_request-1]);
    if(pos != -1 && users[pos-1].group_num == queueFront(&current_group)) {
/*         if((n = pthread_cond_signal_thread_np(
            &pos_conditionals[user->pos_request-1],
            tid[pos-1])) != 0)
            printf("SIGNAL ERROR: %d", n); */
        if((n = pthread_cond_signal(
            &pos_conditionals[user->pos_request-1])) != 0)
            printf("SIGNAL ERROR: %d", n);
    }
    //printf("\t\t\t\tthread %d done\n", (int)pthread_self());
    pthread_exit(NULL);
}

queue * createQueue(int capacity) {
    queue * q = malloc(sizeof(queue));
    q->capacity = capacity;
    q->size = 0;
    q->front = -1;
    q->rear = -1;
    q->data = malloc(sizeof(int)*capacity);
    return q;
}

int queueAdd(queue * q, int i) {
    if(q->size != q->capacity) {
        int newRear = (q->rear+1)%q->capacity;
        q->data[newRear] = i;
        q->size++;
        q->rear = newRear;
        if(q->front == -1)
            q->front = newRear; 
        return 1;
    }
    return 0;
}

int queuePop(queue * q) {
    if(q->size != 0) {
        int i = q->data[q->front];
        if(q->front == q->rear) {
            q->front = -1;
            q->rear = -1;
        } else
            q->front = (q->front+1)%q->capacity;
        q->size--;
        return i;
    }
    return -1;
}

int queueFront(queue * q) {
    if(q->size != 0)
        return q->data[q->front];
    return -1;
}

void printQueue(queue * q) {
    if(q->size == 0)
        printf("<empty>\n");
    else {
        int current = q->front;
        do {
            printf("| %d ", q->data[current]);
            current = (current+1)%q->capacity;
        } while(current <= q->rear);
        printf("|\n");
    }
}
