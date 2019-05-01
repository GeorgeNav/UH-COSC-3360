#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <algorithm>
#include <queue>
#include <fstream>
using namespace std;

class car {
public:
    string licensePlate;
    string lane; // The orientation of the car at the traffic light.
    int timeWait; // Time the car will wait at the light.

    car(); // Default Constructor
    void setAll(string a, string b, int c); // Mutator
    void print();
};

bool carsAreWaitingIn(queue<car> *intersection);
int directionToIndex(string x);
string indexToDirection(int x);

int main() {
    string direction, lic, dir;
    int pass = 0, currentPass = 0, waiting = 0, currentDir=0;
    pid_t pid;
    queue<car> traffic[4]; // The queues are in order [N, E, S, W]

    /// Copy the Contents of the File Into the Array Based on the Orientation
    car placeholder; // Placeholder to place a node into the Queue.
    ifstream input;
    input.open("./input1.txt");

    input >> direction >> pass; // Reads the direction and number of cars allowed to pass.
    while (input >> lic >> dir >> waiting) { // Keep reading cars into the array until you reached the EoF.
        placeholder.setAll(lic, dir, waiting); // Set the placeholder to copy the data from the input stream.
        if (dir == "N") // Push the car into the appropriate lane.
            traffic[0].push(placeholder);
        else if (dir == "E")
            traffic[1].push(placeholder);
        else if (dir == "S")
            traffic[2].push(placeholder);
        else if (dir == "W")
            traffic[3].push(placeholder);
    }

    currentDir = directionToIndex(direction);

    /// Create Parent/Child threads to work on the nodes in the array.
    // While cars are remaining at the light (in any direction)
    while (carsAreWaitingIn(traffic)) {
        // While the max number of cars allowed to pass hasn't been reached yet.
        // Also, while there remains a car at the intersection:
        if (!traffic[currentDir].empty())
            cout<< "Current Direction: " << indexToDirection(currentDir)<<endl;
        
        while( (pass > currentPass) && (!traffic[currentDir].empty()) ) {
            pid = fork();
            if(pid  == 0) { // Child Process
                traffic[currentDir].front().print();
                sleep(traffic[currentDir].front().timeWait); // Sleep for the assigned time.
                traffic[currentDir].pop();
                _exit(0); // Break from the loop
            }
            wait(NULL); // Waiting until the child finishes
            traffic[currentDir].pop(); // Dequeue the top from the traffic queue.
            currentPass++;
        }
        currentDir++; // Change the direction of the traffic
        currentDir %= 4;
        currentPass = 0;
    }
    return 0;
}

car::car() {
    licensePlate = "";
    lane = "";
    timeWait = 0;
}

void car::setAll(string a, string b, int c) {
    licensePlate = a;
    lane = b;
    timeWait = c;
}

void car::print() {
    cout << "Car " << licensePlate << " is using the intersection for ";
    cout << timeWait << " sec(s)."<< endl;
}

bool carsAreWaitingIn(queue<car> *intersection) {
    for (int i = 0; i < 4; i++) // Check each queue if they are empty
        if (!intersection[i].empty()) // Return false if any queue have at least one car
            return true;
    return false;
}

int directionToIndex(string x) {
    if (x == "N")
        return 0;
    else if (x == "E")
        return 1;
    else if (x == "S")
        return 2;
    else if (x == "W")
        return 3;
    else {
        cout << "Direction to index error: unrecognized direction. Input: " << x << endl;
        return -1;
    }
}

string indexToDirection(int x) {
    if (x == 0)
        return "Northbound";
    else if (x == 1)
        return "Eastbound";
    else if (x == 2)
        return "Southbound";
    else if (x == 3)
        return "Westbound";
    else {
        cout << "Direction to index error: unrecognized direction. Input: " << x << endl;
        return "error";
    }
}
