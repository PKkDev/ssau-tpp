#include <stdlib.h>
#include <stdio.h>

#include <limits.h>
#include <unistd.h>

#include <unistd.h>
#define GetCurrentDir getcwd

int nowLocal(){
	char cCurrentPath[FILENAME_MAX];

	if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
		return EXIT_FAILURE;

	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */
	printf ("The current working directory is %s \n", cCurrentPath);
}

int main(int argc, char *argv[]) {
	printf("Welcome to the QNX Momentics IDE FROM C\n");

	nowLocal();

	printf("Welcome 33333\n");

	return EXIT_SUCCESS;
}
