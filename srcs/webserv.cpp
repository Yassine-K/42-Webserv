#include "../inc/webserv.hpp"
#include <fcntl.h>
#include "../inc/config.hpp"
#include <unistd.h>
#include <cstring>

std::vector<server>	webserv::servers;

void	fd_sets::clear_sets()
{
	FD_ZERO(&this->read_fds);
	FD_ZERO(&this->read_fds_tmp);
	FD_ZERO(&this->write_fds);
	FD_ZERO(&this->write_fds_tmp);
}

void	webserv::serve_clients(fd_sets & set_fd, char** env)
{
	for (size_t index = 0; index < servers.size(); index++)
	{
		if (servers[index]._bound == false)
			continue;
		try
		{
			for (size_t j = 0; j < servers[index]._clients.size(); j++)
			{
				if (FD_ISSET(servers[index]._clients[j].get_fd(), &set_fd.read_fds_tmp))
				{
					servers[index]._clients[j].read_request(servers[index].get_config_index(), set_fd);
				}

				if (FD_ISSET(servers[index]._clients[j].get_fd(), &set_fd.write_fds_tmp))
				{

					if (servers[index]._clients[j]._cgi._cgi_processing == true)
					{
						// std::cout << "dkhel l cgi" << std::endl;
						servers[index]._clients[j]._cgi.run_cgi(servers[index]._clients[j], env);
						if (servers[index]._clients[j]._cgi._cgi_processing == false)
						{
							servers[index]._clients[j]._response._requested_file.open(servers[index]._clients[j]._cgi._outfile.c_str());
							servers[index]._clients[j]._response.send_cgi_headers(servers[index]._clients[j].get_fd(), servers[index]._clients[j]._response._requested_file);
							servers[index]._clients[j].set_ready_for_receiving_value(true);
						}
					}
					else if (servers[index]._clients[j].get_ready_for_receiving_value() == false)
					{
						std::cout << "handling request of client fd " << servers[index]._clients[j].get_fd() << std::endl;
						servers[index].handle_request(j, set_fd, servers[index]._clients[j]._location_index);
					}
					if (servers[index]._clients[j].get_ready_for_receiving_value() == true)
					{
						std::cout << "sending response to client with fd " << servers[index]._clients[j].get_fd() << std::endl;
						servers[index]._clients[j]._response.send_response(servers[index]._clients[j].get_fd(), server::_config[servers[index].get_config_index()]);
						if (servers[index]._clients[j]._response._bytes_sent >= servers[index]._clients[j]._response._content_length)
						{
							std::cout << "all chunks are sent to fd " << servers[index]._clients[j].get_fd() << std::endl;
							servers[index]._clients[j].clear_client();
							FD_CLR(servers[index]._clients[j].get_fd(), &set_fd.write_fds);
						}
					}
				}
			}
		}
		catch (const error & e)
		{
			servers[index]._clients[e._client_index]._response.return_error(e._status, servers[index]._clients[e._client_index].get_fd());
			servers[index]._clients[e._client_index].clear_client();
			FD_CLR(servers[index]._clients[e._client_index].get_fd(), &set_fd.write_fds);
		}
	}
}

void    webserv::launch_server(char** env)
{
	socklen_t											client_addr_len;
	struct sockaddr_in									client_addr;
	int													client_sock;
	// struct timeval										connection_time;
	fd_sets												set_fd;
	int													nfds;

	config::parse_mime_types("conf/mime.types");

	for (size_t i = 0; i < server::_config.size(); i++)
	{
		servers.push_back(server(i));
		if (fcntl(servers.back().get_fd(), F_SETFL, O_NONBLOCK) == -1)
		{
        	perror("fcntl F_SETFL");
			if (servers.back()._bound == true)
        		close(servers.back().get_fd());
        	servers.erase(servers.end() - 1);
    	}
	}

	set_fd.clear_sets();
	nfds = 0;

	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i]._bound == true)
		{
			std::cout << "index of bound server = " << i << std::endl;
			nfds = servers[i].get_fd();
			FD_SET(servers[i].get_fd(), &set_fd.read_fds);
		}
	}

	client_addr_len = sizeof(struct sockaddr_in);
	memset(&client_addr, 0, client_addr_len);

	std::cout << "server size() = " << servers.size() << std::endl;
	std::cout << "number of bound addresses = " << server::_bound_addresses.size() << std::endl;
	// exit(0);

	while (true)
	{
		// std::cout << "Waiting for connections...." << std::endl;

		set_fd.read_fds_tmp = set_fd.read_fds;
		set_fd.write_fds_tmp = set_fd.write_fds;

		if (select(nfds + 1, &set_fd.read_fds_tmp, &set_fd.write_fds_tmp, NULL, NULL) == -1)
		{
			perror("Error in select");
			for (size_t i = 0; i < servers.size(); i++)
			{
				if (servers[i]._bound == true)
					close(servers[i].get_fd());
			}
			throw 1;
		}

		for (size_t i = 0; i < servers.size(); i++)
		{
			if (servers[i]._bound == false)
				continue;

			if (FD_ISSET(servers[i].get_fd(), &set_fd.read_fds_tmp))
			{
				for (int iteration = 0; (client_sock = accept(servers[i].get_fd(), (struct sockaddr *)&client_addr, &client_addr_len)) != -1; iteration++)
				{
					// client_sock = accept(servers[i].get_fd(), (struct sockaddr *)&client_addr, &client_addr_len);
					if (client_sock == -1)
						perror("Error accepting connection");
					else
					{
						std::cout << "Connection accepted !!!\nClient number " << client_sock << std::endl;

						servers[i]._clients.push_back(client(client_sock, servers[i].get_config_index(), iteration));
						if (fcntl(servers[i]._clients.back().get_fd(), F_SETFL, O_NONBLOCK) == -1)
						{
        					perror("fcntl F_SETFL");
        					close(servers[i]._clients.back().get_fd());
        					servers[i]._clients.remove_from_end(1);
    					}
						else
						{
							if (client_sock > nfds)
							{
								nfds = client_sock;
							}
							FD_SET(servers[i]._clients.back().get_fd(), &set_fd.read_fds);
						}
					}
				}
			}
		}
		webserv::serve_clients(set_fd, env);
	}
}
