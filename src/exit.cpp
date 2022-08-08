#include<iostream>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<string>
#include<vector>
using namespace std;

struct proc{
public:
	int arrival_time, cpu_burst, blocked_times, waiting_time, turnaround_time, size;
	bool blocked, check;

	proc():blocked_times(0), waiting_time(0), turnaround_time(0), blocked(false), check(false){
	}
};

char newLine[1] = {'\n'};	//For writing new lines
char tab[1] = {'\t'};		//For writing tabs

void writeToFile(int write_fd, string temp) {	//Func to write to file
	char tempc[100];
	int i;

	for (i = 0; i < temp.size(); i++) {
		tempc[i] = temp[i];
	}
	
	write(write_fd, tempc, i);
}

void writeResult(int throughput, float avgWT, float avgTT, int write_fd) {	//Func to write output into file
	char* temp = "Throughput: ";
	write(write_fd, temp, 12);
	writeToFile(write_fd, to_string(throughput));
	write(write_fd, newLine, sizeof(newLine));
	write(write_fd, tab, sizeof(tab));
	temp = "Average Waiting Time: ";
	write(write_fd, temp, 22);
	writeToFile(write_fd, to_string(avgWT));
	write(write_fd, newLine, sizeof(newLine));
	write(write_fd, tab, sizeof(tab));
	temp = "Average Turnaround Time: ";
	write(write_fd, temp, 25);
	writeToFile(write_fd, to_string(avgTT));
}

int main(int argc, char *argv[]) {
	int pipefd = stoi(argv[1]);
	float avgWT = 0, avgTT = 0;
	int total_procs, procs_count = 0, write_fd, ticks = 0, throughput;
	bool cumulative;
	proc temp;
	vector<proc> v;

	read(pipefd, &total_procs, sizeof(total_procs));
	write_fd = open ("processes_stats.txt", O_RDWR);

	while(procs_count != total_procs) {
		read(pipefd, &cumulative, sizeof(cumulative));
		read(pipefd, &temp, sizeof(temp));

		if(temp.blocked == true) {
			temp.cpu_burst += (5 * temp.blocked_times);
		}

		temp.waiting_time -= temp.arrival_time;		//Setting arrival time
		temp.turnaround_time = temp.waiting_time + temp.cpu_burst;		//Calculating TurnaroundTime
		
		//Displaying Gantt Chart of the process
		cout << "------------------Process Completed------------------" << endl << "		  Arrival Time: " << temp.arrival_time << endl << "	 	  Cpu Burst: " << temp.cpu_burst << endl;
		cout << "		  Waiting Time: " << temp.waiting_time << endl << "		  Turnaround Time: " << temp.turnaround_time << endl << endl;
		v.insert(v.end(), temp);
		procs_count++;

		if (cumulative == true) {	//Writing cumulative sum after every 30 ticks
			avgWT = 0;
			avgTT = 0;
			for (int i = 0; i < v.size(); i++) {
				avgWT += v[i].waiting_time;
				avgTT += v[i].turnaround_time;
			}
			avgWT /= float(v.size());
			avgTT /= float(v.size());
			throughput = v.size();
		
			char* temp;

			ticks += 30;
			temp = "After First ";
			write(write_fd, temp, 12);
			writeToFile(write_fd, to_string(ticks));
			temp = " Ticks: ";
			write(write_fd, temp, 7);
			write(write_fd, newLine, sizeof(newLine));
			write(write_fd, tab, sizeof(tab));
			writeResult(throughput, avgWT, avgTT, write_fd);
			write(write_fd, newLine, sizeof(newLine));
			write(write_fd, newLine, sizeof(newLine));
		}
	}

	//Writing final result when all processes are done
	avgWT = 0;
	avgTT = 0;
	for (int i = 0; i < v.size(); i++) {
		avgWT += v[i].waiting_time;
		avgTT += v[i].turnaround_time;
	}
	avgWT /= float(v.size());
	avgTT /= float(v.size());
	throughput = v.size();
	
	char* tempc;

	ticks += 30;
	tempc = "Final Result: ";
	write(write_fd, tempc, 14);
	write(write_fd, newLine, sizeof(newLine));
	write(write_fd, tab, sizeof(tab));
	writeResult(throughput, avgWT, avgTT, write_fd);

	cout << "----------------All Processes Completed----------------" << endl;
}