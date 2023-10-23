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

// Мутексы. Ждущие блокировки. Присоединение.

// мутекс //
pthread_mutex_t mutex;
pthread_mutexattr_t mutexAtr;
// мутекс //

// Ждущие блокировки //
// Флаг завершения записи нитью T1
bool isDoneT1 = false;
// Ждущие блокировки //

// Запись одного символа с имитацией времени записи
void appendChar(char *text, char c);
// Функция потока T1
void* funcT1(void* args);
// Функция потока T2
void* funcT2(void* args);

// Структура параметров для T1
struct threadT1dArg {
	void* func;
	char* textBuf;
};

// Записываемый текст
char* mainText = (char*) "Text0, ";
char* t1Text = (char*) "Text1, ";
char* t2Text = (char*) "Text2.\n ";

int main(int argc, char *argv[]) {
	cout << "Main: Старт" << endl;

	// Буфер формируемого текста
	char textBuf[200] = {};

	int mRes = pthread_mutex_init(&mutex, &mutexAtr);
	if(mRes != 0 ){
		cout << "Main: Ошибка инициализации мутекса " << strerror(mRes) <<  endl;
		exit(EXIT_FAILURE);
	}

	cout << "Main: Захват мутекса\n";
	pthread_mutex_lock(&mutex);

	struct threadT1dArg t1Arg = {(void*) funcT2, textBuf};

	// дескриптор нити, атрибутная запись, функция нити, аргументы - буфер текста
	pthread_t threadT1;
	int threadT1Res = pthread_create(&threadT1, NULL, funcT1, &t1Arg);
	if(threadT1Res != 0){
		cout << "Main: Ошибка старта T1 " << strerror(threadT1Res) <<  endl;
		return EXIT_FAILURE;
	}
	cout << "Main: Поток T1 создан - " << threadT1 <<  endl;

	for (unsigned int i = 0; i < strlen(mainText); i++) {
		appendChar(textBuf, mainText[i]);
	}

	cout << "Main: Освобождение мутекса\n";
	pthread_mutex_unlock(&mutex);

	cout << "Main: Присоединение к T1" <<  endl;
	pthread_join(threadT1, NULL);

	size_t p = strlen(textBuf);
	cout << "Main: Буфер сейчас: ";
	for(int i = 0; i < p; i++){
		cout << textBuf[i];
	}
	cout << endl;

	pthread_mutex_destroy(&mutex);
	cout << "Main: Ok" <<  endl;
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
	sleep(1);
	cout << "T1: Попытка захвата мутекса - ожидание" << endl;
	pthread_mutex_lock(&mutex);

	cout << "T1: Приступаю к работе" << endl;
	for (unsigned int i = 0; i < strlen(t1Text); i++) {
		//pthread_sleepon_lock();
		appendChar(arg.textBuf, t1Text[i]);
		//isDoneT1 = false;
		//pthread_sleepon_signal(&isDoneT1);
		//pthread_sleepon_unlock();
	}

	pthread_mutex_unlock(&mutex);

	cout << "T1: Закончил запись текста" <<  endl;
	isDoneT1 = true;
	pthread_sleepon_lock();
	pthread_sleepon_signal(&isDoneT1);
	pthread_sleepon_unlock();

	cout << "T1: Присоединение к T2" <<  endl;
	pthread_join(threadT2, NULL);

	cout << "T1: Ok" <<  endl;
	return EXIT_SUCCESS;
}

// Функция потока T2
void* funcT2(void* args) {
	cout << "T2: Старт" << endl;

	char* textBuf = (char*) args;

	pthread_sleepon_lock();
	//Проверка надо ли ждать и ожидание смены флага
	while(isDoneT1 == false){
		cout << "T2: Ожидание T1" << endl;
		pthread_sleepon_wait(&isDoneT1);
	}
	pthread_sleepon_unlock();

	cout << "T2: Приступаю к работе" << endl;
	for (unsigned int i = 0; i < strlen(t2Text); i++) {
		appendChar(textBuf, t2Text[i]);
	}

	cout << "T2: Ok" <<  endl;
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
