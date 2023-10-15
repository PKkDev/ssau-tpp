#include <cstdlib>
#include <iostream>

#include <sys/neutrino.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cout;
using std::endl;

#include <pthread.h>

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
// ������������� ����������� ������
struct NamedMemory *connectToNamedMemory(const char* name);
// ��������� �������������� ������� ��� �������� ���������
void setPeriodicTimer(timer_t* periodicTimer, struct itimerspec* periodicTimerStruct, int sigChId, long tick);
// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime);
// ��������� ������� ���������� ������
void deadHandler(int signo);

// ������� ������ T1
void* funcT1(void* args);
// ������� ������ T2
void* funcT2(void* args);

// ���������� T1
pthread_t threadT1;
// ���������� T2
pthread_t threadT2;

int main(int argc, char *argv[]) {
	cout << "P0: �������" << endl;
	viewWorkDirectory();

	//�������� ������
	int chId = ChannelCreate(_NTO_CHF_SENDER_LEN);
	char buffer[20];
	const char *chIdStr = itoa(chId, buffer, 10);
	cout << "P0: ����� ������: chId = " << chId << endl;

	//����� ��������� �������� P1
	int pidP1 = spawnl( P_NOWAIT, "/home/host/lab3p1/x86/o/lab3p1", chIdStr, NULL);
	if (pidP1 < 0){
		cout << "P0: ������ ������� �������� P1" << pidP1 << endl;
		return EXIT_FAILURE;
	}
	cout << "P0: pid �������� P1 - " << pidP1 <<  endl;

	bool isWait = true;
	while(isWait){
		char msg[20];
		_msg_info info;
	    int rcvid = MsgReceive(chId, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P0: ������ MsgReceive - " << strerror(rcvid) << endl;
	    }

	    cout << "P0: �������� ��������� �� P1" <<  endl;
	    if (info.pid == pidP1){
	    	MsgReply(rcvid, NULL, msg, sizeof(msg));
	    	isWait = false;
	    }
	}

	struct NamedMemory *namedMemoryPtr = connectToNamedMemory(NAMED_MEMORY);

    timer_t stopTimer;
	struct itimerspec stopPeriod;
	setTimerStop(&stopTimer, &stopPeriod, namedMemoryPtr->timeInfo.endTime);

	int pidP2 = spawnl( P_NOWAIT, "/home/host/lab3p2/x86/o/lab3p2", chIdStr, NULL);
	if (pidP2 < 0){
		cout << "P0: ������ ������� �������� P2 - " << strerror(pidP2) << endl;
		return EXIT_FAILURE;
	}
	cout << "P0: pid �������� P2 - " << pidP2 <<  endl;

	namedMemoryPtr->pidP1 = pidP1;
	namedMemoryPtr->pidP2 = pidP2;

	int threadT1Res = pthread_create(&threadT1, NULL, funcT1, NULL);
	if(threadT1Res != 0){
		cout << "P0: ������ ������ T1 " << strerror(threadT1Res) <<  endl;
		return EXIT_FAILURE;
	}
	cout << "P0: ����� T1 ������ - " << threadT1 <<  endl;

	int threadT2Res = pthread_create(&threadT2, NULL, funcT2, NULL);
	if(threadT2Res != 0){
		cout << "P0: ������ ������ T2 " << strerror(threadT2Res) <<  endl;
		return EXIT_FAILURE;
	}
	cout << "P0: ����� T2 ������ - " << threadT2 <<  endl;

	cout << "P0: � �������" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P0: ������ ������" << endl;

	// ������ ������� ����������
	int res = timer_settime(stopTimer, 0, &stopPeriod, NULL);
	if(res == -1){
		cout << "P0: ������ ������� ������� ���������� - " << strerror(res)<< endl;
	}

	while(true){ }

	return EXIT_SUCCESS;
}

// ������� ������ T1
void* funcT1(void* args) {

	cout << "P0-T1: �����" << endl;

	struct NamedMemory *namedMemoryPtr = connectToNamedMemory(NAMED_MEMORY);

    int sigChId = ChannelCreate(_NTO_CHF_SENDER_LEN);
    cout << "P0-T1: ����� ������: sigChId = " << sigChId << endl;

    timer_t periodicTimer;
    struct itimerspec periodicTick;
    setPeriodicTimer(&periodicTimer, &periodicTick, sigChId, namedMemoryPtr->timeInfo.durTickDt);

	cout << "P0-T1: � �������" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P0-T1: ������ ������" << endl;

	int res = timer_settime(periodicTimer, 0, &periodicTick, NULL);
	if(res == -1){
		cout << "P0-T1: ������ ������� �������������� ������� - " << strerror(res)<< endl;
	}

	while(true){
		MsgReceivePulse(sigChId, NULL, 0, NULL);
		namedMemoryPtr->timeInfo.countTickDt++;
		//cout << "P0-T1: countTickDt " << namedMemoryPtr->timeInfo.countTickDt <<  endl;

		// ���������� ������ P2
		kill(namedMemoryPtr->pidP2, namedMemoryPtr->tickSigusrP2);
	}
}

// ������� ������ T2
void* funcT2(void* args) {

	cout << "P0-T2: �����" << endl;

	struct NamedMemory *namedMemoryPtr = connectToNamedMemory(NAMED_MEMORY);

    int sigChId = ChannelCreate(_NTO_CHF_SENDER_LEN);
    cout << "P0-T2: ����� ������: sigChId = " << sigChId << endl;

	int p1TickCoid = ConnectAttach(0, namedMemoryPtr->pidP1, namedMemoryPtr->signChIdP1, _NTO_SIDE_CHANNEL, 0);
	if(p1TickCoid < 0){
		cout << "P0-T2: ������ ��������� ���������� c P1 - " << strerror(p1TickCoid) <<  endl;
	    exit(EXIT_FAILURE);
	}

    timer_t periodicTimer;
    struct itimerspec periodicTick;
    setPeriodicTimer(&periodicTimer, &periodicTick, sigChId, namedMemoryPtr->timeInfo.durTickT);

	cout << "P0-T2: � �������" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P0-T2: ������ ������" << endl;

	int res = timer_settime(periodicTimer, 0, &periodicTick, NULL);
	if(res == -1){
		cout << "P0-T2: ������ ������� �������������� ������� - " << strerror(res)<< endl;
	}

	while(true){
		MsgReceivePulse(sigChId, NULL, 0, NULL);
		namedMemoryPtr->timeInfo.countTickT++;
		//cout << "P0-T1: countTickT " << namedMemoryPtr->timeInfo.countTickT <<  endl;

	    // ���������� ������� ���� �������� P1: ��������� - 10, ��� - 10, �������� - 10
	    MsgSendPulse(p1TickCoid, 10, 10, 10);
	}
}

// ��������� �������������� ������� ��� �������� ���������
void setPeriodicTimer(timer_t* periodicTimer, struct itimerspec* periodicTimerStruct, int sigChId, long tick){
	// ���������� ��� ��������� �����������
	int coid = ConnectAttach(0, 0, sigChId, 0, _NTO_COF_CLOEXEC);
	if(coid ==-1){
		cout << "P0: ������ ��������� ���������� ������ � �������� - " << strerror(coid) <<  endl;
	    exit(EXIT_FAILURE);
	}

	// ��������
	struct sigevent event;
	SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, 1, 0);

	timer_create(CLOCK_REALTIME, &event,  periodicTimer);

	// ���������� �������� ������������ �������������� ������� ���� � ��������� �������
	periodicTimerStruct->it_value.tv_sec = 0;
	periodicTimerStruct->it_value.tv_nsec = tick;
	periodicTimerStruct->it_interval.tv_sec = 0;
	periodicTimerStruct->it_interval.tv_nsec =  tick;
}

// ��������� ������� ���������� ������
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime) {
	struct sigevent event;
	SIGEV_SIGNAL_INIT(&event, SIGUSR2);
	int res1 = timer_create(CLOCK_REALTIME, &event, stopTimer);
	if(res1 == -1){
		cout << "P0: ������ �������� ������� ��������� " << strerror(res1)<< endl;
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
		cout << "P0: ������ ������ ���������� ��������" <<  endl;
		pthread_abort(threadT1);
		pthread_abort(threadT2);
		exit(EXIT_SUCCESS);
	}
}

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P0: ������� ������� ���������� - " << cCurrentPath << endl;
}

// ������� ������������� � �������� ����������� ������
struct NamedMemory* connectToNamedMemory(const char* name) {
	struct NamedMemory *namedMemoryPtr;

	//���������� ����������� ������
	int fd = shm_open(name, O_RDWR, 0777);
	if(fd == -1){
		cout << "P0: ������ �������� ������� ����������� ������ - " << strerror(fd) <<  endl;
	    exit(EXIT_FAILURE);
	}

	namedMemoryPtr = (NamedMemory*) mmap(NULL, sizeof(struct NamedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(namedMemoryPtr == MAP_FAILED){
		cout << "P0: ������ ������������� ���. ���. ������������" <<  endl;
	    exit(EXIT_FAILURE);
	}

	return namedMemoryPtr;
}
