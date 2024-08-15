#include "../inc/response.hpp"
#include <dirent.h>
#include <fstream>
#include <sstream>
// #include <sys/_types/_fd_def.h>
#include <cstring>
#include <unistd.h>
#include "../inc/server.hpp"

response::response() : _bytes_sent(0), _status_line(""), _content_length(0), _content_type(""), _location(""), _body(""), _headers(""),
						_chunk(""), _bytes_written(0), _unsent_part("")
{}

response::response(response const & rhs)
{
	std::cout << "dkhel l copy constructor dyal response class" << std::endl;
	(void)rhs;

	this->_body = "";
	this->_bytes_sent = 0;
	this->_bytes_written = 0;
	this->_unsent_part = "";
	this->_chunk = "";
	this->_content_length = 0;
	this->_content_type = "";
	this->_headers = "";
	this->_location = "";
	this->_status_line = "";
	std::cout << "rhs.bytes sent = " << rhs._bytes_sent << std::endl;
	std::cout << "this.bytes sent = " << this->_bytes_sent << std::endl;
}

response &	response::operator=(response const & rhs)
{
	std::cout << "dkhel copy operator dyal response class" << std::endl;
	if (this != &rhs)
	{
		this->_body = "";
		this->_bytes_sent = 0;
		this->_bytes_written = 0;
		this->_unsent_part = "";
		this->_chunk = "";
		this->_content_length = 0;
		this->_content_type = "";
		this->_headers = "";
		this->_location = "";
		this->_status_line = "";
	}
	return (*this);
}

void    response::set_status_line(std::string status_line)
{
    this->_status_line = status_line;
}

void    response::set_content_length(std::ifstream& requested_file)
{
	requested_file.seekg(0, std::ios::end);

	this->_content_length = requested_file.tellg();

	requested_file.seekg(0, std::ios::beg);
}

std::string	response::get_chunk(std::ifstream& requested_file)
{
	std::string	chunk;
	char		buffer[5000];

	memset(buffer, 0, 5000);

	requested_file.read(buffer, 4999);

	chunk.assign(buffer, requested_file.gcount());

	return (chunk);
}

void	response::send_reply(int target_fd)
{
	this->_headers += this->_status_line + "\r\n";

	this->_headers += "Content-Type: " + this->_content_type + "\r\n";

	std::stringstream	ss;

	ss << this->_content_length;

	this->_headers += "Content-Length: " + ss.str() + "\r\n";

	if (!this->_location.empty())
		this->_headers += "Location: " + this->_location + "\r\n";

	this->_headers += "\r\n";
	if (!this->_body.empty())
		this->_headers += this->_body;

	write(target_fd, this->_headers.c_str(), this->_headers.length());
}

void	response::return_error(int status, int target_fd)
{
	if (status == 403)
	{
		this->_status_line = "HTTP/1.1 403 Forbidden";
		this->_body = "<h1>403 Forbidden</h1>";
		this->_content_length = this->_body.length();
	}
	else if (status == 404)
	{
		this->_status_line = "HTTP/1.1 404 Not Found";
		this->_body = "<h1>404 Not found</h1>";
		this->_content_length = this->_body.length();
	}
	else if (status == 405)
	{
		this->_status_line = "HTTP/1.1 405 Method Not Allowed";
		this->_body = "<h1>405 Method Not Allowed</h1>";
		this->_content_length = this->_body.length();
	}

	this->send_reply(target_fd);
	this->clear_response();
}

void	response::clear_response()
{
	this->_status_line.clear();
	this->_content_type.clear();
	this->_content_length = 0;
	this->_location.clear();
	this->_body.clear();
	this->_headers.clear();
	this->_bytes_sent = 0;
	this->_unsent_part.clear();
	this->_chunk.clear();
	this->_bytes_written = 0;
}

void	response::send_response(int fd, config serverConf)
{
	std::cout << "*************** fd = " << fd << std::endl;

	if (!this->_requested_file.is_open())
	{
		this->_status_line = "HTTP/1.1 200 OK";
		this->_requested_file.open(this->_path_to_serve);
		this->set_content_length(this->_requested_file);
		this->_content_type = serverConf.fetch_mime_type_value(this->_path_to_serve);

		this->send_reply(fd);
	}
	else if (this->_requested_file.is_open())
	{
		if (this->_bytes_sent < this->_content_length)
		{
			if (this->_bytes_written == 0)
			{
				this->_chunk = this->get_chunk(this->_requested_file);

				this->_bytes_written = write(fd, this->_chunk.c_str(), this->_chunk.length());

				this->_bytes_sent += this->_bytes_written;
			}

			else if ((size_t)this->_bytes_written < this->_chunk.length())
			{
				long local_write = 0;
				std::cout << "not entirely sent" << std::endl;

				this->_unsent_part = this->_chunk.substr(this->_bytes_written);

				local_write = write(fd, this->_unsent_part.c_str(), this->_unsent_part.length());

				this->_bytes_sent += local_write;
				this->_bytes_written += local_write;
			}

			if ((size_t)this->_bytes_written == this->_chunk.length())
			{
				this->_bytes_written = 0;
				this->_chunk.clear();
				this->_unsent_part.clear();
				std::cout << "chunk sent" << std::endl;
			}

		}
	}
}

void	response::autoindex(int fd, std::string uri)
{
	this->_status_line = "HTTP/1.1 200 OK";
	this->_body = AutoIndex::serve_autoindex(uri, this->_path_to_serve);
	this->_content_length = this->_body.length();

	this->send_reply(fd);
}

void	response::redirect(int fd, std::string uri)
{
	this->_status_line = "HTTP/1.1 301 Moved Permanently";
	this->_content_length = 0;
	this->_location = uri + '/';

	this->send_reply(fd);
}

void	response::remove_uri(int fd, std::string uri, int path_type)
{
	if (path_type == DIRECTORY)
	{
		DIR*			directory;
		struct dirent*	entry;

		directory = opendir(uri.c_str());

		while ((entry = readdir(directory)) != NULL)
		{
			struct stat entry_type;

			stat(uri.c_str(), &entry_type);

			if (S_ISREG(entry_type.st_mode))
			{
				if (std::remove(uri.c_str()) != 0)
				{
					this->return_error(403, fd);
				}
			}
			if (S_ISDIR(entry_type.st_mode))
			{
				this->remove_uri(fd, entry->d_name, DIRECTORY);
			}
		}

	}
	else
	{
		if (std::remove(uri.c_str()) != 0)
		{
			this->return_error(403, fd);
		}
		else
		{
			this->_status_line = "HTTP/1.1 204 No Content";
			this->_body = "<h1>204 No Content</h1>";
			this->_content_length = this->_body.length();

			this->send_reply(fd);
		}
	}
}