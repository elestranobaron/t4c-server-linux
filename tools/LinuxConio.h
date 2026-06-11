/* Minimal kbhit/getch for POSIX (MSVC conio.h compatibility). */
#ifndef T4C_LINUX_CONIO_H
#define T4C_LINUX_CONIO_H

#ifndef _WIN32

#include <poll.h>
#include <unistd.h>

inline int kbhit(void) {
	struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
	return poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN);
}

inline int getch(void) {
	unsigned char c = 0;
	const ssize_t n = read(STDIN_FILENO, &c, 1);
	return n == 1 ? static_cast<int>(c) : -1;
}

#endif /* !_WIN32 */

#endif /* T4C_LINUX_CONIO_H */
