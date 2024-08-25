#pragma once

#include <vector>
#include "request.hpp"

class client;

class cgi
{
	public:
		cgi();
		~cgi();

		int							_fd[2];
		pid_t						_pid;
		int							_exit_status;

		bool						_cgi_processing;
		char**						_env;
		char**						_args;
		bool						_first_time;
		std::string					_outfile;
		std::string					_infile;

		void		set_env_variables(request, char**);
		void		set_args(std::string path);
		void		run_cgi(client & cl, char** environ);
		void		clear_cgi();
		std::string	get_random_file_name(int client_index);
};