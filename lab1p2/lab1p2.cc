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

	cout << "P2: �������" << endl;
	viewWorkDirectory();
	cout << "P2: ���������: " << "argv[0]=  " << argv[0] <<  endl;

	int chid = ChannelCreate(_NTO_CHF_SENDER_LEN);
	cout << "P2: ����� ������: chid = " << chid << endl;

	char buffer[20];
	const char *chidStr = itoa(chid, buffer, 10);

	int pidP3 = spawnl(P_NOWAIT, "/home/host/lab1p3/x86/o/lab1p3", chidStr, NULL);
	if (pidP3 < 0){
		cout << "P2: ������ ������� �������� P3" << pidP3 << endl;
		exit(EXIT_FAILURE);
	}
	cout << "P2: pid �������� P3 - " << pidP3 <<  endl;

	cout << "P2: ����� ��������� ���������" << endl;

	int countMsg = 0;
	while(countMsg < 3){

		char msg[200] = {};
	    _msg_info info;
	    int rcvid;

	    rcvid = MsgReceive(chid, msg, sizeof(msg), &info);
	    if(rcvid == -1){
	    	 cout << "P2: ������ MsgReceive" << endl;
	    }

	    if (info.pid == pidP3){

	    	cout << "P2: �������� ��������� �� P3 - " << msg << endl;

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

	    		cout << "P2: ������������ ���������� � ������� P1" <<  endl;
	    		int coidP1 = ConnectAttach(0, getppid(), pChid, _NTO_SIDE_CHANNEL, 0);
	    		if(coidP1 == -1){
	    			cout << "P2: ������ ���������� � ������� P1" <<  endl;
	    		    exit(EXIT_FAILURE);
	    		}

	    		char rmsg[200] = {};
	    		cout << "�2: ������� ��������� �1" <<  endl;
	    		char *smsg = (char *)"�2 stop";
	    		if(MsgSend(coidP1, smsg, strlen(smsg) + 1, rmsg, sizeof(rmsg)) == -1){
	    			cout << "P2: ������ MsgSend ��� �������� � P1" <<  endl;
	    			exit(EXIT_FAILURE);
	    		}

	    		if(strlen(rmsg) > 0){
	    			cout << "P2: ������ P1 �������" << endl;
	    		}

	    	}

	    	countMsg ++;
	    }
	}

	cout << "P2: �2 ��" << endl;
	return EXIT_SUCCESS;

}

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P2: ������� ������� ���������� - " << cCurrentPath << endl;
}
