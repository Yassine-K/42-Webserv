#pragma once

#include <exception>

class error : public std::exception
{
    public:
        error(int status, int client_index) throw();
        error(error const &) throw();
        int _status;
        int _client_index;
    private:
        error();
};