#include <cstdlib>
#include <iostream>

#include <stdlib.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include <process.h>
#include <string.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cout;
using std::endl;

// Отображение текущей директории
void viewWorkDirectory();

int main(int argc, char *argv[]) {

	cout << "P3: Запущен" << endl;
	viewWorkDirectory();
	cout << "P3: Параметры: " << "argv[0]=  " << argv[0] <<  endl;

	int pChid = atoi(argv[0]);

	// Ответ от P2
	int pidP1;
	int pChidP1;

	// Утсанавливаем соединение с P2 и отправляем сообщение для получения информации о P1 //

	// Id канала для отправки сообщения
	cout << "P3: установление соединения с каналом P2" <<  endl;
	int coidP2 = ConnectAttach(0, getppid(), pChid, _NTO_SIDE_CHANNEL, 0);
	if(coidP2 == -1){
		cout << "P3: Ошибка соединения с каналом P2" <<  endl;
	    exit(EXIT_FAILURE);
	}

	char rmsg[200] = {};

	// Послать сообщение
	cout << "Р3: Посылаю сообщение Р2" << rmsg <<  endl;
	char *smsg1 = (char *)"Дай мне pid процесса Р1";
	if(MsgSend(coidP2, smsg1, strlen(smsg1) + 1, rmsg, sizeof(rmsg)) == -1){
		cout << "P3: Ошибка MsgSend при отправки в P2" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P3: Сервер P2 ответил " << endl;
		pidP1 = atoi(rmsg);
    	cout << "P3: pid процесса Р1: " << pidP1 << endl;
	}

	// Послать сообщение
	cout << "Р3: Посылаю сообщение Р2" <<  endl;
	char *smsg2 = (char *)"Дай мне chid канала Р1";
	if(MsgSend(coidP2, smsg2, strlen(smsg2) + 1, rmsg, sizeof(rmsg)) == -1){
		cout << "P3: Ошибка MsgSend при отправки в P2" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P3: Сервер P2 ответил " << rmsg << endl;
		pChidP1 = atoi(rmsg);
    	cout << "P3: chid канала Р1: " << pChidP1 << endl;
	}

	// Утсанавливаем соединение с P2 и отправляем сообщение для получения информации о P1 //

	// Утсанавливаем соединение с P1 и отправляем сообщение //

	// Id канала для отправки сообщения
	cout << "P3: установление соединения с каналом P1" <<  endl;
	int coidP1 = ConnectAttach(0, pidP1, pChidP1, _NTO_SIDE_CHANNEL, 0);
	if(coidP1 == -1){
		cout << "P3: Ошибка соединения с каналом P1" <<  endl;
	    exit(EXIT_FAILURE);
	}

	// Буфер ответа
	char rmsg2[200] = {};

	cout << "Р3: Посылаю сообщение Р1" <<  endl;
	char *smsg3 = (char *)"Р3 send message to P1";
	if(MsgSend(coidP1, smsg3, strlen(smsg3) + 1, rmsg2, sizeof(rmsg2)) == -1){
		cout << "P3: Ошибка MsgSend при отправки в P1" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg2) > 0){
		cout << "P3: Сервер P1 ответил - " << rmsg2 << endl;
	}

	// Утсанавливаем соединение с P1 и отправляем сообщение //

	cout << "Р3: Посылаю сообщение Р2" <<  endl;
	char *smsg4 = (char *)"Р3 stop";
	if(MsgSend(coidP2, smsg4, strlen(smsg4) + 1, rmsg, sizeof(rmsg)) == -1){
		cout << "P3: Ошибка MsgSend при отправки в P2" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P3: Сервер P2 ответил" << endl;
	}

	cout << "P3: Р3 ОК" << endl;
	return EXIT_SUCCESS;
}

// Отображение текущей директории
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P3: текущая рабочая директория - " << cCurrentPath << endl;
}
