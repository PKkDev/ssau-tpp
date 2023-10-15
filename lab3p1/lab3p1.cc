#include <cstdlib>
#include <iostream>

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <errno.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cout;
using std::endl;

// ������������ ���� 1t (����������� ��������� - 0,05 c) (�����������)
#define DUR_TICK_T 50000000
// ������������ ���� dt (����������� �������� - 0,3 �) (�����������)
#define DUR_TICK_DT 300000000
// ����� ������ ���������� (���)
#define END_TIME 5 //107 5
// ����� ������� ����������� ������ ���� (�����������)
#define TICK_SIGUSR_P2 SIGUSR1
// ��� ����������� ������
#define NAMED_MEMORY "/16/namedMemory"

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
// ��������� ������� ���������� ������
void deadHandler(int signo);
// ���������� P0 ��������� � ���������� � ����������� ������
void sendReadyMessageToP0(char *chIdP0Str);
// ���������� ������� ������� � ������� �������
double func(double t);
// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime);
// �������� ����������� ������
NamedMemory *createNamedMemory(const char* name);

// ��������� ����������� ������
struct NamedMemory *namedMemoryPtr;

int main(int argc, char *argv[]) {
	cout << "P1: �������" << endl;
	viewWorkDirectory();
	cout << "P1: ���������: " << "argv[0]=  " << argv[0] <<  endl;

	// ������������� ����������� ������
	namedMemoryPtr = createNamedMemory(NAMED_MEMORY);

	// ��������� ���������� ������� ����������
	namedMemoryPtr->timeInfo.durTickT = DUR_TICK_T;
	namedMemoryPtr->timeInfo.durTickDt = DUR_TICK_DT;
	namedMemoryPtr->timeInfo.endTime = END_TIME;
	// ��������� ����� ��� � �����, -1 - ���� �� ��������
	namedMemoryPtr->timeInfo.countTickT = -1;
	namedMemoryPtr->timeInfo.countTickDt = -1;

	namedMemoryPtr->tickSigusrP2 = TICK_SIGUSR_P2;

	// ������ ��� ������������� ������ �������� � ���������
	pthread_barrierattr_t startAttr;
	pthread_barrierattr_init(&startAttr);
	pthread_barrierattr_setpshared(&startAttr, PTHREAD_PROCESS_SHARED);
	pthread_barrier_init(&(namedMemoryPtr->startBarrier), &startAttr, 5);

	// ������������� ���������� ������ ������������ �������
	int r1 = pthread_mutexattr_init(&namedMemoryPtr->mutexAttr);
	if(r1 != EOK){
		cout << "�1: ������ pthread_mutexattr_init: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}
	 // ���������� � ���������� ������ ������� �������� "�����������"
	int r2 = pthread_mutexattr_setpshared(&namedMemoryPtr->mutexAttr, PTHREAD_PROCESS_SHARED);
	if(r2 != EOK){
		cout << "�1: ������ pthread_mutexattr_setpshared: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}
	// ������������� ������������ �������
	int r3 = pthread_mutex_init(&namedMemoryPtr->mutex, &namedMemoryPtr->mutexAttr);
	if(r3 != EOK){
		cout << "�1: ������ pthread_mutex_init: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}

	//�������� ������
	int signChIdP1 = ChannelCreate(_NTO_CHF_SENDER_LEN);
	namedMemoryPtr->signChIdP1 = signChIdP1;

	sendReadyMessageToP0(argv[0]);

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

	// �������� ���� �� ���� � ��� 50000000 ���� -> 0.05 ���
	const double tickSecDuration = namedMemoryPtr->timeInfo.durTickT / 1000000000.;

	while(true){
		MsgReceivePulse(signChIdP1, NULL, 0, NULL);

		double time = namedMemoryPtr->timeInfo.countTickT * tickSecDuration;
		double value = func(time);

		pthread_mutex_lock(&(namedMemoryPtr->mutex));
		namedMemoryPtr->p = value;
		pthread_mutex_unlock(&(namedMemoryPtr->mutex));

		//cout << "P1: ��������" << " value: " << value << " time: " << time << " countTickT: " << namedMemoryPtr->timeInfo.countTickT  << endl;
	}

	return EXIT_SUCCESS;
}

// �������� ����������� ������
NamedMemory *createNamedMemory(const char* name){
	struct NamedMemory *namedMemoryPtr;

	//���������� ����������� ������
	int fd = shm_open(name, O_RDWR | O_CREAT, 0777);
	if(fd == -1){
		cout << "P1: ������ ��������/�������� ������� ����������� ������ - " << strerror(fd) <<  endl;
	    exit(EXIT_FAILURE);
	}

	int tr1 = ftruncate(fd, 0);
	int tr2 = ftruncate(fd, sizeof(struct NamedMemory));
	if(tr1 == -1 || tr2 == -1){
		cout << "P1: ������ ftruncate" <<  endl;
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

// ���������� ������� ������� � ������� �������
double func(double t) {
	const double a = 1.9;
	const double b = 1.1;
	double result = (b * pow(t, 2))/(pow(M_E, a * t) + t);
	return result;
}

// ���������� P0 ��������� � ���������� � ����������� ������
void sendReadyMessageToP0(char *chIdP0Str){
	char rmsg[20];
	int chIdP0 = atoi(chIdP0Str);
	cout << "P1: ������������ ���������� � ������� P0" <<  endl;
	int coidP0 = ConnectAttach(0, getppid(), chIdP0, _NTO_SIDE_CHANNEL, 0);
	if(coidP0 == -1){
		cout << "P1: ������ ���������� � ������� P0 - " << strerror(coidP0) <<  endl;
	    exit(EXIT_FAILURE);
	}
	cout << "�1: ������� ��������� �0" <<  endl;
	char *smsg1 = (char *)"P1";
	int sendRes = MsgSend(coidP0, smsg1, strlen(smsg1) + 1, rmsg, sizeof(rmsg));
	if(sendRes == -1){
		cout << "P1: ������ MsgSend ��� �������� � P0 - " << strerror(sendRes) <<  endl;
		exit(EXIT_FAILURE);
	}
}

// ��������� ������� ���������� ������
void deadHandler(int signo) {
	if (signo == SIGUSR2) {
		cout << "P1: ������ ������ ���������� ��������" <<  endl;
		pthread_barrier_destroy(&(namedMemoryPtr->startBarrier));
		exit(EXIT_SUCCESS);
	}
}

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P1: ������� ������� ���������� - " << cCurrentPath << endl;
}























