#include <cstdlib>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cout;
using std::endl;
using std::FILE;

// ��� ����������� ������
#define NAMED_MEMORY "/16/namedMemory"
// ���� ��� ������ �������
#define TREND_FILE "/home/host/lab3Trend/trend.txt"

// ---- ��������� ��� ����������� ������ ---- //
// ��������� ������ � ����������� � ������� ������� ����������
struct Clock {
	long durTickT; 		// ������������ ������ ���� � ������������
	long durTickDt; 	// ������������ ������ ���� � ������������
	int countTickDt;	// ����� �������� ���� ����� ���
	int countTickT;		// ����� �������� ���� ����� ���
	long endTime;		// ������������ ������ ���������� � ��������
};
// ��������� ������, ���������� � ����������� ������ NAMED_MEMORY
struct NamedMemory {
	double p;							// ����������� ��������

	int pidP2; 							// ID �������� P2
	int pidP1; 							// ID �������� P1
	int signChIdP1;						// ID ������ �������� P1
	int tickSigusrP2;					// ����� ������� ����������� ������ ���� (�����������)

	pthread_mutexattr_t mutexAttr;	 	// ���������� ������ �������
	pthread_mutex_t mutex;			 	// ������ ������� � ����������� ������

	pthread_barrier_t startBarrier;		// ������ ������ ��������

	Clock timeInfo;						// ���������� � ������� ������� ���
};
// ---- ��������� ��� ����������� ������ ---- //

// ����������� ������� ����������
void viewWorkDirectory();
// ������������� ����������� ������
struct NamedMemory *connectToNamedMemory(const char* name);
// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime);
// ��������� ������� ���������� ������
void deadHandler(int signo);

FILE* trendFile;

int main(int argc, char *argv[]) {
	cout << "P2: �������" << endl;
	viewWorkDirectory();

	struct NamedMemory *namedMemoryPtr = connectToNamedMemory(NAMED_MEMORY);

	trendFile = fopen(TREND_FILE, "w");
	if(trendFile == NULL){
		cout << "P2: ������ �������� ����� ��� ����� ������" << endl;
		exit(EXIT_FAILURE);
	}
	cout << "�2: ������ ���� ������ trend.txt" << endl;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, namedMemoryPtr->tickSigusrP2);

	timer_t stopTimer;
	struct itimerspec stopPeriod;
	setTimerStop(&stopTimer, &stopPeriod, namedMemoryPtr->timeInfo.endTime);

	cout << "P2: � �������" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P2: ������ ������" << endl;

	// ������ ������� ����������
	int res = timer_settime(stopTimer, 0, &stopPeriod, NULL);
	if(res == -1){
		cout << "P2: ������ ������� �������" << strerror(res)<< endl;
	}

	// �������� ���� �� ���� � ��� 300000000 ���� -> 0.3 ���
	const double tickSecDuration = namedMemoryPtr->timeInfo.durTickDt / 1000000000.;

	while(true){
		int sig = SignalWaitinfo(&set, NULL);
		if(sig == namedMemoryPtr->tickSigusrP2){

			pthread_mutex_lock(&(namedMemoryPtr->mutex));
			double value = namedMemoryPtr->p;
			pthread_mutex_unlock(&(namedMemoryPtr->mutex));

			double time = namedMemoryPtr->timeInfo.countTickDt * tickSecDuration;

			//cout << "P2: ���������� " << "value: " << value << " time: " << time << " countTickDt: " << namedMemoryPtr->timeInfo.countTickDt  << endl;
			fprintf(trendFile, "%f\t%f\n", value, time);
		}

	}

	return EXIT_SUCCESS;
}

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P2: ������� ������� ���������� - " << cCurrentPath << endl;
}

// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime) {
	struct sigevent event;
	SIGEV_SIGNAL_INIT(&event, SIGUSR2);
	int res1 = timer_create(CLOCK_REALTIME, &event, stopTimer);
	if(res1 == -1){
		cout << "P2: ������ �������� ������� ��������� " << strerror(res1)<< endl;
	}
	stopPeriod->it_value.tv_sec = endTime;
	stopPeriod->it_value.tv_nsec = 0;
	stopPeriod->it_interval.tv_sec = 0;
	stopPeriod->it_interval.tv_nsec = 0;

   	struct sigaction act;
   	sigset_t set;
   	sigemptyset(&set);
   	sigaddset(&set, SIGUSR2);
   	act.sa_flags = 0;
   	act.sa_mask = set;
   	act.__sa_un._sa_handler = &deadHandler;
   	sigaction(SIGUSR2, &act, NULL);
}

// ��������� ������� ���������� ������
void deadHandler(int signo) {
	if (signo == SIGUSR2) {
		cout << "P2: ������ ������ ���������� ��������" <<  endl;
		fclose(trendFile);
		exit(EXIT_SUCCESS);
	}
}

// ������� ������������� � �������� ����������� ������
struct NamedMemory* connectToNamedMemory(const char* name) {
	struct NamedMemory *namedMemoryPtr;

	//���������� ����������� ������
	int fd = shm_open(name, O_RDWR, 0777);
	if(fd == -1){
		cout << "P2: ������ �������� ������� ����������� ������ - " << strerror(fd) <<  endl;
	    exit(EXIT_FAILURE);
	}

	namedMemoryPtr = (NamedMemory*) mmap(NULL, sizeof(struct NamedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(namedMemoryPtr == MAP_FAILED){
		cout << "P2: ������ ������������� ���. ���. ������������" <<  endl;
	    exit(EXIT_FAILURE);
	}

	return namedMemoryPtr;
}
