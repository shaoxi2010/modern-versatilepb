#include <errno.h>
#include <reent.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <compiler.h>

/**
 * \brief Stub _close_r() implementation
 *
 * \return -1
 */

WEAK int _close_r(struct _reent *ptr, int fd)
{
	ptr->_errno = EBADF;
	return -1;
}

/**
 * \brief Stub _execve_r() implementation
 *
 * \return -1
 */

WEAK int _execve_r(struct _reent *ptr, const char *pathname,
									char *const argv[], char *const envp[])
{
	ptr->_errno = ENOMEM;
	return -1;
}

/**
 * \brief Stub _exit() implementation
 */

WEAK void _exit(int status)
{
	while (1)
		;
}

/**
 * \brief Stub _fork_r() implementation
 *
 * \return -1
 */

WEAK pid_t _fork_r(struct _reent *ptr)
{

	ptr->_errno = EAGAIN;
	return -1;
}

/**
 * \brief Stub _fstat_r() implementation
 *
 * \return -1
 */

WEAK int _fstat_r(struct _reent *ptr, int fd,
								   struct stat *statbuf)
{
	ptr->_errno = EBADF;
	return -1;
}

/**
 * \brief Stub _getpid_r() implementation
 *
 * \return 1.
 */

WEAK pid_t _getpid_r(struct _reent *ptr) { return 1; }

/**
 * \brief Stub _isatty_r() implementation
 *
 * \return 0
 */

WEAK int _isatty_r(struct _reent *ptr, int fd)
{
	ptr->_errno = EBADF;
	return 0;
}

/**
 * \brief Stub _kill_r() implementation
 *
 * \return -1
 */

WEAK int _kill_r(struct _reent *ptr, pid_t pid, int sig)
{
	ptr->_errno = EINVAL;
	return -1;
}

/**
 * \brief Stub _link_r() implementation
 *
 * \return -1
 */

WEAK int _link_r(struct _reent *ptr, const char *oldpath,
								  const char *newpath)
{
	ptr->_errno = EMLINK;
	return -1;
}

/**
 * \brief Stub _lseek_r() implementation
 *
 * \return -1
 */

WEAK off_t _lseek_r(struct _reent *ptr, int fd, off_t offset,
									 int whence)
{
	ptr->_errno = EBADF;
	return -1;
}

/**
 * \brief Stub _open_r() implementation
 *
 * \return -1
 */

WEAK int _open_r(struct _reent *ptr, const char *pathname,
								  int flags, int mode)
{
	ptr->_errno = EMFILE;
	return -1;
}

/**
 * \brief Stub _read_r() implementation
 *
 * \return -1
 */

WEAK ssize_t _read_r(struct _reent *ptr, int fd, void *buf,
									  size_t count)
{
	ptr->_errno = EBADF;
	return -1;
}

/**
 * \brief Stub _sbrk_r() implementation
 *
 * \return -1
 */

WEAK void *_sbrk_r(struct _reent *ptr, intptr_t increment)
{
	ptr->_errno = ENOMEM;
	return (void *)(-1);
}

/**
 * \brief Stub _stat_r() implementation
 *
 * \return -1
 */

WEAK int _stat_r(struct _reent *ptr, const char *pathname,
								  struct stat *statbuf)
{
	ptr->_errno = ENOENT;
	return -1;
}

/**
 * \brief Stub _times_r() implementation
 *
 * \return -1
 */

WEAK clock_t _times_r(struct _reent *ptr, struct tms *buf)
{
	return -1;
}

/**
 * \brief Stub _unlink_r() implementation
 *
 * \return -1
 */

WEAK int _unlink_r(struct _reent *ptr, const char *pathname)
{
	ptr->_errno = ENOENT;
	return -1;
}

/**
 * \brief Stub _wait_r() implementation
 *
 * \return -1
 */

WEAK pid_t _wait_r(struct _reent *ptr, int *wstatus)
{
	ptr->_errno = ECHILD;
	return -1;
}

/**
 * \brief Stub _write_r() implementation
 *
 * \return -1
 */

WEAK ssize_t _write_r(struct _reent *ptr, int fd,
									   const void *buf, size_t count)
{
	ptr->_errno = EBADF;
	return -1;
}
