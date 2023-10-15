#include <cstdlib>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <pthread.h>

#include <time.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cout;
using std::endl;

// Блокировки чтения/записи. Ждущие блокировки. Барьеры.

// Отображение текущей директории
void viewWorkDirectory();
// Функция потока T1
void* funcT1(void* args);
// Функция потока T2
void* funcT2(void* args);
// Запись одного символа с имитацией времени записи
void appendChar(char *text, char c);
// Распечатывание буфера в консоли
void printBuffer(char *text);

// Структура параметров для T1
struct threadT1dArg {
	void* func;
	char* textBuf;
};

// Объект "блокировка чтение/запись"
pthread_rwlock_t rwlock;
// Объект "барьер"
pthread_barrier_t barrier;

// Флаг завершения записи нитью T1
bool isProcessT1 = false;

char* mainText = (char*) "Text0, ";
char* t1Text = (char*) "Text1, ";
char* t2Text = (char*) "Text2.\n ";

int main(int argc, char *argv[]) {
	cout << "Main: Старт" << endl;
	viewWorkDirectory();

	// Буфер формируемого текста
	char textBuf[200] = {};

	// Инициализация блокировки чтения/записи
	pthread_rwlock_init(&rwlock, NULL);
	int rc;

	// Инициализация барьера
	pthread_barrier_init(&barrier, NULL, 3);

	cout << "Main: Получение блокировки для записи" << endl;
	rc = pthread_rwlock_wrlock(&rwlock);
	if(rc != 0){
		cout << "Main: Ошибка получения блокировки для записи " << strerror(rc) <<  endl;
		exit(EXIT_FAILURE);
	}

	struct threadT1dArg t1Arg = {(void*) funcT2, textBuf};

	// дескриптор нити, атрибутная запись, функция нити, аргументы - буфер текста
	pthread_t threadT1;
	int threadT1Res = pthread_create(&threadT1, NULL, funcT1, &t1Arg);
	if(threadT1Res != 0){
		cout << "Main: Ошибка старта T1 " << strerror(threadT1Res) <<  endl;
		return EXIT_FAILURE;
	}
	cout << "Main: Поток T1 создан - " << threadT1 <<  endl;

	// Посимвольная запись в буфер текста нитью main
	for (unsigned int i = 0; i < strlen(mainText); i++) {
		appendChar(textBuf, mainText[i]);
	}

	cout << "Main: Освобождение блокировки для записи" << endl;
	rc = pthread_rwlock_unlock(&rwlock);
	if(rc != 0){
		cout << "Main: Ошибка освобождения блокировки для записи " << strerror(rc) <<  endl;
		exit(EXIT_FAILURE);
	}

	cout << "Main: Закончил запись текста" <<  endl;

	cout << "Main: У барьера" << endl;
	pthread_barrier_wait(&barrier);
	cout << "Main: Прошёл барьер" << endl;

	printBuffer(textBuf);

	return EXIT_SUCCESS;
}

// Функция потока T1
void* funcT1(void* args) {
	cout << "T1: Старт" << endl;

	struct threadT1dArg arg = *((struct threadT1dArg*)args);

	// дескриптор нити, атрибутная запись, функция нити, аргументы - буфер текста
	pthread_t threadT2;
	int threadT2Res = pthread_create(&threadT2, NULL, (void *(*)(void *)) arg.func, (void*) arg.textBuf);
	if(threadT2Res != 0){
		cout << "T1: Ошибка старта T2 " << strerror(threadT2Res) <<  endl;
		exit(EXIT_FAILURE);
	}
	cout << "T1: Поток T2 создан - " << threadT2 <<  endl;

	int rc;
	int count = 0;
	bool isWait = true;

	while(isWait){
		rc = pthread_rwlock_tryrdlock(&rwlock);
		// EBUSY - 16
		if (rc == 16) {
			if (count >= 15) {
				cout << "T1: Достигнуто ограничение на число попыток" << endl;
				exit(EXIT_FAILURE);
		    }
			 ++count;
			 cout << "T1: Не удалось получить блокировку - ожидание" << endl;
			 sleep(1);
		} else{
			if(rc != 0){
				cout << "T1: Ошибка получения блокировки для записи " << strerror(rc) <<  endl;
				exit(EXIT_FAILURE);
			} else {
				isWait = false;
			}
		}
	}

	cout << "T1: Приступаю к работе" << endl;

	cout << "T1: Захват буфера для записи текста" <<  endl;
	for (unsigned int i = 0; i < strlen(t1Text); i++) {
		//pthread_sleepon_lock();
		appendChar(arg.textBuf, t1Text[i]);
		//isProcessT1 = false;
		//pthread_sleepon_signal(&isProcessT1);
		//pthread_sleepon_unlock();
	}

	cout << "T1: Закончил запись текста" <<  endl;
	isProcessT1 = true;
	pthread_sleepon_lock();
	pthread_sleepon_signal(&isProcessT1);
	pthread_sleepon_unlock();

	 cout << "T1: У барьера" << endl;
	 pthread_barrier_wait(&barrier);
	 cout << "T1: Прошёл барьер" << endl;

	 return EXIT_SUCCESS;
}

// Функция потока T2
void* funcT2(void* args) {
	cout << "T2: Старт" << endl;
	char* textBuf = (char*) args;

	pthread_sleepon_lock();
	//Проверка надо ли ждать и ожидание смены флага
	while(isProcessT1 == false){
		cout << "T2: Ожидание T1" << endl;
		pthread_sleepon_wait(&isProcessT1);
	}
	pthread_sleepon_unlock();

	cout << "T2: Приступаю к работе" << endl;

	cout << "T2: Захват буфера для записи текста" <<  endl;
	const char* t2Text = "Text2.\n ";

	for (unsigned int i = 0; i < strlen(t2Text); i++) {
		appendChar(textBuf, t2Text[i]);
	}

	pthread_sleepon_unlock();

	 cout << "T2: Закончил запись текста" <<  endl;

	 cout << "T2: У барьера" << endl;
	 pthread_barrier_wait(&barrier);
	 cout << "T2: Прошёл барьер" << endl;

	 return EXIT_SUCCESS;
}

// Запись одного символа с имитацией времени записи
void appendChar(char *text, char c) {
	size_t p = strlen(text);

	text[p] = c;
	text[p + 1] = '\0';

	for (int i = 0; i < 100000000; i++);
	sleep(1);
}

// Отображение текущей директории
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "Log: Текущая рабочая директория - " << cCurrentPath << endl;

	time_t rawtime;
	time(&rawtime);
	struct tm* ptm = gmtime(&rawtime);
	cout << "Log: Текущая дата/время - " << ptm->tm_mday << "." << 1 + ptm->tm_mon << "." << 1900 + ptm->tm_year  << " " << ptm->tm_hour << ":" << 1 + ptm->tm_min << ":" << 1 + ptm->tm_sec << endl;
}

// Распечатывание буфера в консоли
void printBuffer(char *text){
	size_t p = strlen(text);

	cout << "Log: Буфер сейчас: ";
	for(int i = 0; i < p; i++){
		cout << text[i];
	}
	cout << endl;
}
