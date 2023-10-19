#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/neutrino.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cout;
using std::endl;
using std::cos;
using std::log;

// ��� ����������� ������
#define NAMED_MEMORY "/23/namedMemory"

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
	int signChIdP2;						// ID ������ �������� P1
	int tickSigusrP1;					// ����� ������� ����������� ������ ���� (�����������)

	pthread_mutexattr_t mutexAttr;	 	// ���������� ������ �������
	pthread_mutex_t mutex;			 	// ������ ������� � ����������� ������

	pthread_barrier_t startBarrier;		// ������ ������ ��������

	Clock timeInfo;						// ���������� � ������� ������� ���
};
// ---- ��������� ��� ����������� ������ ---- //

// ������������� ����������� ������
struct NamedMemory *connectToNamedMemory(const char* name);
// ���������� ������� ������� � ������� �������
double func(double t);
// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime);
// ��������� ������� ���������� ������
void deadHandler(int signo);

int main(int argc, char *argv[]) {
	cout << "P1: �������" << endl;

	struct NamedMemory *namedMemoryPtr = connectToNamedMemory(NAMED_MEMORY);
	cout << "P1: ������������� � ����������� ������" << endl;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, namedMemoryPtr->tickSigusrP1);

	timer_t stopTimer;
	struct itimerspec stopPeriod;
	setTimerStop(&stopTimer, &stopPeriod, namedMemoryPtr->timeInfo.endTime);

	cout << "P1: � �������" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P1: ������ ������" << endl;

	// ������ ������� ����������
	int res = timer_settime(stopTimer, 0, &stopPeriod, NULL);
	if(res == -1){
		cout << "P1: ������ ������� �������" << strerror(res)<< endl;
	}

	// �������� ���� �� ���� � ��� 180000000 ���� -> 0,18 ���
	const double tickSecDuration = namedMemoryPtr->timeInfo.durTickT / 1000000000.;
	cout << "P1: tickSecDuration - " << tickSecDuration << endl;

	while(true){
		// �������� �������
		int sig = SignalWaitinfo(&set, NULL);
		if(sig == namedMemoryPtr->tickSigusrP1){

			double time = namedMemoryPtr->timeInfo.countTickT * tickSecDuration;
			double value = func(time);

			pthread_mutex_lock(&(namedMemoryPtr->mutex));
			namedMemoryPtr->p = value;
			pthread_mutex_unlock(&(namedMemoryPtr->mutex));

			//cout << "P1: ��������" << " value: " << value << " time: " << time << " countTickT: " << namedMemoryPtr->timeInfo.countTickT  << endl;
		}

	}
	return EXIT_SUCCESS;
}

// ���������� ������� ������� � ������� �������
double func(double t) {
	const double b = 0.71;
	double top1 = b/(1 + t * t);
	double top = cos(top1);
	double bot = log(t + 4);
	double result = top / bot;
	return result;
}

// ������� ������������� � �������� ����������� ������
struct NamedMemory* connectToNamedMemory(const char* name) {
	struct NamedMemory *namedMemoryPtr;

	//���������� ����������� ������
	int fd = shm_open(name, O_RDWR, 0777);
	if(fd == -1){
		cout << "P1: ������ �������� ������� ����������� ������ - " << strerror(fd) <<  endl;
	    exit(EXIT_FAILURE);
	}

	namedMemoryPtr = (NamedMemory*) mmap(NULL, sizeof(struct NamedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(namedMemoryPtr == MAP_FAILED){
		cout << "P1: ������ ������������� ���. ���. ������������" <<  endl;
	    exit(EXIT_FAILURE);
	}

	return namedMemoryPtr;
}

// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime) {
	struct sigevent event;
	SIGEV_SIGNAL_INIT(&event, SIGUSR2);
	int res1 = timer_create(CLOCK_REALTIME, &event, stopTimer);
	if(res1 == -1){
		cout << "P1: ������ �������� ������� ��������� " << strerror(res1)<< endl;
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
		cout << "P1: ������ ������ ���������� ��������" <<  endl;
		exit(EXIT_SUCCESS);
	}
}
