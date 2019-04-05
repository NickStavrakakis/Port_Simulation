#include "header.h"

void print_status(int shmid){

    Sh_Memory *shmem;
    int i;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));

    printf("\n---------------------STATUS---------------------\n");
    for(i=0; i<shmem->total_berths; i++){
        printf("Port #%d: --> ", i);
        printf("Type: %s", shmem->public_ledger[i].type);
        printf(" | Cost: %.02f", shmem->public_ledger[i].cost);
        printf(" | Taken: %d", shmem->public_ledger[i].taken);
        if(shmem->public_ledger[i].taken==1){
            printf(" | Vessel Id: %d", shmem->public_ledger[i].vessel_id);
            printf(" | Vessel Type: %s", shmem->public_ledger[i].vessel_type);
            printf(" | Vessel Stay Time: %d", get_time(shmid)-shmem->public_ledger[i].vessel_parktime);
            printf(" | Vessel Total Park Time: %d", shmem->public_ledger[i].vessel_parkperiod);
        }
        printf("\n");
    }
    printf("------------------------------------------------\n");

    return;
}

void print_stats(int shmid){

    Sh_Memory *shmem;
    int i;

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    shmem->type = (Type*)((char*)shmem+sizeof(Sh_Memory)+shmem->total_types*sizeof(Type));

    printf("\n-------------------STATISTICS-------------------\n");

    if(shmem->stats.total_vessels)
        printf("Average Wait Time: %d seconds\n", shmem->stats.total_waittime/shmem->stats.total_vessels);
    else
        printf("Average Wait Time: 0 seconds\n");

    for(i=0; i<shmem->total_types; i++)
        printf("Total Wait Time for type %s: %d seconds\n", shmem->type[i].name, shmem->type[i].total_waittime);
    if(shmem->stats.total_vessels)
        printf("Average Income: %.2f units\n", shmem->stats.total_income/shmem->stats.total_vessels);
    else
        printf("Average Income: 0 units\n");

    printf("Total Income: %.2f units\n", shmem->stats.total_income);
    printf("------------------------------------------------\n");

    return;
}

int main(int argc, char *argv[]){

    Sh_Memory *shmem;
    int monitor_time, monitor_stattimes, flag;
    int temp1 = 1; //monitor_time
    int temp2 = 1; //monitor_stattimes
    int shmid;

    //checking the input
    if(argc!=7){
        printf("Error: Wrong Input\n");
        exit(0);
    }
    else{
        if(strcmp(argv[1], "-d")==0)
            monitor_time = atoi(argv[2]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
        if(strcmp(argv[3], "-t")==0)
            monitor_stattimes = atoi(argv[4]);
        else{
            printf("Errdor: Wrong Input\n");
            exit(0);
        }
        if(strcmp(argv[5], "-s")==0)
            shmid = atoi(argv[6]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
    }

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    do{
        if(time(NULL)==shmem->starttime+monitor_time*temp1){
            print_status(shmid);
            temp1++;
        }

        if(time(NULL)==shmem->starttime+monitor_stattimes*temp2){
            print_stats(shmid);
            temp2++;
        }

        sem_getvalue(&shmem->finished, &flag);
    }while(flag==0);

    return 0;
}
