# Syscalls used
-   [mmap](https://www.man7.org/linux/man-pages/man2/mmap.2.html)
-   [openat](https://linux.die.net/man/2/openat)
        if relative path AT_FDCWD searches it relative to current working dir
        if path is absolute its same as open
        returns fd
-   [fstat](https://linux.die.net/man/2/fstat)
        checks file info for given fd returns 0 for success 
-   [rt_sigaction](https://linux.die.net/man/2/rt_sigaction)
        Sigaction but handles more signals
        The SIGURG signal is sent to a process when a socket has urgent 
        or out-of-band data available to read
