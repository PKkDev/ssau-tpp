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

// ����������� ������� ����������
void viewWorkDirectory();

int main(int argc, char *argv[]) {
	cout << "P1: �������" << endl;
	viewWorkDirectory();

	int chid = ChannelCreate(_NTO_CHF_SENDER_LEN);
	cout << "P1: ����� ������: chid = " << chid << endl;

	char buffer[20];
	const char *chidStr = itoa(chid, buffer, 10);

	int pidP2 = spawnl(P_NOWAIT, "/home/host/lab1p2/x86/o/lab1p2", chidStr, NULL);
	if (pidP2 < 0){
		 cout << "P1: ������ ������� �������� P2" << pidP2 << endl;
		exit(EXIT_FAILURE);
	}
	cout << "P1: pid �������� P2 - " << pidP2 <<  endl;

	cout << "P1: ����� ��������� ���������" << endl;
	int countMsg = 0;
	while(countMsg < 2){

		char msg[200] = {};
	    _msg_info info;
	    int rcvid;

	    rcvid = MsgReceive(chid, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P1: ������ MsgReceive" << endl;
	    }

	    if (info.pid == pidP2){
	    	cout << "P1: �������� ��������� ��  P2 - " << msg << endl;
	    	MsgReply(rcvid, NULL, msg, sizeof(msg));
	    } else{
		    cout << "P1: �������� ��������� �� " << info.pid << " - " << msg << endl;
		    MsgReply(rcvid, NULL, msg, sizeof(msg));
	    }

	    countMsg++;
	}

	cout << "P1: �1 ��" << endl;
	return EXIT_SUCCESS;
}

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P1: ������� ������� ���������� - " << cCurrentPath << endl;
}
