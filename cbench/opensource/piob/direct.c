#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int
main()
{
	int	fd;
	unsigned n;
	ssize_t	cc;
	static char buf[256 * 1024];

	fd = open("/enfs/tmp/rklundt/foo.dat", O_CREAT|O_WRONLY, 0666);
	if (fd < 0) {
		perror("scratchfile");
		exit(1);
	}
	for (n = 0; n < 400; n++) {
		cc = write(fd, buf, sizeof(buf));
		if (cc != sizeof(buf)) {
			perror("scratchfile+write");
			exit(1);
		}
	}
	if (fsync(fd) != 0) {
		perror("scratchfile+write");
		exit(1);
	}
	if (close(fd) != 0) {
		perror("scratchfile+close");
		exit(1);
	}
	return 0;
}
