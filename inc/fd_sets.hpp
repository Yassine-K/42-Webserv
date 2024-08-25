#pragma once

#include <sys/select.h>

typedef struct fd_sets
{
    fd_set	read_fds;
    fd_set	write_fds;
    fd_set	read_fds_tmp;
    fd_set	write_fds_tmp;

    void    clear_sets();

}   fd_sets;