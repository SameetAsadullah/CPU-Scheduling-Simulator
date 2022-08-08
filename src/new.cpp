#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<string>
using namespace std;

struct proc{
public:
	int arrival_time, cpu_burst, blocked_times, waiting_time, turnaround_time, size;
	bool blocked, check;

	proc():blocked_times(0), waiting_time(0), turnaround_time(0), blocked(false), check(false){
	}
};

int main(int argc, char *argv[]) {
	int total_procs = 0;
	char str[1000];
	char scheduling_process[6];
	bool AT = true, check = false;
	int quantum;
	pid_t pid;
	int pipefd, fd, num;

	cout << "=> Press 1 to do RR" << endl << "=> Press 2 to do FCFS" << endl << "=> Press 3 to do SJF" << endl;
	cin >> num;

	cout << endl;

	if (num == 1) {
		fd = open("Sample_1_RR.txt", O_RDONLY);
	}

	else if (num == 2) {
		fd = open("Sample_2_FCFS.txt", O_RDONLY);
	}

	else if (num == 3) {
		fd = open("Sample_4_SJF.txt", O_RDONLY);
	}

	read(fd, str, sizeof(str));	//Reading from file to string

	mkfifo("MYPIPE", 0666);	//Pipe to send procs to ready state
	pipefd = open("MYPIPE", O_RDWR);

	scheduling_process[0] = '\0';

	for (int i = 0; str[i] != '\0'; i++) {	//Fetching data of procs from string
		static string temp;
		static proc temp_proc;

		if (str[i] != '\n') {
			temp += str[i];
		}

		if (str[i] == '\n' || str[i + 1] == '\0'){
			if (scheduling_process[0] == '\0') {
				for (int i = 0 ; temp[i] != '\0' ; i++) {
					scheduling_process[i] = temp[i];
				}
				write(pipefd, &scheduling_process, sizeof(scheduling_process));
			}
			
			else {
				if (scheduling_process[0] == 'R' && scheduling_process[1] == 'R' && check == false){
					quantum = stoi(temp);
					check = true;
					write(pipefd, &quantum, sizeof(quantum));		
				}

				else {		
					if (temp[0] == 'P') {
						temp = "";
					}

					else {
						if (AT == true) {
							temp_proc.arrival_time = stoi(temp);
							AT = false;
						}

						else {
							temp_proc.cpu_burst = stoi(temp);
							AT = true;
							write(pipefd, &temp_proc, sizeof(temp_proc));	//Writing into pipe
							total_procs++;
						}
					}
				}
			}
			
			temp = "";
		}
	}

	pid = fork();

	if (pid > 0) {
		wait(NULL);
	}

	else if (pid == 0) {
		int copy_pipefd = dup(pipefd);	//Using dup for coping file descriptors
		string temps = to_string(copy_pipefd);
		char *tempc = new char[temps.size() + 1];
		temps.copy(tempc, temps.size() + 1);	//Copying value of file descriptor into char* to pass through exec
		tempc[temps.size()] = '\0';

		execlp("./ready", "ready", tempc, NULL);	//Calling exec of ready state
	}

	else{
		cout << "Fork Failed" << endl;
	}
}