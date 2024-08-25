#include "../inc/types.hpp"
#include <list>

client &    myList::operator[](long index)
{
    if (index < 0 || (size_t)index > this->size())
        throw std::out_of_range("invalid index");

    long i;
    std::list<client>::iterator it;

    i = 0;
    for (it = this->begin(); it != this->end(); it++)
    {
        if (i == index)
            break;
        i++;
    }
    return (*it);
}

void    myList::remove_from_begin(int index)
{
    // std::cout << "index = " << index << std::endl;
    int i;

    i = 0;
    for (myList::iterator it = this->begin(); it != this->end(); it++)
    {
        if (i == index)
        {
            this->erase(it);
            break;
        }
        i++;
    }
    // std::cout << "i = " << i << std::endl;
}

void    myList::remove_from_end(int index)
{
    int i;

    i = this->size() - 1;
    for (myList::iterator it = this->end(); it != this->begin(); it--)
    {
        if (i == index)
        {
            this->erase(it);
            break;
        }
        i--;
    }
}