#include <stdio.h> // standard input ouput
#include <stdlib.h> // standard library
#include <stdbool.h>
#include <unistd.h>

typedef struct car {
    char * licensePlate;
    char lane; // The x-bound (N/E/S/W) lane that the car is waiting in
    int waitingTime; // Amount of time the car will be waiting at the light
} car;

typedef struct queue {
    char name;
    int capacity;
    int size;
    int front; // front index
    int rear; // rear index
    car * cars;
} queue;

queue * createQueue(int, char);
queue * charToLane(char);
car queuePop(queue*);
car queueFront(queue*);
bool addCar(char*, char, int);
bool queueAdd(queue*, car);
bool isEmpty(queue*);
bool trafficExists();
int laneToIndex(char);
char * indexToDirection(int);
void printQueueDetailed(queue*);
void printQueue(queue*);
void printCarMovement(car c);

queue *north, *east, *south, *west;
queue* traffic[4];
/* FILE *f; */

int main() {
    north = createQueue(100, 'N');
    east = createQueue(100, 'E');
    south = createQueue(100, 'S');
    west = createQueue(100, 'W');
    traffic[0] = north;
    traffic[1] = east;
    traffic[2] = south;
    traffic[3] = west;
    int direction;
    int passingCars;
    char input[100];
    pid_t pid;

/*     if(!(f = fopen("./input1.txt", "r"))) {
        printf("Cannot find file in same dir as program.\n");
        exit(EXIT_FAILURE);
    }
    // Use input file to create traffic
    fgets(input, 100, f);
    direction = laneToIndex(input[0]);
    fgets(input, 100, f);
    passingCars = input[0] - '0'; */

    gets(input);
    direction = laneToIndex(input[0]);
    gets(input);
    passingCars = input[0] - '0';

/*     while(fgets(input, 100, f)) { */
    while(gets(input)) {
        int whiteSpace = -1;
        int i = 0;
        for(; i < 100 && whiteSpace == -1; i++)
            if(input[i] == ' ')
                whiteSpace = i;

        i = 0;
        char * lp = malloc(sizeof(char) * whiteSpace);
        for(; i < whiteSpace; i++)
            lp[i] = input[i];
        char l = input[whiteSpace+1];
        int w = input[whiteSpace+3] - '0';
        // printf("%s, %c, %d\n", lp, l, w);
        addCar(lp, l, w);
    }

    // Simulate traffic
    int carsPassed = 0;
    while(trafficExists()) {
        if(traffic[direction]->size != 0)
            printf("Current Direction: %s\n", indexToDirection(direction));
        while((passingCars > carsPassed) && traffic[direction]->size != 0) {
            pid = fork();
            if(pid == 0) {
                printCarMovement(
                    queueFront(traffic[direction])
                );
                sleep(
                    queuePop(traffic[direction]).waitingTime
                );
                _exit(0);
            }
            wait(NULL);
            queuePop(traffic[direction]);
            carsPassed++;
        }
        direction = (direction+1)%4;
        carsPassed = 0;
    }

/*     fclose(f); */
    free(north);
    free(east);
    free(south);
    free(west);
    return 0;
}

bool addCar(char * lp, char l, int w) {
    car c = (car) {
        .licensePlate = lp,
        .lane = l,
        .waitingTime = w
    };
    return queueAdd(charToLane(l), c);
}

bool isEmpty(queue * q) { return q->size == 0 ? true : false; }

bool trafficExists() {
    if( north->size > 0 ||
        east->size > 0 ||
        south->size > 0 ||
        west->size > 0)
        return true;
    else
        return false;
}

queue * charToLane(char l) {
    switch(l) {
        case 'N': return north;
        case 'E': return east;
        case 'S': return south;
        case 'W': return west;
        default: return NULL;
    }
}

int laneToIndex(char l) {
    switch(l) {
        case 'N': return 0;
        case 'E': return 1;
        case 'S': return 2;
        case 'W': return 3;
    }
    return -1;
}

char * indexToDirection(int i) {
    switch(i) {
        case 0: return "Northbound";
        case 1: return "Eastbound";
        case 2: return "Southbound";
        case 3: return "Westbound";
    }
    return "error";
}

queue * createQueue(int capacity, char name) {
    queue * q = malloc(sizeof(queue));
    q->name = name;
    q->capacity = capacity;
    q->size = 0;
    q->front = -1;
    q->rear = -1;
    q->cars = (car*)malloc(sizeof(car*)*capacity);
    return q;
}

bool queueAdd(queue * q, car c) {
    if(q->size != q->capacity) {
        int newRear = (q->rear+1)%q->capacity;
        q->cars[newRear] = c;
        q->size++;
        q->rear = newRear;
        if(q->front == -1)
            q->front = newRear; 
        return true;
    } else
        return false;
}

car queuePop(queue * q) {
    if(q->size != 0) {
        car c = q->cars[q->rear];
        if(q->front == q->rear) {
            q->front = (q->front+1)%q->capacity;
            q->rear = q->front;
        } else
            q->front = (q->front+1)%q->capacity;
        q->size--;
        return c;
    }
}

car queueFront(queue * q) {
    if(q->size != 0)
        return q->cars[q->front];
}

void printCarMovement(car c) {
    printf("Car %s is using the intersection for %d sec(s).\n", c.licensePlate, c.waitingTime);
}

void printQueueDetailed(queue * q) {
    printf("%c - ", q->name);
    printf("capacity: %d, ", q->capacity);
    printf("size: %d, ", q->size);
    printf("front: %d, ", q->front);
    printf("rear: %d, ", q->rear);
    printf("queue: ");
    printQueue(q);
}

void printQueue(queue * q) {
    if(q->size == 0)
        printf("<empty>\n");
    else {
        int current = q->front;
        do {
            printf("| %s ", q->cars[current].licensePlate);
            current = (current+1)%q->capacity;
        } while(current <= q->rear);
        printf("|\n");
    }
}
