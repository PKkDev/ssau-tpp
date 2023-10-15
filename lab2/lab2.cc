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

// ���������� ������/������. ������ ����������. �������.

// ����������� ������� ����������
void viewWorkDirectory();
// ������� ������ T1
void* funcT1(void* args);
// ������� ������ T2
void* funcT2(void* args);
// ������ ������ ������� � ��������� ������� ������
void appendChar(char *text, char c);
// �������������� ������ � �������
void printBuffer(char *text);

// ��������� ���������� ��� T1
struct threadT1dArg {
	void* func;
	char* textBuf;
};

// ������ "���������� ������/������"
pthread_rwlock_t rwlock;
// ������ "������"
pthread_barrier_t barrier;

// ���� ���������� ������ ����� T1
bool isProcessT1 = false;

char* mainText = (char*) "Text0, ";
char* t1Text = (char*) "Text1, ";
char* t2Text = (char*) "Text2.\n ";

int main(int argc, char *argv[]) {
	cout << "Main: �����" << endl;
	viewWorkDirectory();

	// ����� ������������ ������
	char textBuf[200] = {};

	// ������������� ���������� ������/������
	pthread_rwlock_init(&rwlock, NULL);
	int rc;

	// ������������� �������
	pthread_barrier_init(&barrier, NULL, 3);

	cout << "Main: ��������� ���������� ��� ������" << endl;
	rc = pthread_rwlock_wrlock(&rwlock);
	if(rc != 0){
		cout << "Main: ������ ��������� ���������� ��� ������ " << strerror(rc) <<  endl;
		exit(EXIT_FAILURE);
	}

	struct threadT1dArg t1Arg = {(void*) funcT2, textBuf};

	// ���������� ����, ���������� ������, ������� ����, ��������� - ����� ������
	pthread_t threadT1;
	int threadT1Res = pthread_create(&threadT1, NULL, funcT1, &t1Arg);
	if(threadT1Res != 0){
		cout << "Main: ������ ������ T1 " << strerror(threadT1Res) <<  endl;
		return EXIT_FAILURE;
	}
	cout << "Main: ����� T1 ������ - " << threadT1 <<  endl;

	// ������������ ������ � ����� ������ ����� main
	for (unsigned int i = 0; i < strlen(mainText); i++) {
		appendChar(textBuf, mainText[i]);
	}

	cout << "Main: ������������ ���������� ��� ������" << endl;
	rc = pthread_rwlock_unlock(&rwlock);
	if(rc != 0){
		cout << "Main: ������ ������������ ���������� ��� ������ " << strerror(rc) <<  endl;
		exit(EXIT_FAILURE);
	}

	cout << "Main: �������� ������ ������" <<  endl;

	cout << "Main: � �������" << endl;
	pthread_barrier_wait(&barrier);
	cout << "Main: ������ ������" << endl;

	printBuffer(textBuf);

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

	int rc;
	int count = 0;
	bool isWait = true;

	while(isWait){
		rc = pthread_rwlock_tryrdlock(&rwlock);
		// EBUSY - 16
		if (rc == 16) {
			if (count >= 15) {
				cout << "T1: ���������� ����������� �� ����� �������" << endl;
				exit(EXIT_FAILURE);
		    }
			 ++count;
			 cout << "T1: �� ������� �������� ���������� - ��������" << endl;
			 sleep(1);
		} else{
			if(rc != 0){
				cout << "T1: ������ ��������� ���������� ��� ������ " << strerror(rc) <<  endl;
				exit(EXIT_FAILURE);
			} else {
				isWait = false;
			}
		}
	}

	cout << "T1: ��������� � ������" << endl;

	cout << "T1: ������ ������ ��� ������ ������" <<  endl;
	for (unsigned int i = 0; i < strlen(t1Text); i++) {
		//pthread_sleepon_lock();
		appendChar(arg.textBuf, t1Text[i]);
		//isProcessT1 = false;
		//pthread_sleepon_signal(&isProcessT1);
		//pthread_sleepon_unlock();
	}

	cout << "T1: �������� ������ ������" <<  endl;
	isProcessT1 = true;
	pthread_sleepon_lock();
	pthread_sleepon_signal(&isProcessT1);
	pthread_sleepon_unlock();

	 cout << "T1: � �������" << endl;
	 pthread_barrier_wait(&barrier);
	 cout << "T1: ������ ������" << endl;

	 return EXIT_SUCCESS;
}

// ������� ������ T2
void* funcT2(void* args) {
	cout << "T2: �����" << endl;
	char* textBuf = (char*) args;

	pthread_sleepon_lock();
	//�������� ���� �� ����� � �������� ����� �����
	while(isProcessT1 == false){
		cout << "T2: �������� T1" << endl;
		pthread_sleepon_wait(&isProcessT1);
	}
	pthread_sleepon_unlock();

	cout << "T2: ��������� � ������" << endl;

	cout << "T2: ������ ������ ��� ������ ������" <<  endl;
	const char* t2Text = "Text2.\n ";

	for (unsigned int i = 0; i < strlen(t2Text); i++) {
		appendChar(textBuf, t2Text[i]);
	}

	pthread_sleepon_unlock();

	 cout << "T2: �������� ������ ������" <<  endl;

	 cout << "T2: � �������" << endl;
	 pthread_barrier_wait(&barrier);
	 cout << "T2: ������ ������" << endl;

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

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "Log: ������� ������� ���������� - " << cCurrentPath << endl;

	time_t rawtime;
	time(&rawtime);
	struct tm* ptm = gmtime(&rawtime);
	cout << "Log: ������� ����/����� - " << ptm->tm_mday << "." << 1 + ptm->tm_mon << "." << 1900 + ptm->tm_year  << " " << ptm->tm_hour << ":" << 1 + ptm->tm_min << ":" << 1 + ptm->tm_sec << endl;
}

// �������������� ������ � �������
void printBuffer(char *text){
	size_t p = strlen(text);

	cout << "Log: ����� ������: ";
	for(int i = 0; i < p; i++){
		cout << text[i];
	}
	cout << endl;
}
