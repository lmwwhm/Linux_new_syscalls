#include <sys/types.h>

struct linux_dirent {
	long           d_ino;
	off_t          d_off;
	unsigned short d_reclen;
	char           d_name[14];
};

