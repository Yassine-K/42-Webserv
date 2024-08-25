#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "config.hpp"

class request
{
    public:
        request();
		
        void    	set_request_line(std::string    request_line, int client_index);
        void    	set_header(std::string key, std::string value);

		void		is_well_formed(int client_index);
		int			does_uri_match_location(std::vector<LocationPair> locations, std::string uri_target);
        std::string fetch_header_value(std::string key);
        int         header_exists(std::string key);


		void		clear_request();

    	std::string							_raw_request;
    	std::string							_raw_body;
        std::string                         _method;
        std::string                         _target;
        std::string                         _query_params;
        std::string                         _http_version;
        std::map<std::string, std::string>  _headers;

};