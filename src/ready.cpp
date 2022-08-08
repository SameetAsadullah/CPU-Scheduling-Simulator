#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
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

bool compareAT(proc p1, proc p2) {	//Function used to sort according to the arrival time
	return (p1.arrival_time < p2.arrival_time);
}

bool compareBurst(proc p1, proc p2) {	//Function used to sort according to the cpu burst
	return (p1.cpu_burst < p2.cpu_burst);
}

int total_procs, quantum, pipefd, pipefd1, pipefd2, pipefd3;
queue<proc> procs;
proc temp, temp1;

void FCFS() {	//Implementing FCFS
	bool end = false, check = false, blocked_processes = false;
	vector<int> v;

	while(!procs.empty() || check == false) {
		int block_status = 0;

		write(pipefd, &end, sizeof(end));

		if(!procs.empty()) {
			proc temp = procs.front();
			procs.pop();

			if (temp.blocked == true) {	//Calculating waiting time
				for (int i = (v.size()-temp.size); i<v.size(); i++) {
					temp.waiting_time += v[i];
				}
			}

			if (blocked_processes == true && temp.cpu_burst > 5 && !procs.empty()) {	//Helping in calculating waiting time
				v.insert(v.end(), 5);
				temp.check = true;
			}

			else if (blocked_processes == true && !procs.empty()) {	//Helping in calculating waiting time
				v.insert(v.end(), temp.cpu_burst);
			}

			write(pipefd, &temp, sizeof(temp));
		}

		while(block_status == 0) {	//Waiting for completion of process in running state
			sleep(1);
			temp.arrival_time = -1;
			temp.cpu_burst = -1;

			read(pipefd1, &block_status, sizeof(block_status));
			read(pipefd2, &check, sizeof(check));
			read(pipefd2, &temp, sizeof(temp));
		
			if (temp.arrival_time != -1 && temp.cpu_burst != -1) {		//For processes coming from blocked state
				temp.size = procs.size();
				blocked_processes = true;
				temp.check = false;
				procs.push(temp);
			}
		}
	}
	end = true;
	write(pipefd, &end, sizeof(end));
}

void RR() {	//Implementing RR
	bool end = false, check = false, blocked_processes = false;
	vector<int> v;

	write(pipefd, &quantum, sizeof(quantum));

	while(!procs.empty() || check == false) {
		int block_status = 0;

		write(pipefd, &end, sizeof(end));

		if(!procs.empty()) {
			proc temp = procs.front();
			procs.pop();

			if (temp.blocked == true) {		//Calculating waiting time
				for (int i = (v.size()-temp.size); i<v.size(); i++) {
					temp.waiting_time += v[i];
				}
			}

			if (blocked_processes == true && temp.cpu_burst > quantum && !procs.empty()) {	//Helping in calculating waiting time
				v.insert(v.end(), quantum);
				temp.check = true;
			}

			else if (blocked_processes == true && !procs.empty()) {	//Helping in calculating waiting time
				v.insert(v.end(), temp.cpu_burst);
			}

			write(pipefd, &temp, sizeof(temp));
		}

		while(block_status == 0) {	//Waiting for completion of process in running state
			sleep(1);
			temp.arrival_time = -1;
			temp.cpu_burst = -1;
			temp1.arrival_time = -1;
			temp1.cpu_burst = -1;

			read(pipefd1, &block_status, sizeof(block_status));
			read(pipefd2, &check, sizeof(check));
			read(pipefd2, &temp, sizeof(temp));
			read(pipefd3, &temp1, sizeof(temp1));

			if (temp.arrival_time != -1 && temp.cpu_burst != -1) {		//For processes coming from blocked state
				temp.size = procs.size();
				blocked_processes = true;
				temp.check = false;
				procs.push(temp);
			}

			if (temp1.arrival_time != -1 && temp1.cpu_burst != -1) {	//For processes coming from running state
				temp1.size = procs.size();
				blocked_processes = true;
				temp1.check = false;
				procs.push(temp1);
			}
		}
	}
	end = true;
	write(pipefd, &end, sizeof(end));
}

struct myComparator {	//Used to create heap
public: 
    int operator() (const proc& p1, const proc& p2) {
        return p1.arrival_time > p2.arrival_time; 
    } 
}; 

void SJF() {	//Implementing SJF
	priority_queue <proc, vector<proc>, myComparator> pq;	//Creating heap
	bool end = false, check = false, blocked_processes = false, first = true;
	vector<int> v;
	vector<proc> v1;
	proc firstTimeSend;
	int burst;

	while (!procs.empty()) {
		pq.push(procs.front());
		procs.pop();
	}

	firstTimeSend = pq.top();
	pq.pop();

	while(!pq.empty() || check == false) {
		int block_status = 0;

		write(pipefd, &end, sizeof(end));

		if (first == true) {	//Checking for first process
			if (pq.top().arrival_time == firstTimeSend.arrival_time) {
				v1.insert(v1.end(), firstTimeSend);
			
				while(pq.top().arrival_time == firstTimeSend.arrival_time) {
					v1.insert(v1.end(), pq.top());
					pq.pop();
				}

				sort(v1.begin(), v1.end(), compareBurst);

				write(pipefd, &v1[0], sizeof(proc));		
				burst = v1[0].cpu_burst;

				for (int i = 1; i<v1.size(); i++) {
					pq.push(v1[i]);
				}
			}

			else {
				burst = firstTimeSend.cpu_burst;
				write(pipefd, &firstTimeSend, sizeof(proc));
			}

			v1.clear();
			first = false;
		}

		else {
			if(!pq.empty()) {
				while (pq.top().arrival_time <= burst && !pq.empty()) {
					v1.insert(v1.end(), pq.top());
					pq.pop();
				}

				sort(v1.begin(), v1.end(), compareBurst);

				proc temp = v1[0];
				burst += v1[0].cpu_burst;
				
				for (int i = 1; i<v1.size(); i++) {
					pq.push(v1[i]);
				}

				v1.clear();

				if (temp.blocked == true) {
					for (int i = (v.size()-temp.size); i<v.size(); i++) {	//Calculating waiting time
						temp.waiting_time += v[i];
					}
				}

				if (blocked_processes == true && temp.cpu_burst > 5 && !procs.empty()) {	//Helping in calculating waiting time
					v.insert(v.end(), 5);
					temp.check = true;
				}

				else if (blocked_processes == true && !procs.empty()) {		//Helping in calculating waiting time
					v.insert(v.end(), temp.cpu_burst);
				}

				write(pipefd, &temp, sizeof(temp));
			}
		}

		while(block_status == 0) {		//Waiting for completion of process in running state
			sleep(1);
			temp.arrival_time = -1;
			temp.cpu_burst = -1;

			read(pipefd1, &block_status, sizeof(block_status));
			read(pipefd2, &check, sizeof(check));
			read(pipefd2, &temp, sizeof(temp));
		
			if (temp.arrival_time != -1 && temp.cpu_burst != -1) {
				temp.size = procs.size();
				blocked_processes = true;
				temp.check = false;
				pq.push(temp);
			}
		}
	}
	end = true;
	write(pipefd, &end, sizeof(end));
}

int main(int argc, char* argv[]) {
	fcntl(stoi(argv[1]), F_SETFL, O_NONBLOCK);

	vector<proc> tempv;
	char scheduling_process[6];

	pipefd = stoi(argv[1]);

	read(pipefd, &scheduling_process, sizeof(scheduling_process));
	
	if (scheduling_process[0] == 'R' && scheduling_process[1] == 'R') {
		read(pipefd, &quantum, sizeof(quantum));
	}
	
	while(temp.arrival_time != -1 && temp.cpu_burst != -1) {
		temp.arrival_time = -1;
		temp.cpu_burst = -1;

		read(pipefd, &temp, sizeof(temp));

		if (temp.arrival_time != -1 && temp.cpu_burst != -1) {
			tempv.insert(tempv.end(), temp);
		}
	}

	total_procs = tempv.size();

	sort(tempv.begin(), tempv.end(), compareAT);

	for (int i = 0 ; i < total_procs ; i++) {
		procs.push(tempv[i]);
	}

	write(pipefd, &total_procs, sizeof(total_procs));
	write(pipefd, &scheduling_process, sizeof(scheduling_process));

	mkfifo("MYPIPE1", 0666);	//Pipe to communicate between ready and running
	pipefd1 = open("MYPIPE1", O_RDWR);
	mkfifo("MYPIPE4", 0666);	//Pipe to communicate between ready and blocked
	pipefd2 = open("MYPIPE4", O_RDWR);

	if (scheduling_process[0] == 'R' && scheduling_process[1] == 'R') {	//Pipe specifically for RR to communicate with running state when quantum is over
		mkfifo("MYPIPE6", 0666);
		pipefd3 = open("MYPIPE6", O_RDWR);
		fcntl(pipefd3, F_SETFL, O_NONBLOCK);
	}

	fcntl(pipefd1, F_SETFL, O_NONBLOCK);
	fcntl(pipefd2, F_SETFL, O_NONBLOCK);

	pid_t pid = fork();

	if (pid > 0) {
		if (scheduling_process[0] == 'F' && scheduling_process[1] == 'C' && scheduling_process[2] == 'F' && scheduling_process[3] == 'S') {	//For FCFS
				FCFS();
		}

		else if (scheduling_process[0] == 'R' && scheduling_process[1] == 'R') {	//FOR RR
			RR();
		}

		else if (scheduling_process[0] == 'S' && scheduling_process[1] == 'J' && scheduling_process[2] == 'F') {	//For SJF
			SJF();
		}
	}

	else if (pid == 0) {
		int copy_pipefd1 = dup(pipefd1);	//Using dup to copy file descriptor
		string temps = to_string(copy_pipefd1);
		char *tempc = new char[temps.size() + 1];	//Converting file descriptor into char* to pass through exec
		temps.copy(tempc, temps.size() + 1);
		tempc[temps.size()] = '\0';

		copy_pipefd1 = dup(pipefd2);	//Using dup to copy file descriptor
		temps = to_string(copy_pipefd1);
		char *tempc1 = new char[temps.size() + 1];	//Converting file descriptor into char* to pass through exec
		temps.copy(tempc1, temps.size() + 1);
		tempc1[temps.size()] = '\0';

		if (scheduling_process[0] == 'R' && scheduling_process[1] == 'R') {	//Specifically for RR to pass file descriptor of one extra pipe
			copy_pipefd1 = dup(pipefd3);	//Using dup to copy file descriptor
			temps = to_string(copy_pipefd1);
			char *tempc2 = new char[temps.size() + 1];	//Coverting file descriptor into char* to pass through exec
			temps.copy(tempc2, temps.size() + 1);
			tempc2[temps.size()] = '\0';

			execlp("./running", "running", argv[1], tempc, tempc1, tempc2, NULL);	//Exec to running state in case of RR
		}

		else {
			execlp("./running", "running", argv[1], tempc, tempc1, NULL);	//Exec to running state in other cases
		}
	}

	else {
		cout << "Fork Failed" << endl;
	}
}