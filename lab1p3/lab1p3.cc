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

	cout << "P3: �������" << endl;
	viewWorkDirectory();
	cout << "P3: ���������: " << "argv[0]=  " << argv[0] <<  endl;

	int pChid = atoi(argv[0]);

	// ����� �� P2
	int pidP1;
	int pChidP1;

	// ������������� ���������� � P2 � ���������� ��������� ��� ��������� ���������� � P1 //

	// Id ������ ��� �������� ���������
	cout << "P3: ������������ ���������� � ������� P2" <<  endl;
	int coidP2 = ConnectAttach(0, getppid(), pChid, _NTO_SIDE_CHANNEL, 0);
	if(coidP2 == -1){
		cout << "P3: ������ ���������� � ������� P2" <<  endl;
	    exit(EXIT_FAILURE);
	}

	char rmsg[200] = {};

	// ������� ���������
	cout << "�3: ������� ��������� �2" << rmsg <<  endl;
	char *smsg1 = (char *)"��� ��� pid �������� �1";
	if(MsgSend(coidP2, smsg1, strlen(smsg1) + 1, rmsg, sizeof(rmsg)) == -1){
		cout << "P3: ������ MsgSend ��� �������� � P2" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P3: ������ P2 ������� " << endl;
		pidP1 = atoi(rmsg);
    	cout << "P3: pid �������� �1: " << pidP1 << endl;
	}

	// ������� ���������
	cout << "�3: ������� ��������� �2" <<  endl;
	char *smsg2 = (char *)"��� ��� chid ������ �1";
	if(MsgSend(coidP2, smsg2, strlen(smsg2) + 1, rmsg, sizeof(rmsg)) == -1){
		cout << "P3: ������ MsgSend ��� �������� � P2" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P3: ������ P2 ������� " << rmsg << endl;
		pChidP1 = atoi(rmsg);
    	cout << "P3: chid ������ �1: " << pChidP1 << endl;
	}

	// ������������� ���������� � P2 � ���������� ��������� ��� ��������� ���������� � P1 //

	// ������������� ���������� � P1 � ���������� ��������� //

	// Id ������ ��� �������� ���������
	cout << "P3: ������������ ���������� � ������� P1" <<  endl;
	int coidP1 = ConnectAttach(0, pidP1, pChidP1, _NTO_SIDE_CHANNEL, 0);
	if(coidP1 == -1){
		cout << "P3: ������ ���������� � ������� P1" <<  endl;
	    exit(EXIT_FAILURE);
	}

	// ����� ������
	char rmsg2[200] = {};

	cout << "�3: ������� ��������� �1" <<  endl;
	char *smsg3 = (char *)"�3 send message to P1";
	if(MsgSend(coidP1, smsg3, strlen(smsg3) + 1, rmsg2, sizeof(rmsg2)) == -1){
		cout << "P3: ������ MsgSend ��� �������� � P1" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg2) > 0){
		cout << "P3: ������ P1 ������� - " << rmsg2 << endl;
	}

	// ������������� ���������� � P1 � ���������� ��������� //

	cout << "�3: ������� ��������� �2" <<  endl;
	char *smsg4 = (char *)"�3 stop";
	if(MsgSend(coidP2, smsg4, strlen(smsg4) + 1, rmsg, sizeof(rmsg)) == -1){
		cout << "P3: ������ MsgSend ��� �������� � P2" <<  endl;
		exit(EXIT_FAILURE);
	}

	if(strlen(rmsg) > 0){
		cout << "P3: ������ P2 �������" << endl;
	}

	cout << "P3: �3 ��" << endl;
	return EXIT_SUCCESS;
}

// ����������� ������� ����������
void viewWorkDirectory(){
	char cCurrentPath[FILENAME_MAX];
	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		exit(EXIT_FAILURE);
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	cout << "P3: ������� ������� ���������� - " << cCurrentPath << endl;
}
