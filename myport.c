#include "header.h"

int main(int argc, char *argv[]){

    FILE *fd_con, *fd_log, *fd_hist;
    Sh_Memory *shmem;
    char str_shmid[STR_SIZE];
    char configfile[STR_SIZE];
    char monitor_time[STR_SIZE], monitor_stattimes[STR_SIZE];
    char vessel_type[STR_SIZE], vessel_postype[STR_SIZE], vessel_parkperiod[STR_SIZE], vessel_mantime[STR_SIZE];
    char temp_type[STR_SIZE];
    float temp_cost;
    int total_berths, total_types, type_id, shmid, temp_ammount, status, i;
    int random_total_vessels, random_type, random_postype, random_parkperiod, random_mantime, random_sleep;
    pid_t pid;

    //getting the configuration file
    if(argc!=3){
        printf("Error: Wrong Input\n");
        exit(0);
    }
    else{
        if(strcmp(argv[1], "-l")==0)
            strcpy(configfile, argv[2]);
        else{
            printf("Error: Wrong Input\n");
            exit(0);
        }
    }

    //sorting the file base on charges
    pid = fork();
    if(pid==0){
        execlp("sort", "sort", "-g", "-k", "3", "-o", configfile, configfile, (char*)NULL);
        printf("Error: sort execlp execution\n");
        exit(0);
    }
    wait(&status);

    //getting total berths number
    fd_con = fopen(configfile, "r" );
    if(fd_con == NULL){
        printf("Error: Cannot open Configuration File\n");
        exit(0);
    }
    fscanf(fd_con, "%d", &total_berths);
    total_types = 0;
    while(fscanf(fd_con, "%s %d %f", temp_type,  &temp_ammount, &temp_cost) != EOF){
        total_types++;
    }

    //request a shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(Sh_Memory) + total_berths*sizeof(Public_Ledger) + total_types*sizeof(Stats), 0666);

    if(shmid == -1){
        perror("Error: Function shmget()\n");
        exit(0);
    }
    sprintf(str_shmid, "%d", shmid);

    //attach the shared memory
    shmem = (Sh_Memory *)shmat(shmid,0,0);
    if(*(int *)shmem == -1){
        printf("Error: Shared Memory Attachment\n");
        exit(0);
    }

    rewind(fd_con);
    fscanf(fd_con, "%d", &total_berths);
    //initialising the berth struct
    shmem->public_ledger = (Public_Ledger*)((char*)shmem+sizeof(Sh_Memory));
    temp_ammount = 0;
    for(i=0; i<total_berths; i++){
        shmem->public_ledger[i].taken = -1;
        shmem->public_ledger[i].vessel_id = -1;
        if(temp_ammount==0){
            fscanf(fd_con, "%s %d %f", temp_type,  &temp_ammount, &temp_cost);
        }
        strcpy(shmem->public_ledger[i].type, temp_type);
        shmem->public_ledger[i].cost = temp_cost;
        temp_ammount--;
    }
    fclose(fd_con);

    //initialising the type stuct
    shmem->type = (Type*)((char*)shmem+sizeof(Sh_Memory)+total_types*sizeof(Type));
    for(i=0; i<total_types; i++){
        shmem->type[i].total_waittime = 0;
        shmem->type[i].name[0] = '\0';
    }
    type_id = 0;
    temp_type[0]='\0';
    for(i=0; i<total_berths; i++){
        if(strcmp(temp_type, shmem->public_ledger[i].type)!=0){
            strcpy(temp_type, shmem->public_ledger[i].type);
            strcpy(shmem->type[type_id].name, shmem->public_ledger[i].type);
            type_id++;
        }
    }

    //initialising the semaphores
    if(sem_init(&(shmem->entry_queue), 1, 1)!=0)
        printf("Error: Cannot initialize semaphore\n");
    if(sem_init(&(shmem->exit_queue), 1, 1)!=0)
        printf("Error: Cannot initialize semaphore\n");
    if(sem_init(&(shmem->portmaster_sleep), 1, 0)!=0)
        printf("Error: Cannot initialize semaphore\n");
    if(sem_init(&(shmem->portmaster_busy), 1, 0)!=0)
        printf("Error: Cannot initialize semaphore\n");
    if(sem_init(&(shmem->entry_waiting), 1, 0)!=0)
        printf("Error: Cannot initialize semaphore\n");
    if(sem_init(&(shmem->exit_waiting), 1, 0)!=0)
        printf("Error: Cannot initialize semaphore\n");
    if(sem_init(&(shmem->finished), 1, 0)!=0)
        printf("Error: Cannot initialize semaphore\n");

    shmem->total_berths = total_berths;
    shmem->total_types = total_types;
    shmem->stats.total_vessels = 0;
    shmem->stats.total_waittime = 0;
    shmem->stats.total_income = 0;

    //creating the log file
    fd_log = fopen("log.txt", "w" );
    if(fd_log == NULL){
        printf("Error: Cannot open Configuration File\n");
        exit(0);
    }
    fclose(fd_log);

    //creating the history file
    fd_hist = fopen("history.txt", "w" );
    if(fd_hist == NULL){
        printf("Error: Cannot open Configuration File\n");
        exit(0);
    }
    fclose(fd_hist);

    //creating port master
    pid = fork();
    if(pid==0){
        execlp("./port-master", "port-master",  "-s", str_shmid, (char*)NULL);
        printf("Error: port-master execlp execution\n");
        exit(0);
    }
    sleep(1);
    printf("\nPort-Master generated successfully\n");

    //creating monitor
    pid = fork();
    if(pid==0){
        sprintf(monitor_time, "%d", 5);
        sprintf(monitor_stattimes, "%d", 8);
        execlp("./monitor", "monitor",  "-d", monitor_time, "-t", monitor_stattimes, "-s", str_shmid, (char*)NULL);
        printf("Error: monitor execlp execution\n");
        exit(0);
    }
    sleep(1);
    printf("\nMonitor generated successfully\n");

    //creating vessels
    srand(time(NULL));
    random_total_vessels = rand()%(VESSELS_MAX+1-VESSELS_MIN)+VESSELS_MIN;
    for(i=0; i<random_total_vessels; i++){
        random_type =  rand()%(total_types);
        random_postype = rand()%2;
        random_parkperiod = rand()%(PARKPERIOD_MAX+1-PARKPERIOD_MIN)+PARKPERIOD_MIN;
        random_mantime = rand()%(MANTIME_MAX+1-MANTIME_MIN)+MANTIME_MIN;
        random_sleep = rand()%(SLEEP_MAX+1-SLEEP_MIN)+SLEEP_MIN;
        pid = fork();
        if(pid==0){
            strcpy(vessel_type, shmem->type[random_type].name);
            if(random_postype)
                strcpy(vessel_postype, "yes");
            else
                strcpy(vessel_postype, "no");

            sprintf(vessel_parkperiod, "%d", random_parkperiod);
            sprintf(vessel_mantime, "%d", random_mantime);
            sleep(random_sleep);
            execlp("./vessel", "vessel", "-t", vessel_type, "-u", vessel_postype, "-p", vessel_parkperiod, "-m", vessel_mantime, "-s", str_shmid, (char*)NULL);
            printf("Error: vessel execlp execution\n");
            exit(0);
        }
    }

    sem_wait(&shmem->finished);

    //destroying the semaphores
    if(sem_destroy(&(shmem->entry_queue))!=0)
        printf("Error: Cannot destroy semaphore\n");
    if(sem_destroy(&(shmem->exit_queue))!=0)
        printf("Error: Cannot destroy semaphore\n");
    if(sem_destroy(&(shmem->portmaster_sleep))!=0)
        printf("Error: Cannot destroy semaphore\n");
    if(sem_destroy(&(shmem->portmaster_busy))!=0)
        printf("Error: Cannot destroy semaphore\n");
    if(sem_destroy(&(shmem->entry_waiting))!=0)
        printf("Error: Cannot destroy semaphore\n");
    if(sem_destroy(&(shmem->exit_waiting))!=0)
        printf("Error: Cannot destroy semaphore\n");
    if(sem_destroy(&(shmem->finished))!=0)
        printf("Error: Cannot destroy semaphore\n");

    shmdt(shmem);

    return 0;

}
