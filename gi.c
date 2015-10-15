#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv) {
	if (argc < 2 || argv[1][0] != 't') {
		fprintf(stderr, "Usage: %s t<git args>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// abuse argv
	argv[0] = "git";
	argv[1]++;
	execvp("git", argv);
	perror("Couldn't exec git");
	return EXIT_FAILURE;
}
