#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>

#define SHMKEY 55555
#define PERMS 0644
typedef struct msgbuffer {
        long mtype;
        //char strData[100];
        int intData;
} msgbuffer;

int main(int argc, char** argv){

        int shmid = shmget(SHMKEY, sizeof(int)*2, 0777);
        if(shmid == -1){
                printf("Error in shmget\n");
                return EXIT_FAILURE;
        }
        int * sharedTime = (int*) (shmat (shmid, 0, 0));

        //printf("This is Child: %d, From Parent: %d, Seconds: %s, NanoSeconds: %s\n", getpid(), getppid(), argv[1], argv[2]);

        int exitTime[2];
        exitTime[0] = atoi(argv[1]) + sharedTime[0];
        exitTime[1] = atoi(argv[2]) + sharedTime[1];
        if (exitTime[1] >= 1000000000){
                exitTime[1] -= 1000000000;
                exitTime[0] += 1;
        }

        //printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d JUST STARTING\n",getpid(),getppid(),sharedTime[0],sharedTime[1],exitTime[0],exitTime[1]);

        int currentTime = sharedTime[0];
        int secondsPassed = 0;
        //while (1)
        //wait for message
        while (exitTime[0] > sharedTime[0] || exitTime[1] > sharedTime[1]){// make if
                if(sharedTime[0] - exitTime[0] >= 2){
                        //send message back that terminating
                        break;
                }
                if(currentTime != sharedTime[0]){
                        secondsPassed += sharedTime[0]-currentTime;
                        printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d, %d SECONDS HAVE PASSED\n",getpid(),getppid(),sharedTime[0],sharedTime[1],exitTime[0],exitTime[1],secondsPassed);
                        currentTime = sharedTime[0];
                }
                //send message saying not terminating
        }//else
        //send message back that terminating
        //break

        //printf("WORKER PID:%d PPID:%d SysClockS: %d SysclockNano: %d TermTimeS: %d TermTimeNano: %d TERMINATING\n",getpid(),getppid(),sharedTime[0],sharedTime[1],exitTime[0],exitTime[1]);

        shmdt(sharedTime);

        return 0;

}
