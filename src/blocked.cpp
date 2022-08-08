#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<string>
#include<cstdlib>
#include<time.h>
#include<bits/stdc++.h>
using namespace std;

struct proc{
public:
	int arrival_time, cpu_burst, blocked_times, waiting_time, turnaround_time, size;
	bool blocked, check;

	proc():blocked_times(0), waiting_time(0), turnaround_time(0), blocked(false), check(false){
	}
};

int main(int argc, char *argv[]) {
	srand(time(NULL));

	int pipefd = stoi(argv[1]), pipefd1 = stoi(argv[2]), pipefd2 = stoi(argv[3]);
	int total_procs, timer;
	bool end = false, check = true;
	proc temp;
	queue<proc> blocked_procs;

	read(pipefd, &total_procs, sizeof(total_procs));
	
	while (end != true) {
		read(pipefd, &end, sizeof(end));
		if (end == 1) {		//Signal to end blocked state because all processes are done
			write(pipefd2, &end, sizeof(end));
			break;
		}

		read(pipefd, &temp, sizeof(temp));	//Reading
		blocked_procs.push(temp);		//Pushing into queue
		temp = blocked_procs.front();
		blocked_procs.pop();
		timer = (rand() % 11) + 15;		//Generating random ticks
		sleep(timer);		//Sleeping for ticks
		temp.waiting_time += timer;

		write(pipefd2, &end, sizeof(end));	//Sending signal to ready state
		write(pipefd2, &temp, sizeof(temp));	//Sending process to ready state
		write(pipefd1, &check, sizeof(check));	//Sending signal to running state
	}
}