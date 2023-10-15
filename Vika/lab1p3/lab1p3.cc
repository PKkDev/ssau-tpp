#include <cstdlib>
#include <iostream>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <string.h>
#include <process.h>

using std::cout;
using std::endl;

int main(int argc, char *argv[]) {
	cout << "P3: �������" << endl;
	cout << "P3: ���������: " << "argv[0]=  " << argv[0] <<  endl;

	int pChid = atoi(argv[0]);

	// Id ������ ��� �������� ���������
	cout << "P3: ������������ ���������� � ������� P1" <<  endl;
	int coidP1 = ConnectAttach(0, getppid(), pChid, _NTO_SIDE_CHANNEL, 0);
	if(coidP1 == -1){
		cout << "P3: ������ ���������� � ������� P1 " <<  endl;
	    exit(EXIT_FAILURE);
	}

	// �������� ������
	int chid = ChannelCreate(_NTO_CHF_SENDER_LEN);
	cout << "P3: ����� ������: chid = " << chid << endl;

	// ������������� ����� � ������
	char buffer[20];
	const char *chidStr = itoa(chid, buffer, 10);

	// ����� ������
	char rmsg[200] = {};
	// ������� ���������
	cout << "�3: ������� ��������� �1" << rmsg <<  endl;
	int res = MsgSend(coidP1, chidStr, strlen(chidStr) + 1, rmsg, sizeof(rmsg));
	if( res == -1){
		cout << "P3: ������ MsgSend ��� �������� � P1" <<  endl;
		exit(EXIT_FAILURE);
	}

	cout << "�3: ������ ��������� �� �1" <<  endl;
	bool isWait = true;
	while(isWait){
		// ����� ������ ���������
		char msg[200] = {};
		// ���������� � ���������
	    _msg_info info;

		// ������ �� ���� �������
		int rcvid = MsgReceive(chid, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P2: ������ MsgReceive" << endl;
	    }

		//if(info.pid == getppid()){
			cout << "P3: �������� ��������� �� P1 - " << msg << endl;
			char *msgOk = (char *)"P3 OK";
			MsgReply(rcvid, NULL, msgOk, strlen(msgOk));
			isWait = false;
		//}
	}

	cout << "P3: OK" << endl;
	return EXIT_SUCCESS;
}
