#include <cstdlib>
#include <iostream>

#include <stdlib.h>
#include <sys/neutrino.h>
#include <process.h>
#include <string.h>

using std::cout;
using std::endl;

// ���������� ��������� �� ����������� ������
void sendMessageToP(int chid, int pid, char *pX);

int main(int argc, char *argv[]) {
	cout << "P1: �������" << endl;

	// �������� ������
	int chid = ChannelCreate(_NTO_CHF_SENDER_LEN);
	cout << "P1: ����� ������: chid = " << chid << endl;

	// ������������� ����� � ������
	char buffer[20];
	const char *chidStr = itoa(chid, buffer, 10);

	// ����� ��������� �������� P2
	int pidP2 = spawnl( P_NOWAIT, "/home/host/vika/lab1p2/x86/o/lab1p2", chidStr, NULL);
	if (pidP2 < 0){
		cout << "P1: ������ ������� �������� P2" << strerror(pidP2) << endl;
		exit(EXIT_FAILURE);
	}
	cout << "P1: pid �������� P2 - " << pidP2 <<  endl;

    // ����� ��������� �������� P2
	int pidP3 = spawnl( P_NOWAIT, "/home/host/vika/lab1p3/x86/o/lab1p3", chidStr, NULL);
	if (pidP3 < 0){
		cout << "P1: ������ ������� �������� P3" << strerror(pidP3) << endl;
		exit(EXIT_FAILURE);
	}
	cout << "P1: pid �������� P3 - " << pidP3 <<  endl;

	int chidP2;
	int chidP3;

	bool isWaitP2 = true;
	bool isWaitP3 = true;
	while(isWaitP2 || isWaitP3){

		// ����� ������ ���������
		char msg[200] = {};
		// ���������� � ���������
	    _msg_info info;

		// ������ �� ���� �������
		int rcvid = MsgReceive(chid, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P1: ������ MsgReceive" << endl;
	    }

		if(info.pid == pidP2){
			cout << "P1: �������� ��������� ��  P2 - " << msg << endl;
			MsgReply(rcvid, NULL, msg, sizeof(msg));
			chidP2 = atoi(msg);
			isWaitP2 = false;
		}

		if(info.pid == pidP3){
			cout << "P1: �������� ��������� ��  P3 - " << msg << endl;
			MsgReply(rcvid, NULL, msg, sizeof(msg));
			chidP3 = atoi(msg);
			isWaitP3 = false;
		}
	}

	sendMessageToP(chidP2, pidP2, (char *)"P2");
	sendMessageToP(chidP3, pidP3, (char *)"P3");

	cout << "P1: OK" << endl;
	return EXIT_SUCCESS;
}

// ���������� ��������� �� ����������� ������
void sendMessageToP(int chid, int pid, char *pX){
	char rmsg[100];

	cout << "P1: ������������ ���������� � ������� " << pX <<  endl;
	int coidP0 = ConnectAttach(0, pid, chid, _NTO_SIDE_CHANNEL, 0);
	if(coidP0 == -1){
		cout << "P1: ������ ���������� � ������� " << pX << " - " << strerror(coidP0) <<  endl;
	    exit(EXIT_FAILURE);
	}

	char *smsg = (char *)"�1 send message to ";

	char result[100] = {};
	strcat(result, smsg);
	strcat(result, pX);

	cout << "�1: ������� ��������� " << pX << " - " << result <<  endl;
	int sendRes = MsgSend(coidP0, result, strlen(result) + 1, rmsg, sizeof(rmsg));
	if(sendRes == -1){
		cout << "P1: ������ MsgSend ��� �������� � " << pX << " - " << strerror(sendRes) <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P1: " << pX << " ������� - " << rmsg << endl;
	}
}
