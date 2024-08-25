#include "../inc/server.hpp"
#include <fstream>
#include <arpa/inet.h> // temp for inet_addr()
// #include <sys/_types/_fd_def.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstdlib>

std::vector<config>    																	server::_config;
std::vector<std::pair<std::pair<std::string, std::string>, std::vector<std::string> > >	server::_bound_addresses;

server::server() : _bound(false)
{}

int	server::match_server_name(int server_conf_index, std::string server_name_to_match)
{
	std::vector<std::string> server_names;
	std::string				 listen_directive;

	listen_directive = server::_config[server_conf_index].fetch_directive_value("listen").front();

	for (size_t i = 0; i < server::_config.size(); i++)
	{
		if (listen_directive == server::_config[i].fetch_directive_value("listen").front())
		{
			server_names = server::_config[i].fetch_directive_value("server_names");
			if (!server_names.empty())
			{
				for (size_t j = 0; j < server_names.size(); j++)
				{
					if (server_name_to_match == server_names[j])
						return (i);
				}
				server_names.clear();
			}
		}
	}
	return (server_conf_index); // if no server name matched just return the server conf index
}

void	server::close_connection(int client_index, fd_sets & set_fd)
{
	std::cout << "closing fd " << this->_clients[client_index].get_fd() << std::endl;

	FD_CLR(this->_clients[client_index].get_fd(), &set_fd.read_fds);

	if (FD_ISSET(this->_clients[client_index].get_fd(), &set_fd.write_fds))
	{
		FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
	}

	close(this->_clients[client_index].get_fd());
	this->_clients.remove_from_begin(client_index);
}

void    server::parse_config(char *PathToConfig)
{
    std::fstream	    		file;
	std::string					reader;
	std::vector<std::string>	listen_directive;

	file.open(PathToConfig);
	if (file.is_open())
	{
    	while (getline(file, reader))
		{
			if (reader == "server")
				server::_config.push_back(config(file));
		}
		file.close();
	}
	// std::cout << "conf size before adding multiple ports = " << server::_config.size() << std::endl;
	for (size_t i = 0; i < server::_config.size(); i++)
	{
		listen_directive = server::_config[i].fetch_directive_value("listen");
		if (listen_directive.size() > 1)
		{
			// std::cout << "multiple addresses:" << std::endl;
			// for (size_t k = 0; k < listen_directive.size(); k++)
			// {
			// 	std::cout << listen_directive[k] << std::endl;
			// }

			for (size_t j = 1; j < listen_directive.size(); j++) // j = 1, to skip the first ip/port
			{
				// std::cout << "listen_directive[j] = '" << listen_directive[j] << "'" << std::endl;
				config	new_conf_block(server::_config[i], listen_directive[j]);
				server::_config.push_back(new_conf_block);
			}
		}
	}

	// check_errors();
}

int	server::if_ip_port_already_bound(std::string ip, std::string port)
{
	for (size_t i = 0; i < server::_bound_addresses.size(); i++)
	{
		if (ip == server::_bound_addresses[i].first.first && port == server::_bound_addresses[i].first.second)
		{
			return (i);
		}
	}
	return (-1);
}

server::server(int conf_index) : _bound(false), _conf_index(conf_index)
{
    int	sock = socket(AF_INET, SOCK_STREAM,0);
	if (sock == -1)
	{
		perror("Error in creating a socket");
		throw 1;
	}

	this->_fd = sock;

	std::vector<std::string>	listenDirective;
	size_t						pos;

	listenDirective = server::_config[_conf_index].fetch_directive_value("listen");

	pos = listenDirective.front().find(':');
	this->_ip = listenDirective.front().substr(0, pos);
	this->_port = listenDirective.front().substr(pos + 1, listenDirective.size() - (pos + 1));

	this->_addr.sin_family = AF_INET;
	this->_addr.sin_addr.s_addr = inet_addr(_ip.c_str());
	this->_addr.sin_port = htons(atoi(_port.c_str()));

    this->init_socket();
}

int	server_name_exists(int index, std::string server_name)
{
	for (size_t i = 0; i < server::_bound_addresses[index].second.size(); i++)
	{
		if (server_name == server::_bound_addresses[index].second[i])
			return (1);
	}
	return (0);
}

void	modify_server_names_in_bound_addresses(int index, std::vector<std::string> server_names)
{
	for (size_t i = 0; i < server_names.size(); i++)
	{
		if (!server_name_exists(index, server_names[i]))
		{
			server::_bound_addresses[index].second.push_back(server_names[i]);
		}
	}
}

void	server::init_socket()
{
	int	bound_address_index;

	bound_address_index = server::if_ip_port_already_bound(this->_ip, this->_port);
	if (bound_address_index != -1)
	{	
		std::vector<std::string> server_names;
		server_names = this->_config[this->_conf_index].fetch_directive_value("server_names");
		if (server_names.empty())
			std::cout << "webserv: [warn] conflicting server name \"\" on " << this->_ip << ":" << this->_port << ", ignored" << std::endl;
		else
		{
			for (size_t i = 0; i < server_names.size(); i++)
			{
				for (size_t j = 0; j < server::_bound_addresses[bound_address_index].second.size(); j++)
				{
					if (server_names[i] == server::_bound_addresses[bound_address_index].second[j])
					{
						std::cout << "webserv: [warn] conflicting server name \"" << server_names[i] << "\" on " << this->_ip << ":" << this->_port << ", ignored" << std::endl;
						break;
					}
				}
			}
		}
		modify_server_names_in_bound_addresses(bound_address_index, server_names);
		return ;
	}

	int	reuseaddr_enable;

	reuseaddr_enable = 1;
	if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_enable, sizeof(reuseaddr_enable)) == -1)
	{
		perror("Error in setting socket option");
		close(this->_fd);
		throw 1;
	}
	if (bind(this->_fd, (struct sockaddr *)&this->_addr, sizeof(this->_addr)) == -1)
	{
		perror("Error binding socket");
		close(this->_fd);
		throw 1;
	}
	if (listen(this->_fd, SOMAXCONN) == -1)
	{
		perror("Error in listening");
		close(this->_fd);
		throw 1;
	}

	// server::_bound_address is a vector of a pair of pair and vector of strings
	server::_bound_addresses.push_back(std::make_pair(std::make_pair(this->_ip, this->_port), this->_config[this->_conf_index].fetch_directive_value("server_names")));
	this->_bound = true;
}

int server::get_fd()
{
    return (this->_fd);
}

int server::get_config_index()
{
    return (this->_conf_index);
}

std::string server::check_availability_of_requested_resource(int client_index, int location_index)
{
    std::vector<std::string>	root_directive_inside_location;
    std::string					full_path;

	
	if (location_index == -1)
		full_path = server::_config[this->_conf_index].fetch_directive_value("root").front();
	else
	{
		root_directive_inside_location = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "root");

		if (root_directive_inside_location.size() == 0)
            full_path = server::_config[this->_conf_index].fetch_directive_value("root").front();

		else
			full_path = root_directive_inside_location.front();

	}

	full_path += this->_clients[client_index]._request._target;
	std::cout << "full path =====----> '" << full_path << "'" << std::endl;

	if (access(full_path.c_str(), F_OK) == -1)
		return ("");

	return (full_path);
}

std::string    server::check_if_method_allowed_in_location(int client_index, int location_index)
{
	LocationPair				location_block;
    std::vector<std::string>    allowed_methods;

	if (location_index == -1)
	{
		allowed_methods = server::_config[this->_conf_index].fetch_directive_value("allowed_methods");

		for (size_t i = 0; i < allowed_methods.size(); i++)
		{
			if (this->_clients[client_index]._request._method == allowed_methods[i])
			{
				// std::cout << "ALLOWED FROM THE SERVER CONF NOT LOCATION" << std::endl;
				return (allowed_methods[i]);
			}
		}
	}
	else
	{
        allowed_methods = server::_config[this->_conf_index].fetch_location_directive_value(location_index, "allowed_methods");

        for (size_t i = 0; i < allowed_methods.size(); i++)
        {
            if (this->_clients[client_index]._request._method == allowed_methods[i])
            {
                // std::cout << "ALLOWEEEEEEED!!!!!" << std::endl;
				return (allowed_methods[i]);
            }
        }
	}
	throw error(405, client_index); // 405 Method Not Allowed
}

int	server::check_resource_type(std::string path)
{
	struct stat	path_stat;

	stat(path.c_str(), &path_stat);

	if (S_ISDIR(path_stat.st_mode))
	{
		return (DIRECTORY);
	}
	return (REG_FILE);
}

void    server::handle_request(int client_index, fd_sets& set_fd, int location_index)
{
	std::string	req_method;

	req_method = this->check_if_method_allowed_in_location(client_index, location_index);
	
	this->_clients[client_index]._response._path_to_serve = this->check_availability_of_requested_resource(client_index, location_index);
	
	if (this->_clients[client_index]._response._path_to_serve.empty()) // in the previous function if the path doesn't exist i return an empty string
	{
		this->_clients[client_index]._response.return_error(404, this->_clients[client_index].get_fd());
		std::cout << "404 SERVED!!!" << std::endl;
		this->_clients[client_index].clear_client();
		FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
	}
	else
	{
		if (this->check_resource_type(this->_clients[client_index]._response._path_to_serve) == DIRECTORY)
		{
			std::cout << "last char = '" << this->_clients[client_index]._request._target[this->_clients[client_index]._request._target.length() - 1] << "'" << std::endl;
			if (this->_clients[client_index]._request._target[this->_clients[client_index]._request._target.length() - 1] == '/')
			{
				if (req_method == "DELETE")
				{
					this->_clients[client_index].handle_delete_directory_request(set_fd);
					return ;
				}
				std::cout << "path before index = '" << this->_clients[client_index]._response._path_to_serve << "'" << std::endl;
				if (this->_clients[client_index].dir_has_index_files()) // this method modifies on path_to_serve attribute
				{
					if (server::_config[this->_clients[client_index]._config_index].if_cgi_directive_exists(this->_clients[client_index]._location_index, this->_clients[client_index]._response._path_to_serve))
					{
						this->_clients[client_index]._cgi._cgi_processing = true;
					}
					else
					{
						if (req_method == "POST")
						{
							std::cout << "cgi not found" << std::endl;
							throw error(403, client_index); // Forbidden
						}

						this->_clients[client_index].set_ready_for_receiving_value(true);
					}
				}
				else
				{
					int autoindex_check = server::_config[this->_clients[client_index]._config_index].fetch_autoindex_value(location_index);

					if (req_method == "POST" || autoindex_check != ON)
					{
						std::cout << "before throw 403**" << std::endl;
						throw error(403, client_index); // Forbidden
					}
					else
					{
						this->_clients[client_index]._response.autoindex(this->_clients[client_index].get_fd(), this->_clients[client_index]._request._target);
						this->_clients[client_index].clear_client();
						FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
						std::cout << "autoindex served !!" << std::endl;
					}
				}
			}
			else
			{
				if (req_method == "DELETE")
					throw error(409, client_index); // Conflict

				this->_clients[client_index]._response.redirect(this->_clients[client_index].get_fd(), 301, this->_clients[client_index]._request._target + '/');
				this->_clients[client_index].clear_client();
				FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
			}
		}
		else
		{
			if (server::_config[this->_clients[client_index]._config_index].if_cgi_directive_exists(this->_clients[client_index]._location_index, this->_clients[client_index]._request._target))
			{
				this->_clients[client_index]._cgi._cgi_processing = true;
			}
			else
			{
				std::string	method = req_method;

				if (method == "POST")
					throw error(403, client_index); // Forbidden

				else if (method == "DELETE")
				{
					this->_clients[client_index]._response.remove_requested_file(this->_clients[client_index].get_fd());

					this->_clients[client_index].clear_client();
					FD_CLR(this->_clients[client_index].get_fd(), &set_fd.write_fds);
				}
				else
				{
					this->_clients[client_index].set_ready_for_receiving_value(true);
				}

			}
		}
	}
}
