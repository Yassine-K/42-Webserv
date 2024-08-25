#pragma once

#include "request.hpp"
#include "response.hpp"
#include "cgi.hpp"
#include "fd_sets.hpp"
#include <fstream>

#define BUFFER_SIZE 4999

class client
{
    public:
        client(int client_sock, int conf_index, int index);

        int         get_fd();
        void        read_request(int config_index, fd_sets&);
        void        fill_request_object();
        void	    set_ready_for_receiving_value(bool value);
		bool&	    get_ready_for_receiving_value();

        int         _index;
        request     _request;
        response    _response;
        cgi         _cgi;

        int         _config_index;
        int         _location_index;

        int         dir_has_index_files();
        void        handle_delete_directory_request(fd_sets&);
        void        does_location_has_redirection();

        void        read_body_based_on_content_length(fd_sets& set_fd);

        void        clear_client();
    private:
        client();

        int             _fd;
        bool		    _ready_for_receiving;
        bool            _read_body;
        size_t          _max_body_size;
        size_t          _bytes_read;
        size_t          _content_length;
        std::fstream   _body_file;
};