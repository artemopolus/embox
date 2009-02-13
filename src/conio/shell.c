/*
 * shell.c
 *
 *  Created on: 02.02.2009
 *      Author: Alexey Fomin
 */

#include "shell.h"
#include "string.h"
#include "tty.h"
#include "mem.h"

static const char* welcome = "monitor> ";

int stub_shell_handler(int argsc, char **argsv) {
	int i;
	if (argsc == 0) {
		printf("Stub! Handler started with no arguments.\n");
		return 0;
	}
	printf("Stub! Handler started with arguments: ");
	for (i = 0; i < argsc; i++) {
		printf("%s%c ", argsv[i], i + 1 == argsc ? '.' : ',');
	}
	printf("\n");
	return 0;
}

int stub_shell_handler2(int argsc, char **argsv) {
	int i;
	if (argsc == 0) {
		printf("Stub2! Handler started with no arguments.\n");
		return 0;
	}
	printf("Stub2! Handler started with arguments: ");
	for (i = 0; i < argsc; i++) {
		printf("%s%c ", argsv[i], i + 1 == argsc ? '.' : ',');
	}
	printf("\n");
	return 0;
}

typedef struct {
	char *name;
	PSHELL_HANDLER phandler;
} SHELL_HANDLER_DESCR;

static SHELL_HANDLER_DESCR shell_handlers[] = {
#include "shell.inc"
		};

// i becomes first non-space symbol index
// makes spaces that skips to zeros
static void skip_spaces_to_zeros(int *i, char *str) {
	while (str[*i] == ' ') {
		str[(*i)++] = 0;
	}
}

// i becomes first non-space symbol index
static void skip_spaces(int *i, char *str) {
	while (str[*i] == ' ') {
		(*i)++;
	}
}

static void tty_callback(char *cmdline) {
	int handler_args_counter = 0;
	int i = 0, j;
	PSHELL_HANDLER phandler;

	if (cmdline != NULL) {

		//		// DEBUG
		//		char *proposals[sizeof(shell_handlers)];
		//		int found = shell_find_commands(cmdline, proposals);
		//
		//		for (i = 0; i < found; i++) {
		//			printf("%s\n", proposals[i]);
		//		}

		char *handler_args[sz_length(cmdline) / 2];

		// cmdline becomes pointer to the first word
		skip_spaces_to_zeros(&i, cmdline);
		cmdline += i;
		i = 0;

		// reading command
		// cmdline points the first word, i - first space (after command name) index
		while (cmdline[i] != 0 && cmdline[i] != ' ') {
			i++;
		}

		// stub! - choosing correct handler
		skip_spaces_to_zeros(&i, cmdline);

		for (j = 0; j < sizeof(shell_handlers); j++) {
			if (sz_cmp(cmdline, shell_handlers[j].name)) {
				phandler = shell_handlers[j].phandler;
				break;
			}
		}
		// if handler not found:
		if (j == sizeof(shell_handlers)) {
			printf("%s: Command not found\n", cmdline);
			return;
		}

		// make cmdline to be a pointer to the first argument if last exists
		cmdline += i;
		i = 0;

		// reading arguments
		while (*cmdline != 0) {
			handler_args[handler_args_counter++] = cmdline;
			while (cmdline[i] != 0 && cmdline[i] != ' ') {
				i++;
			}
			skip_spaces_to_zeros(&i, cmdline);
			cmdline += i;
			i = 0;
		}
		phandler(handler_args_counter, handler_args);

	}
}

int shell_find_commands(char *cmdline, char **proposals) {
	int i = 0, commands_found = 0, j;
	char *searching_command;

	skip_spaces(&i, cmdline);
	searching_command = cmdline + i;

	for (j = 0; j < sizeof(shell_handlers); j++) {
		if (sz_cmp_beginning(searching_command, shell_handlers[j].name)) {
			proposals[commands_found++] = shell_handlers[j].name;
		}
	}
	return commands_found;
}

void shell_start() {
	tty_start(tty_callback, shell_find_commands, welcome);
}

int parse_arg(int argsc, char **argsv, SHELL_KEY *keys) {
	int i = 0, args_count = 0;

	while (i < argsc) {
		if (**(argsv + i) != '-') {
			return -1;
		}

		if (args_count >= MAX_SHELL_KEYS) {
			return -2;
		}
		// Get key name.
		// Second symbol. Ex: -h -x (-hello == -h)
		keys[args_count].name = *(*(argsv + i) + 1);
		keys[args_count].value = 0x0;

		// Now working with value if one exists
		i++;
		if (i < argsc && **(argsv + i) != '-') {
			keys[args_count].value = *(argsv + i);
			i++;
		}
		args_count++;
	}

	return args_count;
}

// Compare keys with available
// returns TRUE if all keys presented are available, FALSE otherwise
int check_keys(SHELL_KEY *keys, int amount, char *available_keys, int amount_of_available_keys) {
	int i, j;

	for (i = 0; i < amount; i++) {
		for (j = 0; j < amount_of_available_keys; j++) {
			if (keys[i].name == available_keys[j]) {
				break;
			}
		}
		if (j >= amount_of_available_keys) {
			return FALSE;
		}
	}
	return TRUE;
}

// Determines whether key was entered
// returns TRUE if key is active (was entered by user) and fills value
int get_key(char key, SHELL_KEY *keys, int keys_amount, char **value) {
	int i;
	for (i = 0; i < keys_amount; i++) {
		if (keys[i].name == key) {
			*value = keys[i].value;
			return TRUE;
		}
	}
	return FALSE;
}
