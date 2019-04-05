#include "header.h"

int main(int argc, char *argv[]){

    char vessel_type[STR_SIZE], vessel_postype[STR_SIZE], temp_str[STR_SIZE*3];
    int vessel_id, vessel_parkperiod, vessel_mantime;
    int shmid;
    time_t vessel_waittime;
    Sh_Memory *shmem;

    //checking the input
    if(argc!=11){
        printf("Error:pointer Wrong Input\n");
        exit(0);
    }
    else{
        if(strcmp(argv[1], "-t")==0)
            strcpy(vessel_type, argv[2]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
        if(strcmp(argv[3], "-u")==0)
            strcpy(vessel_postype, argv[4]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
        if(strcmp(argv[5], "-p")==0)
            vessel_parkperiod = atoi(argv[6]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
        if(strcmp(argv[7], "-m")==0)
            vessel_mantime = atoi(argv[8]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
        if(strcmp(argv[9], "-s")==0)
            shmid = atoi(argv[10]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
    }

    vessel_id = getpid();

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    vessel_waittime = time(NULL);
    sprintf(temp_str, "Vessel #%d waiting in queue.\n", vessel_id);
    add_log(temp_str, shmid);
    sem_wait(&shmem->entry_queue);
    do{
        sleep(1);

        //the vessel is requesting for mooring
        strcpy(shmem->requested_type[0], vessel_type);
        strcpy(shmem->requested_type[1], vessel_postype);
        shmem->entry_request = 1;

        //the vessel is waiting the port-master's answer
        sem_post(&shmem->portmaster_sleep);
        sem_wait(&shmem->entry_waiting);

        //if the answer was positive
        if(shmem->entry_answer){
            add_waittime(vessel_type, time(NULL)-vessel_waittime, shmid);

            //the vessel entered the port for mooring
            sleep(vessel_mantime);

            //the vessel just arrived in its berth

            //the vessel is sending its info to the Port-Master
            shmem->entry_id = vessel_id;
            shmem->entry_mantime = vessel_mantime;
            shmem->entry_parkperiod = vessel_parkperiod;
            sem_post(&shmem->portmaster_busy);
        }
    }while(shmem->entry_answer==0);

    //the vessel after some time wants to leave

    //informing the vessel for its half-time cost (needed)
    sleep(vessel_parkperiod/2);
    sprintf(temp_str, "Vessel #%d has half-time cost %.2f units.\n", vessel_id, get_cost(vessel_id, vessel_parkperiod/2, shmid));
    add_log(temp_str, shmid);
    sleep(vessel_parkperiod/2);

    //the vessel is requesting for departure
    sem_wait(&shmem->exit_queue);

    shmem->exit_request = 1;
    sem_post(&shmem->portmaster_sleep);
    sem_post(&shmem->portmaster_sleep);

    //the vessel is waiting the port-master's answer
    sem_wait(&shmem->exit_waiting);

    //the vessel is sending its info to the Port-Master
    shmem->exit_id = vessel_id;

    //the vessel is quiting the berth for departure
    sleep(vessel_mantime);

    sprintf(temp_str, "Vessel #%d just exited.\n", vessel_id);
    add_log(temp_str, shmid);

    sem_post(&shmem->portmaster_busy);

    return 0;

}
