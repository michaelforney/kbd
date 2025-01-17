/*
 * disalloc.c - aeb - 940501 - Disallocate virtual terminal(s)
 * Renamed deallocvt.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/vt.h>
#include <getopt.h>
#include <sysexits.h>

#include "libcommon.h"

static void __attribute__((noreturn))
usage(int rc)
{
	fprintf(stderr, _("Usage: %s [option...] [N ...]\n"
	                  "\n"
	                  "Valid options are:\n"
	                  "\n"
	                  "  -h --help          display this help text\n"
	                  "  -V --version       print version number\n"),
		get_progname());
	exit(rc);
}

int main(int argc, char *argv[])
{
	int fd, num, i;

	const char *const short_opts = "hV";
	const struct option long_opts[] = {
		{ "help",    no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'V' },
		{ NULL, 0, NULL, 0 }
	};

	if (argc < 1) /* unlikely */
		return EXIT_FAILURE;
	set_progname(argv[0]);
	setuplocale();

	while ((i = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1) {
		switch (i) {
			case 'V':
				print_version_and_exit();
				break;
			case 'h':
				usage(EXIT_SUCCESS);
			case '?':
				usage(EX_USAGE);
		}
	}

	for (i = optind; i < argc; i++) {
		if (!isdigit(argv[i][0])) {
			fprintf(stderr, _("%s: unknown option\n"), get_progname());
			return EX_USAGE;
		}
	}

	if ((fd = getfd(NULL)) < 0)
		kbd_error(EXIT_FAILURE, 0, _("Couldn't get a file descriptor referring to the console"));

	if (argc == 1) {
		/* deallocate all unused consoles */
		if (ioctl(fd, VT_DISALLOCATE, 0)) {
			kbd_error(EXIT_FAILURE, errno, "ioctl VT_DISALLOCATE");
		}
	} else
		for (i = 1; i < argc; i++) {
			num = atoi(argv[i]);
			if (num == 0) {
				kbd_error(EXIT_FAILURE, 0, _("0: illegal VT number\n"));
			} else if (num == 1) {
				kbd_error(EXIT_FAILURE, 0, _("VT 1 is the console and cannot be deallocated\n"));
			} else if (ioctl(fd, VT_DISALLOCATE, num)) {
				kbd_error(EXIT_FAILURE, errno, _("could not deallocate console %d: "
				                                 "ioctl VT_DISALLOCATE"),
				          num);
			}
		}
	exit(EXIT_SUCCESS);
}
