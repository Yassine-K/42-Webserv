#include "../inc/client.hpp"
#include <cstddef>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include "../inc/server.hpp"


client::client()
{}

client::client(int client_sock, int conf_index) : _config_index(conf_index), _location_index(-2), _fd(client_sock), _ready_for_receiving(false)
{}

int client::get_fd()
{
    return (this->_fd);
}

void    client::fill_request_object()
{
    std::stringstream	ss(this->_request.get_rawRequest());
	std::string			reader;
	std::string			key;
	std::string			value;
	size_t				pos;


	while (getline(ss, reader))
	{
		for (size_t i = 0; i < reader.size(); i++)
		{
			if (reader[i] == '\r')
				reader.erase(i, 1);
		}

		if (reader.find("HTTP/") != reader.npos) // temp
			this->_request.set_request_line(reader);
		
		pos = reader.find(':');
		if (pos != reader.npos)
		{
			key = reader.substr(0, pos);
			value = reader.substr(pos + 2, reader.size());
			this->_request.set_header(key, value);
		}
	}
}

int	client::if_cgi_directive_exists()
{
	if (this->_location_index == -1)
	{
		if (server::_config[this->_config_index].directive_exists("cgi"))
			return (1);
	}
	else
	{
		if (server::_config[this->_config_index].directive_inside_location_exists(this->_location_index, "cgi"))
			return (1);
	}
	return (0);
}

int	client::dir_has_index_files(std::string & path)
{
	std::vector<std::string>	index_files;
	std::string					access_path;

	if (this->_location_index == -1)
	{
		index_files = server::_config[this->_config_index].fetch_directive_value("index");
	}
	else
	{
		if (server::_config[this->_config_index].directive_inside_location_exists(this->_location_index, "index"))
		{
			index_files = server::_config[this->_config_index].fetch_location_directive_value(this->_location_index, "index"); // temp because we should loop over the whole vector
		}
		else
		{
			index_files = server::_config[this->_config_index].fetch_directive_value("index");
		}
	}


	for (size_t i = 0; i < index_files.size(); i++)
	{
		access_path = path + index_files[i];
		if (access(access_path.c_str(), F_OK) == 0) // Default index
		{
			path = access_path;
			return (1);
		}
		access_path.clear();
	}

	return (0);
}

void    client::read_request(int conf_index, fd_sets & set_fd)
{
    // int			max_body_size;
	size_t		pos;
	int			valread = 0;
	char		buffer[BUFFER_SIZE + 1];
	static int	read_body;

	memset(buffer, 0, BUFFER_SIZE + 1);

	if (read_body == 0)
	{
		valread = read(this->_fd, buffer, BUFFER_SIZE);

		if (valread == 0 || valread == -1)
			throw -1;

		std::cout << "------------------ valread = " << valread << std::endl;
		std::cout << "------ buffer read from fd  " << this->_fd << ":" << std::endl;
		std::cout << buffer << std::endl;
		std::cout << "-----------------" << std::endl;

        // sleep(2)
;

		this->_request.set_raw_request(buffer);
		pos = this->_request.get_rawRequest().find("\r\n\r\n");

		if (pos != this->_request.get_rawRequest().npos)
		{
			this->fill_request_object();

			this->_config_index = server::match_server_name(this->_config_index, this->_request.fetch_header_value("host"));

			this->_request.is_well_formed(server::_config[conf_index]);
			
			this->_location_index = this->_request.does_uri_match_location(server::_config[conf_index].get_locations(), this->_request.get_target());
            
			// std::cout << "location index ===>> " << location_index << std::endl;
            // server::_config[conf_index].does_location_has_redirection(location_index);

			// std::string	transfer_encoding = this->_request.retrieve_header_value("Transfer-Encoding");
			// if (!this->_request.retrieve_header_value("Transfer-Encoding").empty())
			// {
			// 	read_body = 1;
			// }
			// else
			// {
				FD_SET(this->_fd, &set_fd.write_fds);
			// }
		}
	}

	// std::stringstream ss(serverConf.fetch_directive_value("client_max_body_size").front());
	// ss >> max_body_size;

	// if (read_body == 1)
	// {
	// 	if (buffer[0] != '\0') // first time entering this block
	// 	{
	// 		this->_rawBody = this->_rawRequest.substr(pos + 4, this->_rawRequest.length()); // pos + 4 to skip "\r\n\r\n"
	// 		std::cout << "raw_body = '" << this->_rawBody << "'" << std::endl;
	// 		this->_rawRequest.clear();
	// 	}
	// 	else
	// 	{
	// 		std::cout << "dkhellllll======" << std::endl;
	// 		valread = read(this->_fd, buffer, BUFFER_SIZE);
	// 		this->_rawBody += buffer;

	// 		if (valread == 0)
	// 			throw -2;
	// 	}
	// 	if (valread < BUFFER_SIZE)
	// 	{
	// 		// std::cout << "full body :" << std::endl;
	// 		// std::cout << "'" << this->_rawBody << "'" << std::endl;
	// 		this->_request.parse_body(this->_rawBody, max_body_size);
	// 		this->_rawBody.clear();
	// 		// FD_SET(this->_fd, &write_fds);
	// 	}
	// }
}

void	client::handle_delete_request(std::string & path)
{
	if (this->if_cgi_directive_exists())
	{
		if (this->dir_has_index_files(path))
		{
			// run cgi on requested file with DELETE REQUESTED_METHOD
		}
		else
			this->_response.return_error(403, this->_fd);
	}
	else
	{
		this->_response.remove_uri(this->_fd, this->_request.get_target(), DIRECTORY);
	}
}

void    client::set_ready_for_receiving_value(bool value)
{
    this->_ready_for_receiving = value;
}

bool    client::get_ready_for_receiving_value()
{
    return (this->_ready_for_receiving);
}
