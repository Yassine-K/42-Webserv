#pragma once

#include <list>
#include "client.hpp"

class client;

class myList : public std::list<client>
{
    public:
        client &    operator[](long);
        void        remove_from_begin(int index);
        void        remove_from_end(int index);
};
