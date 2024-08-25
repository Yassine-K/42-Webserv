#include "../inc/webserv.hpp"


int main(int ac, char** av, char** env)
{
    if (ac != 2)
    {
        std::cerr << "Invalid number of arguments" << std::endl;
        return (0);
    }

    try
    {
        server::parse_config(av[1]);
        webserv::launch_server(env);
    }
    catch (int status)
    {
        std::cout << "********* status catched = " << status << std::endl;
        return (0);
    }
    catch (const char* msg)
    {
        std::cout << "--------- msg catched = " << msg << std::endl;
        return (0);
    }
}
