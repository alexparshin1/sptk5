// vim:set ts=4:
#include <stdlib.h>
#include <stdio.h>
#include <sptk5/sptk.h>

#ifndef _WIN32
    #include <unistd.h>
#else
    #include <io.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef O_BINARY
# define O_BINARY 0
#endif

int main(int argc, char **argv)
{
	struct stat st;
	int fd;
	int i;
	unsigned char *x;

	if (argc != 3) {
		fprintf(stderr, "Usage: bin2h filename valuename.\n");
		return 1;
	}

	if (stat(argv[1], &st)) {
		fprintf(stderr, "Stat on \"%s\" failed! (errno=%d)\n", argv[1], errno);
		return 1;
	}

#ifdef _WIN32
    fd = _open(argv[1], _O_RDONLY|_O_BINARY);
#else
	fd = open(argv[1], O_RDONLY|O_BINARY);
#endif

	if (fd < 0) {
		fprintf(stderr, "Open on \"%s\" failed! (errno=%d)\n", argv[1], errno);
		return 1;
	}
	
	printf("static size_t %s_len = %ld;\n", argv[2], (long) st.st_size);
	printf("static unsigned char %s[%ld] = {\n", argv[2], (long) st.st_size);

	x = (unsigned char *)malloc(st.st_size);

	if (read(fd, x, st.st_size) != st.st_size) {
		fprintf(stderr, "Read from \"%s\" failed! (errno=%d)\n", argv[1], errno);
		close(fd);
		return 1;
	}

	for (i = 0; i < st.st_size; i++) {
		printf("0x%02x, ", x[i]);
		if (!(i & 0xf)) printf("\n");
	}
	printf("};\n");

	close(fd);
	return 0;
}

