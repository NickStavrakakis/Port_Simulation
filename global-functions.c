#include "header.h"

void add_waittime(char type[STR_SIZE], int waittime, int shmid){

    Sh_Memory *shmem;
    int i;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }
    shmem->type = (Type*)((char*)shmem+sizeof(Sh_Memory)+shmem->total_types*sizeof(Type));

    for(i=0; i<shmem->total_types; i++){
        if(strcmp(shmem->type[i].name, type)==0){
            shmem->type[i].total_waittime = shmem->type[i].total_waittime + waittime;
            shmem->stats.total_waittime = shmem->stats.total_waittime + waittime;
            break;
        }
    }

    return;

}

float get_cost(int vessel_id, int time, int shmid){

    Sh_Memory *shmem;
    int i;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }
    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));

    for(i=0; i<shmem->total_berths; i++){
        if(shmem->public_ledger[i].vessel_id == vessel_id)
            return shmem->public_ledger[i].cost*time;
    }

    return -1;
}

void add_log(char str[STR_SIZE*3], int shmid){

    FILE *fd_log;
    Sh_Memory *shmem;
    int seconds, minutes;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    fd_log = fopen("log.txt", "a" );
    if(fd_log == NULL){
        printf("Error: Cannot open Configuration File\n");
        exit(0);
    }
    minutes = (time(NULL)-shmem->starttime)/60;
    seconds = (time(NULL)-shmem->starttime)%60;

    if(minutes<10)
        fprintf(fd_log, "0%d", minutes);
    else
        fprintf(fd_log, "%d", minutes);
    if(seconds<10)
        fprintf(fd_log, ":0%d | ", seconds);
    else
        fprintf(fd_log, ":%d | ", seconds);
    fprintf(fd_log, "%s", str);

    fclose(fd_log);
    return;

}

int get_time(int shmid){
    Sh_Memory *shmem;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    return time(NULL)-shmem->starttime;
}
