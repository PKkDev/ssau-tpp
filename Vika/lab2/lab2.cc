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

// �������. ������ ����������. �������������.

// ������ //
pthread_mutex_t mutex;
pthread_mutexattr_t mutexAtr;
// ������ //

// ������ ���������� //
// ���� ���������� ������ ����� T1
bool isDoneT1 = false;
// ������ ���������� //

// ������ ������ ������� � ��������� ������� ������
void appendChar(char *text, char c);
// ������� ������ T1
void* funcT1(void* args);
// ������� ������ T2
void* funcT2(void* args);

// ��������� ���������� ��� T1
struct threadT1dArg {
	void* func;
	char* textBuf;
};

// ������������ �����
char* mainText = (char*) "Text0, ";
char* t1Text = (char*) "Text1, ";
char* t2Text = (char*) "Text2.\n ";

int main(int argc, char *argv[]) {
	cout << "Main: �����" << endl;

	// ����� ������������ ������
	char textBuf[200] = {};

	int mRes = pthread_mutex_init(&mutex, &mutexAtr);
	if(mRes != 0 ){
		cout << "Main: ������ ������������� ������� " << strerror(mRes) <<  endl;
		exit(EXIT_FAILURE);
	}

	cout << "Main: ������ �������\n";
	pthread_mutex_lock(&mutex);

	struct threadT1dArg t1Arg = {(void*) funcT2, textBuf};

	// ���������� ����, ���������� ������, ������� ����, ��������� - ����� ������
	pthread_t threadT1;
	int threadT1Res = pthread_create(&threadT1, NULL, funcT1, &t1Arg);
	if(threadT1Res != 0){
		cout << "Main: ������ ������ T1 " << strerror(threadT1Res) <<  endl;
		return EXIT_FAILURE;
	}
	cout << "Main: ����� T1 ������ - " << threadT1 <<  endl;

	for (unsigned int i = 0; i < strlen(mainText); i++) {
		appendChar(textBuf, mainText[i]);
	}

	cout << "Main: ������������ �������\n";
	pthread_mutex_unlock(&mutex);

	cout << "Main: ������������� � T1" <<  endl;
	pthread_join(threadT1, NULL);

	size_t p = strlen(textBuf);
	cout << "Main: ����� ������: ";
	for(int i = 0; i < p; i++){
		cout << textBuf[i];
	}
	cout << endl;

	pthread_mutex_destroy(&mutex);
	cout << "Main: Ok" <<  endl;
	return EXIT_SUCCESS;
}

// ������� ������ T1
void* funcT1(void* args) {
	cout << "T1: �����" << endl;

	struct threadT1dArg arg = *((struct threadT1dArg*)args);

	// ���������� ����, ���������� ������, ������� ����, ��������� - ����� ������
	pthread_t threadT2;
	int threadT2Res = pthread_create(&threadT2, NULL, (void *(*)(void *)) arg.func, (void*) arg.textBuf);
	if(threadT2Res != 0){
		cout << "T1: ������ ������ T2 " << strerror(threadT2Res) <<  endl;
		exit(EXIT_FAILURE);
	}
	cout << "T1: ����� T2 ������ - " << threadT2 <<  endl;
	sleep(1);
	cout << "T1: ������� ������� ������� - ��������" << endl;
	pthread_mutex_lock(&mutex);

	cout << "T1: ��������� � ������" << endl;
	for (unsigned int i = 0; i < strlen(t1Text); i++) {
		//pthread_sleepon_lock();
		appendChar(arg.textBuf, t1Text[i]);
		//isDoneT1 = false;
		//pthread_sleepon_signal(&isDoneT1);
		//pthread_sleepon_unlock();
	}

	pthread_mutex_unlock(&mutex);

	cout << "T1: �������� ������ ������" <<  endl;
	isDoneT1 = true;
	pthread_sleepon_lock();
	pthread_sleepon_signal(&isDoneT1);
	pthread_sleepon_unlock();

	cout << "T1: ������������� � T2" <<  endl;
	pthread_join(threadT2, NULL);

	cout << "T1: Ok" <<  endl;
	return EXIT_SUCCESS;
}

// ������� ������ T2
void* funcT2(void* args) {
	cout << "T2: �����" << endl;

	char* textBuf = (char*) args;

	pthread_sleepon_lock();
	//�������� ���� �� ����� � �������� ����� �����
	while(isDoneT1 == false){
		cout << "T2: �������� T1" << endl;
		pthread_sleepon_wait(&isDoneT1);
	}
	pthread_sleepon_unlock();

	cout << "T2: ��������� � ������" << endl;
	for (unsigned int i = 0; i < strlen(t2Text); i++) {
		appendChar(textBuf, t2Text[i]);
	}

	cout << "T2: Ok" <<  endl;
	return EXIT_SUCCESS;
}

// ������ ������ ������� � ��������� ������� ������
void appendChar(char *text, char c) {
	size_t p = strlen(text);

	text[p] = c;
	text[p + 1] = '\0';

	for (int i = 0; i < 100000000; i++);
	sleep(1);
}
