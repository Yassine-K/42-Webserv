#pragma once

#include "server.hpp"
#include <iostream>


class   webserv
{
    public:
        static void launch_server(char** env);
        static std::vector<server>  servers;
        static void serve_clients(fd_sets & set_fd, char** env);
};
