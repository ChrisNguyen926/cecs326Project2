// dining.cpp — minimal base (single file, C++ + Pthreads)
// Build: g++ -std=c++17 -O2 dining.cpp -lpthread -o dining

#include <pthread.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

const int N = 5;

enum class State { Thinking, Hungry, Eating };

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
}
// called by a philosopher when finished eating
void return_forks(int i) {
    pthread_mutex_lock(&mtx);
    state_of[i] = State::Thinking;
    check_can_eat(left_neighbor(i));
    check_can_eat(right_neighbor(i));
    pthread_mutex_unlock(&mtx);
}

