#include <cstdlib>
#include <iostream>

#include <limits.h>
#include <unistd.h>

#include <unistd.h>
#define GetCurrentDir getcwd

using std::cin;
using std::cout;
using std::endl;

int nowLocal(){
	char cCurrentPath[FILENAME_MAX];

	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		return EXIT_FAILURE;

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';

	cout << "The current working directory is" << cCurrentPath << endl;
}

int main(int argc, char *argv[]) {
	cout << "Welcome to the QNX Momentics IDE FROM C++" << endl;

	nowLocal();

	cout << "Welcome 33333" << endl;

	return EXIT_SUCCESS;
}

