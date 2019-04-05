#ifndef HEADER
#define HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define STR_SIZE 30
#define VESSELS_MIN 5
#define VESSELS_MAX 15
#define PARKPERIOD_MIN 10
#define PARKPERIOD_MAX 25
#define MANTIME_MIN 2
#define MANTIME_MAX 5
#define SLEEP_MIN 0
#define SLEEP_MAX 5

typedef struct Public_Ledger{
    char type[STR_SIZE];
    float cost;
    int taken; //-1: free | 0: taken but the vessel has not parked yet | 1: taken
    int vessel_id;
    char vessel_type[STR_SIZE];
    int vessel_mantime;
    int vessel_parktime;
    int vessel_parkperiod;
} Public_Ledger;

typedef struct Type{
    int total_waittime;
    char name[STR_SIZE];
} Type;

typedef struct Stats{
    int total_vessels;
    int total_waittime;
    float total_income;
} Stats;

typedef struct Sh_Memory{

    Public_Ledger* public_ledger;
    Type* type;
    Stats stats;

    sem_t portmaster_sleep;
    sem_t portmaster_busy;
    sem_t entry_queue;
    sem_t exit_queue;
    sem_t entry_waiting;
    sem_t exit_waiting;
    sem_t finished;

    int total_berths;
    int total_types;
    int starttime;

    int entry_request;
    int entry_id;
    int entry_mantime;
    int entry_parkperiod;
    int entry_answer;

    int exit_request;
    int exit_time;
    int exit_id;

    char requested_type[2][STR_SIZE];

} Sh_Memory;

//global functions
void add_waittime(char type[STR_SIZE], int waittime, int shmid);

float get_cost(int vessel_id, int time, int shmid);

void add_log(char str[STR_SIZE*3], int shmid);

int get_time(int shmid);

//port-master functions
int empty_port(int shmid);

void add_hist(int vessel_id, float cost, int parktime, int exittime);

int available_berth(int shmid);

void add_berth(int berth_id, int shmid);

void remove_berth(int shmid);

#endif
