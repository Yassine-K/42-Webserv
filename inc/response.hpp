#pragma once

#include "config.hpp"
#include <fstream>
#include <string>
#include "autoindex.hpp"
#include <iostream>

class response
{
    public:
        response();
        response(response const &);
        response &  operator=(response const &);

        void            set_status_line(std::string);
        void            set_content_length(std::ifstream&);
        void            set_location(std::string);
        void            set_body(std::string);

        std::string     get_chunk(std::ifstream&);

        void            send_reply(int target_fd);

        void            return_error(int status, int target_fd);
        void            clear_response();


        void            send_response(int fd, config serverConf);
        void            autoindex(int fd, std::string uri);
        void            redirect(int fd, int status, std::string uri);
        int             remove_requested_directory(int fd, std::string uri);
        void            remove_requested_file(int fd);

        void            send_cgi_headers(int fd, std::ifstream&);

        /*=============== ATTRIBUTES ===============*/

        long long       _bytes_sent;
        std::string     _status_line;
        long long       _content_length;
        std::string     _content_type;
        std::string     _location;
        std::string     _body;
        std::string     _headers;
        std::string     _chunk;
        long            _bytes_written;
        std::string     _unsent_part;
        std::ifstream   _requested_file;
        std::string     _path_to_serve;
        std::string     _redirection_path;

};