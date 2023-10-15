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

// Имя именованной памяти
#define NAMED_MEMORY "/16/namedMemory"
// Файл для записи трендов
#define TREND_FILE "/home/host/lab3Trend/trend.txt"

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
// Присоединение именованной памяти
struct NamedMemory *connectToNamedMemory(const char* name);
// Устанвока таймера завершения работы
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime);
// Обработка сигнала завершения работы
void deadHandler(int signo);

FILE* trendFile;

int main(int argc, char *argv[]) {
	cout << "P2: Запущен" << endl;
	viewWorkDirectory();

	struct NamedMemory *namedMemoryPtr = connectToNamedMemory(NAMED_MEMORY);

	trendFile = fopen(TREND_FILE, "w");
	if(trendFile == NULL){
		cout << "P2: Ошибка открытия файла для запси тренда" << endl;
		exit(EXIT_FAILURE);
	}
	cout << "Р2: Открыт файл тренда trend.txt" << endl;

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, namedMemoryPtr->tickSigusrP2);

	timer_t stopTimer;
	struct itimerspec stopPeriod;
	setTimerStop(&stopTimer, &stopPeriod, namedMemoryPtr->timeInfo.endTime);

	cout << "P2: У барьера" << endl;
	pthread_barrier_wait(&(namedMemoryPtr->startBarrier));
	cout << "P2: Прошёл барьер" << endl;

	// запуск таймера завершения
	int res = timer_settime(stopTimer, 0, &stopPeriod, NULL);
	if(res == -1){
		cout << "P2: Ошибка запуска таймера" << strerror(res)<< endl;
	}

	// величина тика из нсек в сек 300000000 нсек -> 0.3 сек
	const double tickSecDuration = namedMemoryPtr->timeInfo.durTickDt / 1000000000.;

	while(true){
		int sig = SignalWaitinfo(&set, NULL);
		if(sig == namedMemoryPtr->tickSigusrP2){

			pthread_mutex_lock(&(namedMemoryPtr->mutex));
			double value = namedMemoryPtr->p;
			pthread_mutex_unlock(&(namedMemoryPtr->mutex));

			double time = namedMemoryPtr->timeInfo.countTickDt * tickSecDuration;

			//cout << "P2: Записывает " << "value: " << value << " time: " << time << " countTickDt: " << namedMemoryPtr->timeInfo.countTickDt  << endl;
			fprintf(trendFile, "%f\t%f\n", value, time);
		}

	}

	return EXIT_SUCCESS;
}

// Отображение текущей директории
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P2: текущая рабочая директория - " << cCurrentPath << endl;
}

// Устанвока таймера завершения работы
void setTimerStop(timer_t* stopTimer, struct itimerspec* stopPeriod, long endTime) {
	struct sigevent event;
	SIGEV_SIGNAL_INIT(&event, SIGUSR2);
	int res1 = timer_create(CLOCK_REALTIME, &event, stopTimer);
	if(res1 == -1){
		cout << "P2: Ошибка создания таймера остановки " << strerror(res1)<< endl;
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

// Обработка сигнала завершения работы
void deadHandler(int signo) {
	if (signo == SIGUSR2) {
		cout << "P2: пришёл сигнал завершения процесса" <<  endl;
		fclose(trendFile);
		exit(EXIT_SUCCESS);
	}
}

// Функция присоединения к процессу именованной памяти
struct NamedMemory* connectToNamedMemory(const char* name) {
	struct NamedMemory *namedMemoryPtr;

	//дескриптор именованной памяти
	int fd = shm_open(name, O_RDWR, 0777);
	if(fd == -1){
		cout << "P2: Ошибка открытия объекта именованной памяти - " << strerror(fd) <<  endl;
	    exit(EXIT_FAILURE);
	}

	namedMemoryPtr = (NamedMemory*) mmap(NULL, sizeof(struct NamedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(namedMemoryPtr == MAP_FAILED){
		cout << "P2: Ошибка сопоставления вир. адр. пространства" <<  endl;
	    exit(EXIT_FAILURE);
	}

	return namedMemoryPtr;
}
