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

using namespace std;

int count = 0;

class Bakery_Lock {
private:
    vector<int> ticket_number;
    vector<bool> flag;

public:
    Bakery_Lock(int number_of_threads) { // Constructor
        ticket_number.resize(number_of_threads);
        for (int i = 0; i < number_of_threads; i++) {
            ticket_number[i] = 0; // set all values of the vector to 0
        }

        flag.resize(number_of_threads);
        for (int i = 0; i < number_of_threads; i++) {
            flag[i] = false; // set all values of the vector to false
        }
    }

    ~Bakery_Lock() { // destructor
    }

    int getmax() {
        int max = -999;
        for (int i = 0; i < ticket_number.size(); i++) {
            if (ticket_number[i] > max) {
                max = ticket_number[i];
            }
        }
        return max;
    }

    //----------------------------------
    void lock(int thread_number) {
        flag[thread_number] = true; // set the flag of current thread to true, indicating that we want to access
        // critical section
        ticket_number[thread_number] = getmax() + 1;

        for (int other_tid = 0; other_tid < flag.size(); ++other_tid) {
            if (other_tid == thread_number) continue; // dont check for our thread_number
            while (ticket_number[other_tid] != 0 && flag[other_tid] && (ticket_number[other_tid] < ticket_number[thread_number] || (ticket_number[other_tid] == ticket_number[thread_number] && other_tid < thread_number))) {
                // Wait until it's our turn
            }
        }
    }

    void unlock(int thread_number) {
        ticket_number[thread_number] = 0;
        flag[thread_number] = false; // set to false indicating that we are leaving the critical section
    }
};


Bakery_Lock B(4);

void* increment(void* arg) {
    int id = *(int*)arg;
    for (int i = 0; i < 10; ++i) {
        B.lock(id);
        ++count;
        cout << "Thread " << id << ": " << count << std::endl;
        B.unlock(id);
    }
    pthread_exit(NULL);
}


int main() {
    pthread_t thread_ids[4];

    for (int i = 0; i < 4; i++) {
        int* tid = new int(i);
        pthread_create(&thread_ids[i], NULL, increment, (void*)tid);
    }

    pthread_join(thread_ids[0], NULL);
    pthread_join(thread_ids[1], NULL);
    pthread_join(thread_ids[2], NULL);
    pthread_join(thread_ids[3], NULL);

  	cout << "Final count: " << count << std::endl;
    exit(0);
}
