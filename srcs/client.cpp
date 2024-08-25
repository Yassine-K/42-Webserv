#include "../inc/client.hpp"
#include <cstddef>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include "../inc/server.hpp"


client::client()
{}

client::client(int client_sock, int conf_index, int index) : _index(index), _config_index(conf_index), _location_index(-2), _fd(client_sock), _ready_for_receiving(false), _read_body(false),
													_max_body_size(0), _bytes_read(0), _content_length(0)
{}

int client::get_fd()
{
    return (this->_fd);
}

void    client::fill_request_object()
{
    std::stringstream	ss(this->_request._raw_request);
	std::string			reader;
	std::string			key;
	std::string			value;
	size_t				pos;

	// need to exctract the first line before entering the loop
	// and check the first word if is it different from
	// GET, POST and DELETE
	getline(ss, reader);
	this->_request.set_request_line(reader, this->_index);

	while (getline(ss, reader))
	{
		for (size_t i = 0; i < reader.size(); i++)
		{
			if (reader[i] == '\r')
				reader.erase(i, 1);
		}		
		pos = reader.find(':');
		if (pos != reader.npos)
		{
			key = reader.substr(0, pos);
			value = reader.substr(pos + 2, reader.size());
			this->_request.set_header(key, value);
		}
	}
}

int	client::dir_has_index_files()
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
			index_files = server::_config[this->_config_index].fetch_location_directive_value(this->_location_index, "index");
		}
		else
		{
			index_files = server::_config[this->_config_index].fetch_directive_value("index");
		}
	}


	for (size_t i = 0; i < index_files.size(); i++)
	{
		access_path = this->_response._path_to_serve + index_files[i];
		std::cout << "access path = '" << access_path << "'" << std::endl;
		if (access(access_path.c_str(), F_OK) == 0) // Default index
		{
			this->_response._path_to_serve = access_path;
			return (1);
		}
		access_path.clear();
	}
	return (0);
}

void	client::does_location_has_redirection()
{
	if (this->_location_index == -1)
	{
		return ;
	}
	else
	{
		std::vector<std::string> return_directive;

		return_directive = server::_config[this->_config_index].fetch_location_directive_value(this->_location_index, "return");
		if (!return_directive.empty())
		{
			this->_response._redirection_path = return_directive.back();
			std::cout << "path to redirect = '" << this->_response._redirection_path << "'" << std::endl;
			throw error(atoi(return_directive.front().c_str()), this->_index);
		}
	}
}

void	client::read_body_based_on_content_length(fd_sets& set_fd)
{
	int		valread = 0;
	char	buffer[BUFFER_SIZE + 1];

	if (this->_cgi._infile.empty())
	{
		this->_cgi._infile = this->_cgi.get_random_file_name(this->_index);
		this->_body_file.open(this->_cgi._infile, std::ios::app);
	}
	if (this->_body_file.is_open())
	{
		if (this->_bytes_read < this->_content_length)
		{
			if (this->_request._raw_body.empty())
			{
				memset(buffer, 0, BUFFER_SIZE + 1);
				valread = read(this->_fd, buffer, BUFFER_SIZE);
				if (valread == 0 || valread == -1)
					throw error(-1, this->_index);
				std::cout << "buffer read from body ---------- :" << std::endl;
				std::cout << buffer << std::endl;

				this->_body_file << buffer;
				this->_bytes_read += valread;
			}
			else
			{
				this->_body_file << this->_request._raw_body;
				this->_bytes_read += this->_request._raw_body.length();
				this->_request._raw_body.clear();
			}
		}
		if (this->_bytes_read == this->_content_length)
		{
			this->_body_file.close();
			FD_SET(this->_fd, & set_fd.write_fds);
		}
	}
	else
	{
		std::cout << "this throw" << std::endl;
		throw error(500, this->_index);
	}
}

void    client::read_request(int conf_index, fd_sets & set_fd)
{
	size_t		pos;
	int			valread = 0;
	char		buffer[BUFFER_SIZE + 1];

	memset(buffer, 0, BUFFER_SIZE + 1);

	if (this->_read_body == false)
	{
		valread = read(this->_fd, buffer, BUFFER_SIZE);

		if (valread == 0 || valread == -1)
			throw error(-1, this->_index);

		std::cout << "------------------ valread = " << valread << std::endl;
		std::cout << "------ buffer read from fd  " << this->_fd << ":" << std::endl;
		std::cout << buffer << std::endl;
		std::cout << "-----------------" << std::endl;

		this->_request._raw_request += buffer;
		pos = this->_request._raw_request.find("\r\n\r\n");

		if (pos != this->_request._raw_request.npos)
		{
			this->fill_request_object();
			this->_request.is_well_formed(this->_index);

			this->_config_index = server::match_server_name(this->_config_index, this->_request.fetch_header_value("host"));
			this->_location_index = this->_request.does_uri_match_location(server::_config[conf_index].get_locations(), this->_request._target);
            
        	this->does_location_has_redirection();

			if (this->_request.header_exists("Transfer-Encoding") || this->_request.header_exists("Content-Length"))
			{
				this->_read_body = true;
				std::stringstream ss(server::_config[this->_config_index].fetch_directive_value("client_max_body_size").front()); // i expect this directive to be present at this point
				ss >> this->_max_body_size;
				if (this->_request.header_exists("Content-Length"))
				{
					std::stringstream	s(this->_request._headers["Content-Length"]);
					s >> this->_content_length;
				}

			}
			else
			{
				FD_SET(this->_fd, &set_fd.write_fds);
				this->_read_body = false;
			}
		}
	}

	if (this->_read_body == true)
	{
		if (buffer[0] != '\0') // In case this is the first time entering this block
		{
			this->_request._raw_body = this->_request._raw_request.substr(pos + 4);
		}
		// if (this->_request.header_exists("Transfer-Encoding")) // handle_chunked_body
		// {
			// else
			// {
			// 	std::cout << "dkhellllll======" << std::endl;
			// 	valread = read(this->_fd, buffer, BUFFER_SIZE);
			// 	this->_rawBody += buffer;

			// 	if (valread == 0)
			// 		throw error(-2, this->_index);
			// }
			// if (valread < BUFFER_SIZE)
			// {
			// 	// std::cout << "full body :" << std::endl;
			// 	// std::cout << "'" << this->_rawBody << "'" << std::endl;
			// 	this->_request.parse_body(this->_rawBody, max_body_size);
			// 	this->_rawBody.clear();
			// 	// FD_SET(this->_fd, &write_fds);
			// }
		// }
		// else
		// {
			this->read_body_based_on_content_length(set_fd);
		// }
	}
}

void	client::handle_delete_directory_request(fd_sets& set_fd)
{
	if (server::_config[this->_config_index].if_cgi_directive_exists(this->_location_index, this->_request._target))
	{
		if (this->dir_has_index_files())
		{
			// run cgi on requested file with DELETE REQUESTED_METHOD
		}
		else
			this->_response.return_error(403, this->_fd);
	}
	else
	{
		this->_response.remove_requested_directory(this->_fd, this->_response._path_to_serve);
	}

	this->clear_client();
	FD_CLR(this->_fd, &set_fd.write_fds);
}

void    client::set_ready_for_receiving_value(bool value)
{
    this->_ready_for_receiving = value;
}

bool&    client::get_ready_for_receiving_value()
{
    return (this->_ready_for_receiving);
}

void	client::clear_client()
{
	this->_ready_for_receiving = false;
	this->_location_index = -2;
	this->_bytes_read = 0;
	this->_content_length = 0;
	this->_read_body = false;
	this->_max_body_size = 0;

	if (this->_body_file.is_open())
		this->_body_file.close();

	this->_request.clear_request();
	this->_response.clear_response();
	this->_cgi.clear_cgi();
}