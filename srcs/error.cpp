#include "../inc/error.hpp"

error::error()
{}

error::error(int status, int client_index) throw() : _status(status), _client_index(client_index)
{}

error::error(const error & rhs) throw()
{
    this->_client_index = rhs._client_index;
    this->_status = rhs._status;
}
