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

	cout << "P2: Запущен" << endl;
	viewWorkDirectory();
	cout << "P2: Параметры: " << "argv[0]=  " << argv[0] <<  endl;

	int chid = ChannelCreate(_NTO_CHF_SENDER_LEN);
	cout << "P2: Канал создан: chid = " << chid << endl;

	char buffer[20];
	const char *chidStr = itoa(chid, buffer, 10);

	int pidP3 = spawnl(P_NOWAIT, "/home/host/lab1p3/x86/o/lab1p3", chidStr, NULL);
	if (pidP3 < 0){
		cout << "P2: Ошибка запуска процесса P3" << pidP3 << endl;
		exit(EXIT_FAILURE);
	}
	cout << "P2: pid процесса P3 - " << pidP3 <<  endl;

	cout << "P2: Старт получения сообщений" << endl;

	int countMsg = 0;
	while(countMsg < 3){

		char msg[200] = {};
	    _msg_info info;
	    int rcvid;

	    rcvid = MsgReceive(chid, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P2: Ошибка MsgReceive" << endl;
	    }

	    if (info.pid == pidP3){

	    	cout << "P2: Получено сообщение от P3 - " << msg << endl;

	    	if(countMsg == 0){
	    		int pidP1 = getppid();
	    		char pidP1Buffer[50];
	    		char const *pidP3Str = itoa(pidP1, pidP1Buffer, 10);
	    		MsgReply(rcvid, NULL, pidP3Str, strlen(pidP3Str));
	    	}

	    	if(countMsg == 1){
	    		char *pChidStr = argv[0];
	    		MsgReply(rcvid, NULL, pChidStr, sizeof(pChidStr));
	    	}

	    	if(countMsg == 2){
	    		MsgReply(rcvid, NULL, msg, sizeof(msg));

	    		int pChid = atoi(argv[0]);

	    		cout << "P2: установление соединения с каналом P1" <<  endl;
	    		int coidP1 = ConnectAttach(0, getppid(), pChid, _NTO_SIDE_CHANNEL, 0);
	    		if(coidP1 == -1){
	    			cout << "P2: Ошибка соединения с каналом P1" <<  endl;
	    		    exit(EXIT_FAILURE);
	    		}

	    		char rmsg[200] = {};
	    		cout << "Р2: Посылаю сообщение Р1" <<  endl;
	    		char *smsg = (char *)"Р2 stop";
	    		if(MsgSend(coidP1, smsg, strlen(smsg) + 1, rmsg, sizeof(rmsg)) == -1){
	    			cout << "P2: Ошибка MsgSend при отправки в P1" <<  endl;
	    			exit(EXIT_FAILURE);
	    		}

	    		if(strlen(rmsg) > 0){
	    			cout << "P2: Сервер P1 ответил" << endl;
	    		}

	    	}

	    	countMsg ++;
	    }
	}

	cout << "P2: Р2 ОК" << endl;
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
