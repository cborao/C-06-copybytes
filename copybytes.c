
// JULIO 2020. CÉSAR BORAO MORATINOS; copybytes.c

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

enum {
	Buffsize = 8000
};

// return the converted value if success or -1 if failed
int
convert(char *arg) {

	char *endptr;
	long var;
	var = strtol(arg,&endptr,10);
	if (endptr == arg)
		return -1;
	return var;
}

void
copy(int infile, int outfile, int offset, int count) {

	int nr;
	int rest = count;
	char buff[Buffsize];

	if (infile != 0) {
		if (lseek(infile,offset,SEEK_SET) < 0)
			errx(EXIT_FAILURE, "error: offset cannot set");
	}

	while ((rest > Buffsize) && ((nr = read(infile,&buff,Buffsize)) != 0)) {
		rest -= nr;
		if (nr < 0)
			errx(EXIT_FAILURE, "error: cannot read");

		if (write(outfile,&buff,nr) != nr)
			errx(EXIT_FAILURE, "error: cannot write");
	}

	if (rest <= Buffsize) {
		int i = 0;
		while ((nr = read(infile,&buff,rest-i)) != 0) {
			i += nr;
			if (nr < 0)
				errx(EXIT_FAILURE, "error: cannot read");

			if (write(outfile,&buff,nr) != nr)
				errx(EXIT_FAILURE, "error: cannot write");
		 }
	}
}

// return input file fd or -1 if error
int
selecin(char *input) {
	int infile;
	if (strcmp(input,"-") == 0)
		return 0;

	if (access(input,R_OK) < 0)
		errx(EXIT_FAILURE,"error: cannos access file %s ", input);

	if ((infile = open(input, O_RDONLY)) < 0)
		return -1;

	return infile;
}

int
selectout(char *output) {
	int outfile;
	if (strcmp(output,"-") == 0)
		return 1;

	if ((outfile = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0640)) < 0)
		return -1;

	return outfile;
}

int
main(int argc, char *argv[]) {

	argv++;
	argc--;

	if (argc != 4)
		errx(EXIT_FAILURE, "usage: copybytes [origin] [destination] [offset] [bytes]");

	int offset, count, infile, outfile;

	if ((offset = convert(argv[2])) < 0)
		errx(EXIT_FAILURE,"error: strtol() failed");

	if ((count = convert(argv[3])) < 0)
		errx(EXIT_FAILURE,"error: strtol() failed");

	// select input file
	if ((infile = selecin(argv[0])) < 0)
		errx(EXIT_FAILURE, "error: open %s failed",argv[0]);

	// select output file
	if ((outfile = selectout(argv[1])) < 0)
		errx(EXIT_FAILURE, "error: open %s failed",argv[1]);

	copy(infile,outfile,offset,count);

	if (infile != 0) {
		if (close(infile) < 0)
			errx(EXIT_FAILURE,"error: closing fd %d", infile);
	}

	if (outfile != 1) {
		if (close(outfile) < 0)
			errx(EXIT_FAILURE,"error: closing fd %d", outfile);
	}

	exit(EXIT_SUCCESS);
}
