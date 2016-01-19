#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <aio.h>

#include "uthread.h"

int async_read(int fd, void *buf, size_t count)
{
	int retval = 0;
	off_t offset = lseek(fd, 0, SEEK_CUR);

	// some error
	if (offset == -1 && errno != ESPIPE)
		return -errno;

	// just a non-seekable file
	if (offset == -1 && errno == ESPIPE)
		offset = 0;

	struct aiocb readreq = {
		.aio_fildes = fd,
		.aio_buf = buf,
		.aio_nbytes = count,
		.aio_offset = offset,
		.aio_sigevent = {.sigev_notify = SIGEV_NONE}
	};

	retval = aio_read(&readreq);
	if (retval == -1)
		return errno;

	// check for short reads or errors and return without yield
	retval = aio_error(&readreq);

	if (retval == 0)
		goto done;

	if (retval != EINPROGRESS)
		return retval;

	do {
		uthread_yield();

		retval = aio_error(&readreq);
	} while (retval == EINPROGRESS);

 done:
	retval = aio_return(&readreq);

	if (retval > 0)
		if (offset)
			lseek(fd, offset + retval, SEEK_SET);

	return retval;
}
