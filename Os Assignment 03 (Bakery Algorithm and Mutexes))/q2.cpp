#include <iostream>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <mutex>
#include <semaphore.h>
#include <condition_variable>
#include <chrono>


using namespace std;



class vehicle{
public:
    int vehicle_type; // unique identifier
    int direction; // 1 or 0

    vehicle(){
        vehicle_type =0;
        direction = -1;
    } // constructor
    ~vehicle(){} 
};


bool keep_checking_wait(mutex& key, int& total_weight, bool& status){
    key.lock();
    if(total_weight==4){
        status = true;
    }
    bool current_status = status;
    key.unlock();
    return current_status;
}

class Bridge {
public:
    int total_buses;
    int total_cars;
    int total_weight; // 1 for car, 2 for bus, max_total weight can be 4
    mutex key;
    condition_variable cv;

    Bridge() {
        total_buses = 0;
        total_cars = 0;
        total_weight = 0;
    }

    void Arrive_at_bridge(int direction, int id) { // id 0 car, id 1 bus
        unique_lock<mutex> lock(key);
        while (total_weight + (id == 0 ? 1 : 3) > 4) {
            cv.wait(lock);
        }
        if (id == 0) { // car
            total_cars++;
        } else if (id == 1) { // bus
            total_buses++;
        }
        total_weight += (id == 0 ? 1 : 3);
        cout << "Vehicle " << (id == 0 ? "CAR" : "BUS") << " arrived from direction " << direction << endl;
    }

    void Exit_the_bridge(int id) {
        unique_lock<mutex> lock(key);
        if (id == 0) { // car
            total_cars--;
        } else if (id == 1) { // bus
            total_buses--;
        }
        total_weight -= (id == 0 ? 1 : 3);
        cout << "Vehicle " << (id == 0 ? "CAR" : "BUS") << " exited the bridge" << endl;
        cv.notify_all();
    }
};



Bridge B; // global

void * vehicle_run(void * arg){
    vehicle * V = (vehicle *) arg;
    B.Arrive_at_bridge(V->direction,V->vehicle_type);

    // Use a timer to add a delay between Arrive_at_bridge() and Exit_the_bridge()
    usleep(1000000);

    B.Exit_the_bridge(V->vehicle_type);
}





void * spawn_vehicles(void * arg){
    Bridge * B = (Bridge *) arg;
    cout << "Enter The Duration You want the Program to run: ";
    int duration;
    cin >> duration;

    int seconds_time = duration * 60; // Convert minutes to seconds

    pthread_t thread_ids[200]; // keep track of all threads

    int vehicle_type=0;
    int direction=0;
    int current_thread=0;

    auto end_time = chrono::system_clock::now() + chrono::seconds(seconds_time);
    while(chrono::system_clock::now() < end_time){ // run loop till end of time
        int random = rand();
        if(random%2==0){
            vehicle_type = 0;
        }
        else{
            vehicle_type = 1;
        }

        random = rand()%200 +1;
        if(random%2==0){
            direction = 1;
        }
        else{
            direction = 0;
        }

        vehicle * V = new vehicle();
        V->vehicle_type = vehicle_type;
        V->direction = direction;
        usleep(100000);
        if(current_thread < 200) {
            pthread_create(&thread_ids[current_thread], NULL, vehicle_run, (void*) V);
            current_thread++;
        } 
        else {
            //pthread_join(thread_ids[current_thread - 200], NULL); // Wait for one thread to finish before creating a new one
            //pthread_create(&thread_ids[current_thread % 200], NULL, vehicle_run, (void*) V);
            break;
        }
        
    }

    // Wait for remaining threads to finish
    for(int i=0; i<current_thread%200; i++){
        pthread_join(thread_ids[i], NULL);
    }

    pthread_exit(NULL);
}




int main(){
    srand(time(NULL)); // seed time;



    pthread_t Master_thread;

    pthread_create(&Master_thread,NULL,spawn_vehicles,(void*)&B);

    pthread_join(Master_thread,NULL); // wait for master thread to finish

    exit(0);
}