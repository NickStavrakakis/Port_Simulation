#include "header.h"

int empty_port(int shmid){

    Sh_Memory *shmem;
    int i;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    for(i=0; i<shmem->total_berths; i++){
        if(shmem->public_ledger[i].taken != -1)
            return 0;
    }

    return 1;

}

void add_hist(int vessel_id, float cost, int parktime, int exittime){

    FILE *fd_hist;
    char temp_str[STR_SIZE*3];

    fd_hist = fopen("history.txt", "a" );
    if(fd_hist == NULL){
        printf("Error: Cannot open Configuration File\n");
        exit(0);
    }
    sprintf(temp_str, "Vessel #%d arrived at %d seconds, paid %.2f units and left at %d seconds\n", vessel_id, parktime, cost, exittime);
    fprintf(fd_hist, "%s", temp_str);
    fclose(fd_hist);
    return;
}

int available_berth(int shmid){

    Sh_Memory *shmem;
    char new_type[STR_SIZE];
    int i;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }
    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));

    for(i=0; i<shmem->total_berths; i++){
        if(strcmp(shmem->public_ledger[i].type, shmem->requested_type[0])==0 && shmem->public_ledger[i].taken==-1){
            shmem->public_ledger[i].taken = 0;
            strcpy(shmem->public_ledger[i].vessel_type, shmem->requested_type[0]);
            return i;
        }
    }
    //finding the pos //EXPLAIN LATER
    for(i=0; i<shmem->total_berths; i++){
        if(strcmp(shmem->public_ledger[i].type, shmem->requested_type[0])==0)
            break;
    }
    strcpy(new_type, shmem->requested_type[0]);
    if(strcmp(shmem->requested_type[1], "yes")==0){
        for(; i<shmem->total_berths; i++)
            if(shmem->public_ledger[i].taken == -1){
                shmem->public_ledger[i].taken = 0;
                strcpy(shmem->public_ledger[i].vessel_type, shmem->requested_type[0]);
                return i;
            }
    }

    return -1;

}

void add_berth(int berth_id, int shmid){

    Sh_Memory *shmem;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }
    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));
    shmem->public_ledger[berth_id].taken = 1;
    shmem->public_ledger[berth_id].vessel_id = shmem->entry_id;
    shmem->public_ledger[berth_id].vessel_parktime = get_time(shmid);
    shmem->public_ledger[berth_id].vessel_mantime = shmem->entry_mantime;
    shmem->public_ledger[berth_id].vessel_parkperiod = shmem->entry_parkperiod;
    shmem->stats.total_vessels++;

    return;
}

void remove_berth(int shmid){

    Sh_Memory *shmem;
    char temp_str[STR_SIZE*3];
    float cost;
    int i, exittime;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }
    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));

    for(i=0; i<shmem->total_berths; i++){
        if(shmem->public_ledger[i].vessel_id == shmem->exit_id){
            exittime = get_time(shmid)-shmem->public_ledger[i].vessel_mantime;
            cost = get_cost(shmem->public_ledger[i].vessel_id, exittime-shmem->public_ledger[i].vessel_parktime, shmid);
            sprintf(temp_str, "Vessel #%d owns to the Port-Master %.2f units.\n", shmem->public_ledger[i].vessel_id, cost);
            add_log(temp_str, shmid);
            shmem->stats.total_income = shmem->stats.total_income + cost;
            add_hist(shmem->exit_id, cost, shmem->public_ledger[i].vessel_parktime, exittime);
            shmem->public_ledger[i].taken = -1;
            shmem->public_ledger[i].vessel_id = -1;
            shmem->public_ledger[i].vessel_type[0] = '\0';
            shmem->public_ledger[i].vessel_parktime = -1;
            shmem->public_ledger[i].vessel_parkperiod = -1;

            return;
        }
    }
}
