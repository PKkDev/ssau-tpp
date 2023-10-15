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

// Длительность тика 1t (уведомление импульсом - 0,05 c) (наносекунды)
#define DUR_TICK_T 50000000
// Длительность тика dt (уведомление сигналом - 0,3 с) (наносекунды)
#define DUR_TICK_DT 300000000
// Время работы приложения (сек)
#define END_TIME 5 //107 5
// Номер сигнала наступления нового тика (уведомления)
#define TICK_SIGUSR_P2 SIGUSR1
// Имя именованной памяти
#define NAMED_MEMORY "/16/namedMemory"

// ---- Структуры для именованной памяти ---- //
// Структура данных с информацией о течении времени приложения
struct Clock {
	long durTickT; 		// Длительность одного тика в наносекундах
	long durTickDt; 	// Длительность одного тика в наносекундах
	int countTickDt;	// Номер текущего тика часов ПРВ
	int countTickT;		// Номер текущего тика часов ПРВ
	long endTime;		// Длительность работы приложения в секундах
};
// Структура данных, хранящаяся в именованной памяти NAMED_MEMORY
struct NamedMemory {
	double p;							// Вычисляемый параметр

	int pidP2; 							// ID процесса P2
	int pidP1; 							// ID процесса P1
	int signChIdP1;						// ID канала процесса P1
	int tickSigusrP2;					// Номер сигнала наступления нового тика (уведомления)

	pthread_mutexattr_t mutexAttr;	 	// Атрибутная запись мутекса
	pthread_mutex_t mutex;			 	// Мутекс доступа к именованной памяти

	pthread_barrier_t startBarrier;		// Барьер старта таймеров

	Clock timeInfo;						// Информация о течении времени ПРВ
};
// ---- Структуры для именованной памяти ---- //

// Отображение текущей директории
void viewWorkDirectory();
// Обработка сигнала завершения работы
void deadHandler(int signo);
// Отправляет P0 сообщение о готовности к продолжению работы
void sendReadyMessageToP0(char *chIdP0Str);
// Выполнение расчёта функции в единицу времени
double func(double t);
// Устанвока таймера завершения работы
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime);
// Создание именованной памяти
NamedMemory *createNamedMemory(const char* name);

// Указатель именованной памяти
struct NamedMemory *namedMemoryPtr;

int main(int argc, char *argv[]) {
	cout << "P1: Запущен" << endl;
	viewWorkDirectory();
	cout << "P1: Параметры: " << "argv[0]=  " << argv[0] <<  endl;

	// Присоединение именованной памяти
	namedMemoryPtr = createNamedMemory(NAMED_MEMORY);

	// Установка параметров времени приложения
	namedMemoryPtr->timeInfo.durTickT = DUR_TICK_T;
	namedMemoryPtr->timeInfo.durTickDt = DUR_TICK_DT;
	namedMemoryPtr->timeInfo.endTime = END_TIME;
	// показание часов ПРВ в тиках, -1 - часы не запущены
	namedMemoryPtr->timeInfo.countTickT = -1;
	namedMemoryPtr->timeInfo.countTickDt = -1;

	namedMemoryPtr->tickSigusrP2 = TICK_SIGUSR_P2;

	// Барьер для синхронизации старта таймеров в процессах
	pthread_barrierattr_t startAttr;
	pthread_barrierattr_init(&startAttr);
	pthread_barrierattr_setpshared(&startAttr, PTHREAD_PROCESS_SHARED);
	pthread_barrier_init(&(namedMemoryPtr->startBarrier), &startAttr, 5);

	// Инициализация атрибутной записи разделяемого мутекса
	int r1 = pthread_mutexattr_init(&namedMemoryPtr->mutexAttr);
	if(r1 != EOK){
		cout << "Р1: Ошибка pthread_mutexattr_init: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}
	 // Установить в атрибутной записи мутекса свойство "разделяемый"
	int r2 = pthread_mutexattr_setpshared(&namedMemoryPtr->mutexAttr, PTHREAD_PROCESS_SHARED);
	if(r2 != EOK){
		cout << "Р1: Ошибка pthread_mutexattr_setpshared: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}
	// Инициализация разделяемого мутекса
	int r3 = pthread_mutex_init(&namedMemoryPtr->mutex, &namedMemoryPtr->mutexAttr);
	if(r3 != EOK){
		cout << "Р1: Ошибка pthread_mutex_init: " << strerror(errno) << endl;
		return EXIT_FAILURE;
	}

	//создание канала
	int signChIdP1 = ChannelCreate(_NTO_CHF_SENDER_LEN);
	namedMemoryPtr->signChIdP1 = signChIdP1;

	sendReadyMessageToP0(argv[0]);

	timer_t stopTimer;
	struct itimerspec stopPeriod;
	setTimerStop(&stopTimer, &stopPeriod, namedMemoryPtr->timeInfo.endTime);

	cout << "P1: У барьера" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P1: Прошёл барьер" << endl;

	// запуск таймера завершения
	int res = timer_settime(stopTimer, 0, &stopPeriod, NULL);
	if(res == -1){
		cout << "P1: Ошибка запуска таймера" << strerror(res)<< endl;
	}

	// величина тика из нсек в сек 50000000 нсек -> 0.05 сек
	const double tickSecDuration = namedMemoryPtr->timeInfo.durTickT / 1000000000.;

	while(true){
		MsgReceivePulse(signChIdP1, NULL, 0, NULL);

		double time = namedMemoryPtr->timeInfo.countTickT * tickSecDuration;
		double value = func(time);

		pthread_mutex_lock(&(namedMemoryPtr->mutex));
		namedMemoryPtr->p = value;
		pthread_mutex_unlock(&(namedMemoryPtr->mutex));

		//cout << "P1: Расчитал" << " value: " << value << " time: " << time << " countTickT: " << namedMemoryPtr->timeInfo.countTickT  << endl;
	}

	return EXIT_SUCCESS;
}

// Создание именованной памяти
NamedMemory *createNamedMemory(const char* name){
	struct NamedMemory *namedMemoryPtr;

	//дескриптор именованной памяти
	int fd = shm_open(name, O_RDWR | O_CREAT, 0777);
	if(fd == -1){
		cout << "P1: Ошибка создания/открытия объекта именованной памяти - " << strerror(fd) <<  endl;
	    exit(EXIT_FAILURE);
	}

	int tr1 = ftruncate(fd, 0);
	int tr2 = ftruncate(fd, sizeof(struct NamedMemory));
	if(tr1 == -1 || tr2 == -1){
		cout << "P1: Ошибка ftruncate" <<  endl;
		exit(EXIT_FAILURE);
	}

	namedMemoryPtr = (NamedMemory*) mmap(NULL, sizeof(struct NamedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(namedMemoryPtr == MAP_FAILED){
		cout << "P1: Ошибка сопоставления вир. адр. пространства" <<  endl;
	    exit(EXIT_FAILURE);
	}

	return namedMemoryPtr;
}

// Устанвока таймера завершения работы
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime) {
	struct sigevent event;
	SIGEV_SIGNAL_INIT(&event, SIGUSR2);
	int res1 = timer_create(CLOCK_REALTIME, &event, stopTimer);
	if(res1 == -1){
		cout << "P1: Ошибка создания таймера остановки " << strerror(res1)<< endl;
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

// Выполнение расчёта функции в единицу времени
double func(double t) {
	const double a = 1.9;
	const double b = 1.1;
	double result = (b * pow(t, 2))/(pow(M_E, a * t) + t);
	return result;
}

// Отправляет P0 сообщение о готовности к продолжению работы
void sendReadyMessageToP0(char *chIdP0Str){
	char rmsg[20];
	int chIdP0 = atoi(chIdP0Str);
	cout << "P1: установление соединения с каналом P0" <<  endl;
	int coidP0 = ConnectAttach(0, getppid(), chIdP0, _NTO_SIDE_CHANNEL, 0);
	if(coidP0 == -1){
		cout << "P1: Ошибка соединения с каналом P0 - " << strerror(coidP0) <<  endl;
	    exit(EXIT_FAILURE);
	}
	cout << "Р1: Посылаю сообщение Р0" <<  endl;
	char *smsg1 = (char *)"P1";
	int sendRes = MsgSend(coidP0, smsg1, strlen(smsg1) + 1, rmsg, sizeof(rmsg));
	if(sendRes == -1){
		cout << "P1: Ошибка MsgSend при отправки в P0 - " << strerror(sendRes) <<  endl;
		exit(EXIT_FAILURE);
	}
}

// Обработка сигнала завершения работы
void deadHandler(int signo) {
	if (signo == SIGUSR2) {
		cout << "P1: пришёл сигнал завершения процесса" <<  endl;
		pthread_barrier_destroy(&(namedMemoryPtr->startBarrier));
		exit(EXIT_SUCCESS);
	}
}

// Отображение текущей директории
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P1: текущая рабочая директория - " << cCurrentPath << endl;
}























