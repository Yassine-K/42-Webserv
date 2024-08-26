#include "../inc/cgi.hpp"
#include "../inc/client.hpp"
#include <cstddef>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <dirent.h>
#include "../inc/error.hpp"

std::string numberToString(int number) {
  std::stringstream ss;
  ss << number;
  return ss.str();
}

cgi::cgi() : _pid(-1), _exit_status(-1), _cgi_processing(false), _env(NULL), _args(NULL), _first_time(true), _outfile(""),
				_infile("")
{
	this->_fd[0] = -1;
	this->_fd[1] = -1;
}

void	cgi::set_env_variables(request client_req, char** environ)
{
	std::vector<std::string>	temp;

	temp.push_back("REQUEST_METHOD=" + client_req._method);
	temp.push_back("CONTENT_LENGTH=" + numberToString(client_req._content_length));
	temp.push_back("QUERY_STRING=" + client_req._query_params);
	

	size_t	index;
	size_t	j;
	size_t	k;

	index = 0;
	while (environ[index] != NULL)
		index++;

	this->_env = new char*[index + temp.size() + 1];

	j = 0;
	while (j < index)
	{
		this->_env[j] = new char[std::strlen(environ[j]) + 1];
		std::strcpy(this->_env[j], environ[j]);
		j++;
	}
	k = 0;
	while (k < temp.size())
	{
		this->_env[j] = new char[temp[k].size() + 1];
		std::strcpy(this->_env[j], temp[k].c_str());
		j++;
		k++;
	}

	this->_env[temp.size() + index] = NULL;
}

void	cgi::set_args(std::string path)
{
	this->_args = new char*[2];

	this->_args[0] = new char[path.length() + 1];
	std::strcpy(this->_args[0], path.c_str());
	this->_args[1] = NULL;
}

std::string	cgi::get_random_file_name(int client_index, int file_type)
{
	DIR*			directory;
	struct dirent*	entry;
	std::string 	file_name;

	if (file_type == INPUT_FILE)
		file_name = "input";
	else
		file_name = "output";
	while (true)
	{
		// directory = opendir("/home/ykhayri/Desktop/wsl_webserv/");
		// directory = opendir("/tmp");
		directory = opendir("/tmp/post_test");
		if (directory == NULL)
		{
			std::cerr << "Can't open directory" << std::endl;
			throw error(500, client_index);
		}
		while ((entry = readdir(directory)) != NULL)
		{
			if (file_name == entry->d_name)
			{
				file_name += '_';
				closedir(directory);
				break;
			}
		}
		if (entry == NULL)
		{
			closedir(directory);
			break;
		}
	}
	// return ("/tmp/" + file_name);
	return ("/tmp/post_test/" + file_name);
}

// void	cgi::run_cgi(request client_req, std::string path_to_serve, char** environ)
void	cgi::run_cgi(client & cl, char** environ)
{
	unsigned char*	st;

	if (this->_first_time == true)
	{
		this->set_args(cl._response._path_to_serve);
		this->set_env_variables(cl._request, environ);
		this->_outfile = this->get_random_file_name(cl._index, OUTPUT_FILE);

		this->_fd[1] = open(this->_outfile.c_str(), O_CREAT | O_RDWR, 0644);
		if (this->_fd[1] == -1)
		{
			// std::cout << "fd not opened" << std::endl;
			throw error(500, cl._index);
		}

		this->_pid = fork();
		if (this->_pid == -1)
		{
			// std::cout << "pid = -1" << std::endl;
			close(this->_fd[1]);
			throw error(500, cl._index);
		}
		if (this->_pid == 0)
		{
			if (dup2(this->_fd[1], STDOUT_FILENO) == -1)
			{
				close(this->_fd[1]);
				exit(EXIT_FAILURE); // then i catch the exit status and throw 500
			}
			close(this->_fd[1]);
			execve(cl._response._path_to_serve.c_str(), this->_args, this->_env);
			close(_fd[1]);
			exit(EXIT_FAILURE);
		}
		else
		{
			this->_first_time = false;
		}
	}
	else if (this->_first_time == false)
	{
		pid_t	wpid = waitpid(this->_pid, &this->_exit_status, WNOHANG);
		if (wpid == -1)
		{
			// std::cout << "wpid = -1" << std::endl;
			close(this->_fd[1]);
			throw error(500, cl._index);
		}
		if (wpid == 0)
			return ;
		if (wpid == this->_pid)
		{
			st = (unsigned char *)&this->_exit_status;
			
			if (st[0] != 0 || st[1] != 0)
			{
				// std::cout << "st = 1" << std::endl;
				close(this->_fd[1]);
				throw error(500, cl._index);
			}
			else
			{
				close(this->_fd[1]);
				this->_cgi_processing = false;
			}
		}
	}
}

void	cgi::clear_cgi()
{
	int i;

	if (this->_args != NULL)
	{
		i = -1;
		while (this->_args[++i] != NULL)
		{
			delete[] this->_args[i];
			this->_args[i] = NULL;
		}

		delete[] this->_args;
		this->_args = NULL;
	}

	if (this->_env != NULL)
	{
		i = -1;
		while (this->_env[++i] != NULL)
		{
			delete[] this->_env[i];
			this->_env[i] = NULL;
		}

		delete[] this->_env;
		this->_env = NULL;
	}

	// std::remove(this->_outfile.c_str());
	this->_outfile.clear();
	// std::remove(this->_infile.c_str());
	this->_infile.clear();
	this->_fd[0] = -1;
	this->_fd[1] = -1;
	this->_cgi_processing = false;
	this->_first_time = true;
	this->_exit_status = -1;
	this->_pid = -1;
}

cgi::~cgi()
{
	int i;

	if (this->_args != NULL)
	{
		i = -1;
		while (this->_args[++i] != NULL)
		{
			delete[] this->_args[i];
		}

		delete[] this->_args;
	}

	if (this->_env != NULL)
	{
		i = -1;
		while (this->_env[++i] != NULL)
		{
			delete[] this->_env[i];
		}

		delete[] this->_env;
	}
}
