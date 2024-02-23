/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpListener.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mprofett <mprofett@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/14 09:32:35 by mprofett          #+#    #+#             */
/*   Updated: 2024/02/23 14:45:27 by mprofett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TCPLISTENER_HPP
# define TCPLISTENER_HPP

# include <fstream>
# include <iostream>
# include <list>
# include <map>
# include <string>
# include <sstream>
# include <fcntl.h>
# include <exception>
# include <errno.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <arpa/inet.h>
# include <unistd.h>
# include "Server.hpp"
# include "Route.hpp"
# include "../utils.hpp"

# define MAXBUFFERSIZE 2097152

class TcpListener
{
	public:

		class	socketInitializationFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	socketConfigurationFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	socketBindingFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	socketListeningFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	socketSelectFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	socketAcceptingNewConnectionFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	socketPortIdentificationFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	openFileFailure : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	confFileMisconfiguration : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		class	confFileMissingDirective : public std::exception
		{
			public:
				virtual const char *what() const throw();
		};

		TcpListener(std::string	configfile);
		TcpListener(const char * ipAddress, int port, int buffer_max);
		~TcpListener();

		void	init();
		void	run();
		void	parseConfigurationFile(std::string filename);

	private:
		const char *_ipAddress; //temporary var
		int			_port; //temporary var

		long long int				_buffer_max;
		int							_socket;
		fd_set						_read_master_fd;
		fd_set						_write_master_fd;
		std::map<int, std::string>	_responses;
		std::list<Server *>			_servers;

		//parse config file utils
		void					isDigit(std::string) const;
		std::list<std::string>	popFrontToken(std::list<std::string> token_list);

		std::list<std::string>	getNextLocationDirective(std::list<std::string> token_list, Route *new_route);
		std::list<std::string>	getAutoIndexDirective(std::list<std::string> token_list, Route *new_route);
		std::list<std::string>	getAllowMethodsDirective(std::list<std::string> token_list, Route *new_route);
		std::list<std::string>	getCgiPathDirective(std::list<std::string> token_list, Route *new_route);
		std::list<std::string>	getIndexDirective(std::list<std::string> token_list, Route *new_route);
		std::list<std::string>	getProxyPassDirective(std::list<std::string> token_list, Route *new_route);
		std::list<std::string>	getRootDirective(std::list<std::string> token_list, Route *new_route);

		std::list<std::string>	getNextServerDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getListenDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getHostDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getServerNameDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getErrorPageDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getRootDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getIndexDirective(std::list<std::string> token_list, Server *new_server);
		std::list<std::string>	getLocationDirective(std::list<std::string> token_list, Server *new_server);

		std::list<std::string>	getMaxBodySizeDirective(std::list<std::string> token_list);
		std::list<std::string>	getNextServerConfig(std::list<std::string> token_list);
		std::list<std::string>	getServerDirectives(std::list<std::string>	token_list);
		std::list<std::string>	tokenizeConfigurationFile(std::string filename);
		// void					parseConfigurationFile(std::string filename);
		void					printServers() const;

		//init servers utils
		void			bindSocket(int port);

		//listening request utils
		void			handleNewConnection();
		void			readRequest(int socket);
		void			writeResponse(int socket);
		int				getPortFromSocket(int *socket);
};

#endif

