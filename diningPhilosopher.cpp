// dining.cpp — minimal base (single file, C++ + Pthreads)
// Build: g++ -std=c++17 -O2 dining.cpp -lpthread -o dining

#include <pthread.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <unistd.h>

const int N = 5;

enum class State { Thinking, Hungry, Eating };

pthread_mutex_t cout_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  can_eat[N];
State state_of[N];

// eighbors philosophers
int left_neighbor(int i)  { return (i + N - 1) % N; }
int right_neighbor(int i) { return (i + 1) % N; }
// fork indices philosophers around the table
int right_fork(int i)     { return i; }
int left_fork(int i)      { return (i + 1) % N; }

// decides if philosopher i may start eating 
void check_can_eat(int i) {
    if (state_of[i] == State::Hungry &&
        state_of[left_neighbor(i)]  != State::Eating &&
        state_of[right_neighbor(i)] != State::Eating) {
        state_of[i] = State::Eating;
        pthread_cond_signal(&can_eat[i]);
    }
}

// called by a philsopher when it wants to eat
void pickup_forks(int i) {
    pthread_mutex_lock(&mtx);
    state_of[i] = State::Hungry;
    check_can_eat(i);
    while (state_of[i] != State::Eating) {
        pthread_cond_wait(&can_eat[i], &mtx);
    }
    
    std::cout << "Philosopher " << i << " has fork " << right_fork(i) << "\n";
    std::cout << "Philosopher " << i << " has fork " << left_fork(i) << "\n";
    std::cout << "Philosopher " << i << " started eating\n";
    pthread_mutex_unlock(&mtx);
}

// called by a philosopher when finished eating
void return_forks(int i) {
    pthread_mutex_lock(&mtx);
    state_of[i] = State::Thinking;
    std::cout << "Philosopher " << i << " returned their forks " << right_fork(i) 
              << " and " << left_fork(i) << "\n";
    check_can_eat(left_neighbor(i));
    check_can_eat(right_neighbor(i));
    pthread_mutex_unlock(&mtx);
}

// sims thinking
void think(int i) {
    float time_slept = (rand() % 3 + 1);
    std::cout << "Philosopher " << i << " started thinking" << std::endl;
    sleep(time_slept);
    std::cout << "Philosopher " << i << " thought for " << time_slept << " seconds\n";
}

// sims eating
void eat(int i) {
    float time_slept = (rand() % 3 + 1);
    sleep(time_slept);
    std::cout << "Philosopher " << i << " ate for " << time_slept << " seconds\n";
}

 // runs the sim
void* philosopher(void* arg) {
    int id = *(int*)arg;
    while (true) {
        think(id);
        pickup_forks(id);
        eat(id);
        return_forks(id);
    }
    return nullptr;
}

int main() {

    // thread creation
    srand(time(nullptr));
    pthread_t threads[N];
    int ids[N] = {0, 1, 2, 3, 4};

    for (int i = 0; i < N; i++) {
        pthread_cond_init(&can_eat[i], nullptr);
        state_of[i] = State::Thinking;
    }

    for (int i = 0; i < N; i++) {
        pthread_create(&threads[i], nullptr, philosopher, &ids[i]);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], nullptr);
    }

    return 0;
}