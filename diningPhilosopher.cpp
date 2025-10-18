// dining.cpp — minimal base (single file, C++ + Pthreads)
// Build: g++ -std=c++17 -O2 diningPhilosopher.cpp -lpthread -o dining

#include <pthread.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <unistd.h>

const int N = 5;
const int ROUNDS = 5; //fixed amount of rounds 

enum class State { Thinking, Hungry, Eating };

pthread_mutex_t cout_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  can_eat[N];
State state_of[N];

// prints one line for each philosopher
void println(const std::string& s){
    pthread_mutex_lock(&cout_mtx);
    std::cout << s << '\n';
    pthread_mutex_unlock(&cout_mtx);
}

// neighbors philosophers
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
    
    println("Philosopher " + std::to_string(i) + " has fork " + std::to_string(right_fork(i)));
    println("Philosopher " + std::to_string(i) + " has fork " + std::to_string(left_fork(i)));
    println("Philosopher " + std::to_string(i) + " started eating");
    pthread_mutex_unlock(&mtx);
}

// called by a philosopher when finished eating
void return_forks(int i) {
    pthread_mutex_lock(&mtx);
    state_of[i] = State::Thinking;
    println("Philosopher " + std::to_string(i) + " returned their forks " +
        std::to_string(right_fork(i)) + " and " + std::to_string(left_fork(i)));
    check_can_eat(left_neighbor(i));
    check_can_eat(right_neighbor(i));
    pthread_mutex_unlock(&mtx);
}

// sims thinking
void think(int i) {
    int time_slept = rand() % 3 + 1;   
    println("Philosopher " + std::to_string(i) + " started thinking");
    sleep(time_slept);
    println("Philosopher " + std::to_string(i) + " thought for " + std::to_string(time_slept) + " seconds");

}

// sims eating
void eat(int i) {
    int time_slept = rand() % 3 + 1;  
    sleep(time_slept);
    println("Philosopher " + std::to_string(i) + " ate for " + std::to_string(time_slept) + " seconds");

}

 // runs the sim
void* philosopher(void* arg) {
    int id = *(int*)arg;
    for(int r = 0; r < ROUNDS; ++r){
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
