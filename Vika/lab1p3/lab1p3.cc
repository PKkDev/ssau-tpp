#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <string.h>
#include <process.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
	cout << "P3: Запущен" << endl;
	cout << "P3: Параметры: " << "argv[0]=  " << argv[0] <<  endl;

	int pChid = atoi(argv[0]);

	// Id канала для отправки сообщения
	cout << "P3: установление соединения с каналом P1" <<  endl;
	int coidP1 = ConnectAttach(0, getppid(), pChid, _NTO_SIDE_CHANNEL, 0);
	if(coidP1 == -1){
		cout << "P3: Ошибка соединения с каналом P1 " <<  endl;
	    exit(EXIT_FAILURE);
	}

	// Создание канала
	int chid = ChannelCreate(_NTO_CHF_SENDER_LEN);
	cout << "P3: Канал создан: chid = " << chid << endl;

	// Преобразовать число в строку
	char buffer[20];
	const char *chidStr = itoa(chid, buffer, 10);

	// буфер ответа
	char rmsg[200] = {};
	// Послать сообщение
	cout << "Р3: Посылаю сообщение Р1" << rmsg <<  endl;
	int res = MsgSend(coidP1, chidStr, strlen(chidStr) + 1, rmsg, sizeof(rmsg));
	if( res == -1){
		cout << "P3: Ошибка MsgSend при отправки в P1" <<  endl;
		exit(EXIT_FAILURE);
	}

	cout << "Р3: Ожидаю сообщение от Р1" <<  endl;
	bool isWait = true;
	while(isWait){
		// Буфер приема сообщения
		char msg[200] = {};
		// Информация о сообщении
	    _msg_info info;

		// Ссылка на нить клиента
		int rcvid = MsgReceive(chid, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P2: Ошибка MsgReceive" << endl;
	    }

		//if(info.pid == getppid()){
			cout << "P3: Получено сообщение от P1 - " << msg << endl;
			char *msgOk = (char *)"P3 OK";
			MsgReply(rcvid, NULL, msgOk, strlen(msgOk));
			isWait = false;
		//}
	}

	cout << "P3: OK" << endl;
	return EXIT_SUCCESS;
}
