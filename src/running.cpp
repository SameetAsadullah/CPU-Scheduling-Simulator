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
#include<queue>
#include<vector>
#include<bits/stdc++.h>
using namespace std;

struct proc{
public:
	int arrival_time, cpu_burst, blocked_times, waiting_time, turnaround_time, size;
	bool blocked, check;

	proc():blocked_times(0), waiting_time(0), turnaround_time(0), blocked(false), check(false){
	}
};

int pipefd, pipefd1, pipefd2, pipefd3, pipefd4, pipefd5, total_procs;

void FCFS() {	//Implementing FCFS and also using this for SJF
	queue<proc> blocked_procs;
	int ticks = 0, block_status = 0, procs_count = 0, counter = 0;
	proc temp;
	bool end  = false, timer = true, check = true;
	vector<int>v;

	while(end == false){
		temp.arrival_time = -1;
		temp.cpu_burst = -1;
		
		read(pipefd, &end, sizeof(end));
		read(pipefd, &temp, sizeof(temp));
		
		if (temp.arrival_time != -1 && temp.cpu_burst != -1) {
			cout << "Process => Arrival Time: " << temp.arrival_time << ", CPU Burst: " << temp.cpu_burst << " ";
			if (temp.blocked == false) {		//Calculating waiting time
				cout << endl;
				temp.waiting_time += counter;
			}

			else if (temp.blocked == true) {		//Calculating waiting time
				cout << "(CAME FROM BLOCKED STATE TO EXECUTE REMAINING CPU BURST)" << endl;
				for (int i = (v.size()-temp.size); i<v.size(); i++) {
					temp.waiting_time += v[i];
				}
			}

			if (counter < temp.arrival_time) {		//Checking arrival time of processes
				timer = false;
			}
			
			if (timer == false) {			//Matching arrival time of processes
				cout << "Sleeping for " << temp.arrival_time - counter << " to match arrival time"  << endl; 
				sleep(temp.arrival_time - counter);
				ticks += temp.arrival_time - counter;
				timer = true;
			}

			if (temp.cpu_burst <= 5) {		//First 5 seconds of process
				cout << "Sleeping to execute " << temp.cpu_burst << " cpu burst" << endl;
				sleep(temp.cpu_burst);
				ticks += temp.cpu_burst;
				counter += temp.cpu_burst;
				procs_count++;

				if (ticks >= 30) {
					bool temp = true;
					write(pipefd4, &temp, sizeof(temp));
					ticks = 0;
				}

				else {
					bool temp = false;
					write(pipefd4, &temp, sizeof(temp));
				}
				write(pipefd4, &temp, sizeof(temp));
			}

			else {		//Checking blocked status if cpu burst is more than 5
				cout << "Sleeping to execute 5 cpu burst" << endl;
				sleep(5);
				ticks += temp.cpu_burst;
				counter += 5;
				temp.cpu_burst -= 5;
				block_status = rand() % 2;
				cout << "Block Status of process: " << block_status << endl;
		
				if (block_status == 1) {		//Executing remaining cpu burst
					cout << "Sleeping to execute remaining " << temp.cpu_burst << " cpu burst" << endl;
					sleep(temp.cpu_burst);
					counter += temp.cpu_burst;
					ticks += temp.cpu_burst;

					if (temp.check == true) {
						v.insert(v.end(), temp.cpu_burst);
					}

					if (ticks >= 30) {
						bool temp = true;
						write(pipefd4, &temp, sizeof(temp));
						ticks = 0;
					}

					else {
						bool temp = false;
						write(pipefd4, &temp, sizeof(temp));
					}

					temp.cpu_burst += 5;
					write(pipefd4, &temp, sizeof(temp));
					procs_count++;
				}

				else {		//Going into blocked state
					cout << "PROCESS WENT TO BLOCKED STATE" << endl << endl;
					if (temp.check == true) {
						v.insert(v.end(), 0);
					}

					temp.blocked_times++;
					temp.blocked = true;
					blocked_procs.push(temp);
				}
			}
		}

		if (procs_count == total_procs) {	//Sending signal to blocked state to stop
			bool temp = true;
			write(pipefd2, &temp, sizeof(temp));
		}

		read(pipefd3, &check, sizeof(check));
		if (check == true && !blocked_procs.empty()) {	//Sending process to blocked state
			check = false;
			
			if (procs_count != total_procs) {
				write(pipefd2, &check, sizeof(check));		
			}

			write(pipefd2, &blocked_procs.front(), sizeof(proc));
			blocked_procs.pop();
		}

		block_status = 1;
		write(pipefd1, &block_status, sizeof(block_status));
		sleep(1);
		ticks += 1;
	}
}

void RR() {
	queue<proc> blocked_procs;
	int ticks = 0, block_status = 0, procs_count = 0, counter = 0, quantum;
	proc temp;
	bool end  = false, timer = true, check = true;
	vector<int>v;

	read(pipefd, &quantum, sizeof(quantum));

	while(end == false){
		temp.arrival_time = -1;
		temp.cpu_burst = -1;
		
		read(pipefd, &end, sizeof(end));
		read(pipefd, &temp, sizeof(temp));
		
		if (temp.arrival_time != -1 && temp.cpu_burst != -1) {
			cout << "Process => Arrival Time: " << temp.arrival_time << ", CPU Burst: " << temp.cpu_burst << " ";
			if (temp.blocked == false) {
				cout << endl;
			}

			else {
				cout << "(CAME FROM BLOCKED OR READY STATE TO EXECUTE REMAINING CPU BURST)" << endl;
			}

			if (counter < temp.arrival_time) {		//Checking arrival time of processes
				timer = false;
			}
			
			if (timer == false) {		//Matching arrival time of processes
				cout << "Sleeping for " << temp.arrival_time - counter << " to match arrival time"  << endl;
				sleep(temp.arrival_time - counter);
				ticks += temp.arrival_time - counter;
				counter += temp.arrival_time - counter;
				timer = true;
			}

			if (temp.blocked == false) {	//Calculating waiting time
				temp.waiting_time += counter;
			}

			if (temp.cpu_burst <= quantum) {	//Executing cpu burst if it is less than quantum
				cout << "Sleeping to execute " << temp.cpu_burst << " cpu burst" << endl;
				sleep(temp.cpu_burst);
				ticks += temp.cpu_burst;
				counter += temp.cpu_burst;
				procs_count++;

				if (ticks >= 30) {
					bool temp = true;
					write(pipefd4, &temp, sizeof(temp));
					ticks = 0;
				}

				else {
					bool temp = false;
					write(pipefd4, &temp, sizeof(temp));
				}
				write(pipefd4, &temp, sizeof(temp));
			}

			else {		//Checking blocked status of process
				cout << "Sleeping to execute " << quantum << " cpu burst" << endl;
				sleep(quantum);
				ticks += quantum;
				counter += quantum;
				temp.cpu_burst -= quantum;
				block_status = rand() % 2;
				temp.blocked = true;
				cout << "Block Status of process: " << block_status << endl;

				if (block_status == 1) {	//Quantum is over so sending process back to ready state
					cout << "PROCESS WENT BACK TO READY STATE BECAUSE QUANTUM " << quantum << " IS OVER" << endl << endl;
					temp.blocked_times++;
					write(pipefd5, &temp, sizeof(temp));
				}

				else {		//Sending process to blocked state
					cout << "PROCESS WENT TO BLOCKED STATE" << endl << endl;
					temp.blocked_times++;
					blocked_procs.push(temp);
				}
			}
		}

		if (procs_count == total_procs) {		//Sending signal to blocked state to stop
			bool temp = true;
			write(pipefd2, &temp, sizeof(temp));
		}

		read(pipefd3, &check, sizeof(check));
		if (check == true && !blocked_procs.empty()) {		//Sending process to blocked state
			check = false;
			
			if (procs_count != total_procs) {
				write(pipefd2, &check, sizeof(check));		
			}

			write(pipefd2, &blocked_procs.front(), sizeof(proc));
			blocked_procs.pop();
		}

		block_status = 1;
		write(pipefd1, &block_status, sizeof(block_status));
		sleep(1);
		ticks += 1;
	}
}

int main(int argc, char *argv[]) {
	srand(time(NULL));

	char scheduling_process[6];

	pipefd = stoi(argv[1]);
	pipefd1 = stoi(argv[2]);

	read(pipefd, &total_procs, sizeof(total_procs));
	read(pipefd, &scheduling_process, sizeof(scheduling_process));

	mkfifo("MYPIPE2", 0666);		//Creating pipe for communication between running and blocked state
	pipefd2 = open("MYPIPE2", O_RDWR);
	mkfifo("MYPIPE3", 0666);		//Creating pipe for communication between running and blocked state
	pipefd3 = open("MYPIPE3", O_RDWR);
	fcntl(pipefd3, F_SETFL, O_NONBLOCK);
	mkfifo("MYPIPE5", 0666);		//Creating pipe for communication between running and exit state
	pipefd4 = open("MYPIPE5", O_RDWR);

	write(pipefd2, &total_procs, sizeof(total_procs));
	write(pipefd4, &total_procs, sizeof(total_procs));

	pid_t pid = fork();

	if (pid > 0) {
		pid = fork();

		if(pid > 0) {
			if (scheduling_process[0] == 'F' && scheduling_process[1] == 'C' && scheduling_process[2] == 'F' && scheduling_process[3] == 'S') {	//For FCFS
				FCFS();
			}

			else if (scheduling_process[0] == 'R' && scheduling_process[1] == 'R') {	//For RR
				pipefd5 = stoi(argv[4]);	//Specifically for RR
				RR();
			}

			else if (scheduling_process[0] == 'S' && scheduling_process[1] == 'J' && scheduling_process[2] == 'F') {	//For SJF
				FCFS();
			}
		}

		else if (pid == 0) {
			int copy_pipefd2 = dup(pipefd4);	//Using dup to copy file descriptor
			string temps = to_string(copy_pipefd2);
			char *tempc = new char[temps.size() + 1];	//Converting file descriptor into char* to pass through exec
			temps.copy(tempc, temps.size() + 1);
			tempc[temps.size()] = '\0';

			execlp("./exit", "exit", tempc, NULL);	//Exec to exit state
		}

		else {
			cout << "Fork Failed" << endl;
		}
	}

	else if (pid == 0) {
		int copy_pipefd2 = dup(pipefd2);	//Using dup to copy file descriptor
		string temps = to_string(copy_pipefd2);
		char *tempc = new char[temps.size() + 1];	//Converting file descriptor into char* to pass through exec
		temps.copy(tempc, temps.size() + 1);
		tempc[temps.size()] = '\0';

		copy_pipefd2 = dup(pipefd3);	//Using dup to copy file descriptor
		temps = to_string(copy_pipefd2);
		char *tempc1 = new char[temps.size() + 1];	//Converting file descriptor into char* to pass through exec
		temps.copy(tempc1, temps.size() + 1);
		tempc1[temps.size()] = '\0';

		execlp("./blocked", "blocked", tempc, tempc1, argv[3], NULL);	//Exec to blocked state
	}	

	else {
		cout << "Fork Failed" << endl;
	}
}