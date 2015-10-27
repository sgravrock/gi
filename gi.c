#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
	int width;
	int col;
} LineState;

static bool cowsay(char **argv);
static void exec_cowsay(int *pipefds);
static bool consume_cowsay(int *pipefds, pid_t pid, char **argv);

static void animate(char **argv);
static int termwidth(void);
static void concat_msg(char *buf, size_t bufsize, char **words);
// All printing functions assume that msg does not contain escape sequences.
static void marquee(LineState *state, int start_col, int end_col, unsigned long delay, const char *msg);
static void bounce(LineState *state, const char *msg);
static void spacegit(LineState *state, const char *msg);
static void move_to_col(LineState *state, int col);
static void move_right(LineState *state, int cols);
static void move_left(LineState *state, int cols);
static void print(LineState *state, const char *msg);

int main(int argc, char **argv) {
	if (argc < 2 || argv[1][0] != 't') {
		fprintf(stderr, "Usage: %s t<git args>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!getenv("STOP_REMINDING_ME_THAT_I_SUCK_AT_TYPING")) {
		if (!cowsay(argv)) {
			animate(argv);
		}
	}

	// abuse argv
	argv[0] = "git";
	argv[1]++;
	execvp("git", argv);
	perror("Couldn't exec git");
	return EXIT_FAILURE;
}

static bool cowsay(char **argv) {
	int pipefds[2];

	if (pipe(pipefds) < 0) {
		perror("pipe");
		return false;
	}

	pid_t pid = fork();

	if (pid < 0) {
		perror("fork");
		return false;
	} else if (pid == 0) {
		exec_cowsay(pipefds);
		exit(EXIT_FAILURE); // unreached unless exec fails
	} else {
		return consume_cowsay(pipefds, pid, argv);
	}
}

static void exec_cowsay(int *pipefds) {
	close(pipefds[1]);

	if (dup2(pipefds[0], 0) < 0) {
		exit(EXIT_FAILURE);
	}

	char *cowfile = getenv("GI_COWFILE");

	if (cowfile && cowfile[0] != '\0') {
		execlp("cowsay", "cowsay", "-f", cowfile, NULL);
	} else {
		execlp("cowsay", "cowsay", NULL);
	}
}

static bool consume_cowsay(int *pipefds, pid_t pid, char **argv) {
	close(pipefds[0]);
	FILE *pp = fdopen(pipefds[1], "w");

	if (!pp) {
		perror("fdopen");
		return false;
	}

	fputs("I think you meant git ", pp);
	fputs(argv[1] + 1, pp); // "tfoo" -> "foo"

	for (char **words = argv + 2; *words; words++) {
		fprintf(pp, " %s", *words);
	}

	if (fclose(pp) != 0) {
		perror("Error writing to cowsay");
	}

	int status;
	waitpid(pid, &status, 0);

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
		return false;
	}

	sleep(1);
	return true;
}

static void animate(char **argv) {
	LineState state = { termwidth(), 0 };
	size_t msgsize = state.width + 1;
	char msg[msgsize];
	concat_msg(msg, msgsize, argv + 1);

	move_to_col(&state, 0);
	print(&state, "gi");
	marquee(&state, state.width - 1, 1, 10000, msg);
	usleep(50000);
	bounce(&state, msg);
	usleep(50000);
	spacegit(&state, msg);
	printf("\n");
}

static int termwidth(void) {
	struct winsize ws;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
	return ws.ws_col;
}

static void concat_msg(char *buf, size_t bufsize, char **words) {
	size_t pos = 0;

	while (*words && pos < bufsize) {
		pos += snprintf(buf + pos, bufsize - pos, "%s ", *words);
		words++;
	}
}

static void marquee(LineState *state, int start_col, int end_col, unsigned long delay, const char *msg) {
	int delta = start_col < end_col ? 1 : -1;

	for (int i = start_col; i != end_col; i += delta) {
		if (delta > 0 && i != start_col) {
			// Blank the space we just vacated
			move_to_col(state, i - 1);
			print(state, " ");
		} else {
			move_to_col(state, i);
		}

		print(state, msg);

		fflush(stdout);
		usleep(delay);
	}
}

static void bounce(LineState *state, const char *msg) {
	for (int i = 0; i < 3; i++) {
		// gi        t<command>
		marquee(state, 2, 10, 20000, msg);
		usleep(500000);

		// git<command>
		marquee(state, 10, 1, 10000, msg);
	}
}

static void spacegit(LineState *state, const char *msg) {
	// git <command>
	move_to_col(state, 3);
	print(state, " ");
	print(state, msg + 1);
}

static void move_to_col(LineState *state, int col) {
	if (state->col < col) {
		move_right(state, col - state->col);
	} else if (state->col > col) {
		move_left(state, state->col - col);
	}
}

static void move_right(LineState *state, int cols) {
	printf("\033[%dC", cols);
	state->col += cols;
}

static void move_left(LineState *state, int cols) {
	printf("\033[%dD", cols);
	state->col -= cols;
}

static void print(LineState *state, const char *msg) {
	int max = state->width - state->col;

	for (int i = 0; i < max && msg[i] != '\0'; i++) {
		putchar(msg[i]);
		state->col++;
	}
}

