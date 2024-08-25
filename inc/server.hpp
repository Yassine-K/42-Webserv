#pragma once

#include <netinet/in.h>
#include <string>
#include <vector>
#include "types.hpp"
#include "error.hpp"

class server
{
    public:
        static  void	parse_config(char* PathToConfig);
		static	int		if_ip_port_already_bound(std::string ip, std::string port);
		static	int		match_server_name(int server_conf_index, std::string server_name_to_match);

        static std::vector<config>									_config;
        static std::vector<std::pair<std::pair<std::string, std::string>, std::vector<std::string> > >	_bound_addresses;

		server(int _conf_index);


		void		init_socket();
		int			get_fd();
		int			get_config_index();
		void		handle_request(int client_index, fd_sets &, int location_index);
		// void		check_if_method_allowed_in_location(int client_index, int location_index);
		std::string	check_if_method_allowed_in_location(int client_index, int location_index);
		std::string	check_availability_of_requested_resource(int client_index, int location_index);
		int         check_resource_type(std::string path);
		void		close_connection(int client_index, fd_sets &);

		myList				_clients;

		bool				_bound;
    private:
        server();
        int					_fd;
        std::string			_ip;
        std::string			_port;
		int					_conf_index;
		sockaddr_in			_addr;
};

enum    path_check
{
    DIRECTORY = 1,
    REG_FILE
};
