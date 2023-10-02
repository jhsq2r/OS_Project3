#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

//Creator: Jarod Stagner
//Turn-in date:

#define SHMKEY 55555

static void myhandler(int s){
        printf("Killing all... exiting...\n");
        kill(0,SIGTERM);
}

static int setupinterrupt(void) {
                struct sigaction act;
                act.sa_handler = myhandler;
                act.sa_flags = 0;
                return (sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}
static int setupitimer(void) {
                struct itimerval value;
                value.it_interval.tv_sec = 60;
                value.it_interval.tv_usec = 0;
                value.it_value = value.it_interval;
                return (setitimer(ITIMER_PROF, &value, NULL));
}

struct PCB {
        int occupied;
        pid_t pid;
        int startSeconds;
        int startNano;
};

void displayTable(int i, struct PCB *processTable){//Make it also write to output file
        printf("Process Table:\nEntry Occupied PID StartS StartN\n");
        for (int x = 0; x < i; x++){

                printf("%d      %d      %d      %d      %d\n", x,processTable[x].occupied,processTable[x].pid,processTable[x].startSeconds,processTable[x].startNano);

        }
}

void updateTime(int *sharedTime){
        sharedTime[1] = sharedTime[1] + 100000000;
        if (sharedTime[1] >= 1000000000 ){
                sharedTime[0] = sharedTime[0] + 1;
                sharedTime[1] = sharedTime[1] - 1000000000;
        }
}

void help(){
        printf("Program usage\n-h = help\n-n [int] = Num Children to Launch\n-s [int] = Num of children allowed at once\n-t [int] = Max num of seconds for each child to be alive\n-f [filename] = name of file to write log to");
        printf("Default values are -n 5 -s 3 -t 3\nThis Program is designed to take in 4 inputs for Num Processes, Num of processes allowed at once,\nMax num of seconds for each process, and the name of a file to write the log to");
        printf("\nThis program requires a filename to be entered");
}

int main(int argc, char** argv) {

        if (setupinterrupt() == -1) {
                perror("Failed to set up handler for SIGPROF");
                return 1;
        }
        if (setupitimer() == -1) {
                perror("Failed to set up the ITIMER_PROF interval timer");
                return 1;
        }

        srand(time(NULL));
        int seed = rand();
        int proc = 5;
        int simul = 3;
        int maxTime = 3;//default parameters
        //string filename;

        int shmid = shmget(SHMKEY, sizeof(int)*2, 0777 | IPC_CREAT);
        if(shmid == -1){
                printf("Error in shmget\n");
                return EXIT_FAILURE;
        }
        int * sharedTime = (int *) (shmat (shmid, 0, 0));
        sharedTime[0] = 0;
        sharedTime[1] = 0;

        int option;
        while((option = getopt(argc, argv, "hn:s:t:f:")) != -1) {//Read command line arguments
                switch(option){
                        case 'h':
                                help();
                                return EXIT_FAILURE;
                                break;
                        case 'n':
                                proc = atoi(optarg);
                                break;
                        case 's':
                                simul = atoi(optarg);
                                break;
                        case 't':
                                maxTime = atoi(optarg);
                                break;
                        case 'f':
                                //filename = optarg;
                        case '?':
                                help();
                                return EXIT_FAILURE;
                                break;
                }
        }

        struct PCB processTable[20];
        int status;
        int i = 0;
        int next = 0;
        while(i < proc){
                seed++;
                srand(seed);
                int pid = waitpid(-1,&status,WNOHANG);
                //printf("i = %d\n", i);
                updateTime(sharedTime);
                if (sharedTime[1] == 500000000){
                        displayTable(i, processTable);
                }

                if((i >= simul && pid != 0) || i < simul){
                        for(int x = 0; x < (i+1); x++){
                                if(waitpid(processTable[x].pid, &status, WNOHANG) != 0){
                                        processTable[x].occupied = 0;
                                }
                        }
                        pid_t child_pid = fork();
                        if(child_pid == 0){
                                char convertSec[20];
                                char convertNan[20];

                                int randomSec = (rand() % ((maxTime - 1) -1 + 1)) + 1;
                                int randomNano = (rand() % (999999999 - 1 + 1)) + 1;

                                sprintf(convertSec, "%d", randomSec);
                                sprintf(convertNan, "%d", randomNano);

                                char *args[] = {"./worker", convertSec, convertNan, NULL};
                                execvp("./worker", args);

                                printf("Something horrible happened...\n");
                                exit(1);
                        }else{
                                processTable[i].occupied = 1;
                                processTable[i].pid = child_pid;
                                processTable[i].startSeconds = sharedTime[0];
                                processTable[i].startNano = sharedTime[1];
                        }
                        //while processTable[next].occupied == 0 next++ if next == 20 next = 0
                        //send message to next
                        //wait for response
                        //if terminating, output terminating to terminal and logfile
                        //next++
                        //if next == 20 next = 0
                        i++;
                }

        }

        i = 0;
        while(i < simul){
                updateTime(sharedTime);
                int pid = waitpid(-1,&status,WNOHANG);//wait for all children to die
                if (sharedTime[1] == 500000000){
                        displayTable(proc, processTable);
                }
                if(pid != 0){
                        for(int x = 0; x < (proc - simul + i + 1); x++){
                                if(waitpid(processTable[x].pid,&status,WNOHANG) != 0){
                                        processTable[x].occupied = 0;
                                }
                        }
                        //if i + 1 == simul do nothing
                        //else do as above
                        i++;
                }
        }

        displayTable(proc, processTable);

        shmdt(sharedTime);
        shmctl(shmid,IPC_RMID,NULL);
        //delete msgque

        printf("Done\n");

        return 0;
}

