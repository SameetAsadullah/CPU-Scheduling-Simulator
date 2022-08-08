#include<iostream>
#include<unistd.h>
using namespace std;

int main() {
	execlp("./new", "new", NULL);	//Starting the simualator by calling exec to new state
}