#include "header.h"

int main(int argc, char *argv[]){

    Sh_Memory *shmem;
    struct timespec ts;
    char temp_str[STR_SIZE*3];;
    int shmid, berth_id, bored;

    if(argc!=3){
        printf("Error: Wrong Input\n");
        exit(0);
    }
    else{
        if(strcmp(argv[1], "-s")==0)
            shmid = atoi(argv[2]);
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
    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));
    shmem->starttime = time(NULL);
    sprintf(temp_str, "The Port opened!\n");
    add_log(temp_str, shmid);
    while(1){
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec +=SLEEP_MAX*2;

        //the Port-Master is waiting for a job
        if(empty_port(shmid)){
            bored = sem_timedwait(&shmem->portmaster_sleep, &ts);
            if(bored!=0){
                sprintf(temp_str, "This Port has no jobs anymore, bon voyage...\n");
                add_log(temp_str, shmid);
                sem_post(&shmem->finished);
                return 0;
            }
        }
        else
            sem_wait(&shmem->portmaster_sleep);

        //the Port-Master informing vessel about its request
        if(shmem->entry_request){
            berth_id = available_berth(shmid);
            shmem->entry_request = 0;
            if(berth_id!=-1){
                shmem->entry_answer = 1;
                //the Port-Master is waiting for the vessel to finish its job
                sem_post(&shmem->entry_waiting);
                sem_wait(&shmem->portmaster_busy);
                add_berth(berth_id, shmid);
                sprintf(temp_str, "Vessel #%d parked in the berth #%d of type %s.\n", shmem->public_ledger[berth_id].vessel_id, berth_id, shmem->public_ledger[berth_id].type);
                add_log(temp_str, shmid);
                sem_post(&shmem->entry_queue);
            }
            else{
                shmem->entry_answer = 0;
                sem_post(&shmem->entry_waiting);
            }
        }

        if(shmem->exit_request){
            shmem->exit_request = 0;
            //the Port-Master is waiting for the vessel to finish its job
            sem_post(&shmem->exit_waiting);
            sem_wait(&shmem->portmaster_busy);
            remove_berth(shmid);
            sem_post(&shmem->exit_queue);
        }
    }

    return 0;
}
