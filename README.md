This program is designed to take in 3 integers and 1 file name in this format:
-n [int] -s [int] -t [int] -f [filename]
-n is the number of processes to launch
-s is the number of processes to run at once
-t is the max number of seconds a process can run
-f is the name of the file to output the log
Default Values: -n 5 -s 3 -t 3
THERE MUST BE A FILENAME ENTERED FOR THIS PROGRAM TO WORK
-n can not be more than 20
Example: ./oss -n 15 -s 3 -t 4 -f log.txt
This program will use its own internal clock and message queues to know when to launch child processes and will end when all processes have completed or when 60 real life seconds have passed. Every half second the program will output a list of PCBs that will show the pid, occupied flag, and start time of each process
