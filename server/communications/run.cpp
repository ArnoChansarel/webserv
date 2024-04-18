/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   run.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mprofett <mprofett@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: Invalid date        by                   #+#    #+#             */
/*   Updated: 2024/04/18 09:44:29 by mprofett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "../TcpListener.hpp"
# include "../../request_manager/includes/Cgi.hpp"

void	TcpListener::runTcpListener()
{
	bool	running = true;
	fd_set	ready_to_read_fds;
	fd_set	ready_to_write_fds;
	Server	*server;

	while (running)
	{
		ready_to_read_fds = this->_read_master_fd;
		ready_to_write_fds = this->_write_master_fd;
		if (select(FD_SETSIZE, &ready_to_read_fds, &ready_to_write_fds, NULL, NULL) < 0)
			throw socketSelectFailure();
		for (int i = 0; i < FD_SETSIZE; i++)
		{
			if (FD_ISSET(i, &ready_to_read_fds))
			{
				server = this->getServerBySocket(i);
				if (server)
					handleNewConnection(server);
				else
				{
					readRequest(i);
					if (FD_ISSET(i, &_write_master_fd))
						handleRequest(i);
				}
			}
			else if (FD_ISSET(i, &ready_to_write_fds))
				writeResponse(i);
		}
	}
}

void	TcpListener::handleNewConnection(Server *server)
{
	std::cout << "Handling new connection" << std::endl;
	int			client_socket;
	sockaddr	client_addr;
	socklen_t	addr_size = sizeof(sockaddr);

	client_socket = accept(server->getSocket(), &client_addr, &addr_size);
	if (client_socket < 0)
		throw socketAcceptingNewConnectionFailure();
	FD_SET(client_socket, &this->_read_master_fd);
}

void	TcpListener::handleRequest(int client_socket)
{
	if (_pending_request.getMethod() == POST)
	{
		std::cout << "Method is POST" << std::endl;
		std::cout << "Body is: " << _pending_request.getBody().size() << "Content lenght is: " << _pending_request.getContentLength() <<std::endl;
	}
	int status_code = 200;
	Route *route = NULL;
	Server *server = getServerByHost(getPortBySocket(&client_socket), _pending_request.getHeader("Host"));

	std::string checkRoute = _pending_request.getPath();
	if (!checkRoute.empty() && checkRoute[checkRoute.size() - 1] != '/')
		checkRoute += "/";

	std::list<Route *> r = server->getRoute();
	for (std::list<Route *>::iterator it = r.begin(); it != r.end(); it++) {
		if (!(*it)->getPath().compare(checkRoute) || !((*it)->getPath() + "/").compare(checkRoute)) { // la deuxieme condition necessaire car ne rentre pas dans cgi sinon
			route = *it;
			break;
		} //else if (_pending_request.getPath() == (*it)->getCgiPath())
	}

	if (!route) {
		std::cout << "No ROUTE found.\n";
	} else {
		std::cout << "ROUTE found.\n";
	}

	std::cout << "[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]\n";
	if (route) {
		std::cout << "[CGI] YES ROUTE ! method is : " << _pending_request.getMethod() << std::endl;
		if ((route && !route->getExtension().empty()) || _pending_request.getMethod() == POST || (_pending_request.getMethod() == GET && _pending_request.getPath().compare("/"))) {
				std::cout << "[CGI] Start\n";
				try {
					Cgi cgi(_pending_request, *route);
					cgi.executeCgi();
					status_code = cgi.getExitCode();
				}
				catch(std::exception &e) {
					std::cout << e.what() << std::endl;
				}
				std::cout << "[CGI] End\n [SATUS CODE =" << status_code << "]\n";
		}
	}

	Response* response = new Response(server, status_code, &_pending_request, client_socket);//                create response here
	response->buildResponse(route);
	FD_SET(client_socket, &this->_write_master_fd);
	this->registerResponse(client_socket, response);

	/*
		Server		*server = getServerByHosT();
		Route		*route = findRoute();
		char		*responseBody;
		int			status_code;
		Response	response(server, this->_pending_request, socket);

		if (Route)
		{
			std::string	filename = getFilenameToUseForResponse();
			if (filename == isCGI())
				getCGIResponse(&status_code, &responseBody);
			else
				getResponse(&status_code, &responseBody);
			response.buildResponse(&status_code, &responseBody);
		}
		else
			response.buildResponse(&status_code, &responseBody);
		this->registerResponse(client_socket, response);
	*/
}

void	TcpListener::registerResponse(int socket, Response *response)
{
	std::map<int, Response*>::iterator	it;

	it = this->_responses.find(socket);
	if (it == this->_responses.end())
		this->_responses.insert(std::pair<int, Response*>(socket, response));
	else
		it->second = response;
}

// void	TcpListener::writeResponse(int client_socket)
// {
// 	std::cout << "Sending response\n";
// 	std::string	response = this->_responses.find(client_socket)->second->getResponse();

// 	send(client_socket, response.c_str(), response.size(), 0);
// 	this->_responses.erase(client_socket);
// 	FD_CLR(client_socket, &this->_write_master_fd);
// 	// DELETE RESPONSES HERE
// 	this->_responses.erase(client_socket);
// }

void	TcpListener::writeResponse(int client_socket)
{
	std::cout << "Sending response" << std::endl;
	std::string	response = this->_responses.find(client_socket)->second->getResponse();

	std::cout << "Response length, about to send is : " << response.length() << std::endl;

	size_t totalBytesSent = 0;
	std::vector<std::string> chunks;

	size_t pos = 0;
	size_t chunkSize = 8192;
	while (pos < response.size()) {
        size_t len = std::min(chunkSize, response.size() - pos);
        chunks.push_back(response.substr(pos, len));
        pos += len;
    }

	int i = 1;
	for (std::vector<std::string>::iterator it = chunks.begin(); it != chunks.end(); ++it) {
		std::cout << "We had " << i << " loops." << std::endl;
		i++;

        const std::string& chunk = *it;
        ssize_t bytesSent = send(client_socket, chunk.c_str(), chunk.size(), 0);
		if (bytesSent == -1) {
			std::cerr << "Error on sending response." << std::endl;
			perror("send");
			break;
		} else if (bytesSent == 0) {
			std::cerr << "Nothing has been sent :0" << std::endl;
			break;
		} else {
			std::cout << bytesSent << " bytes sent." << std::endl;
			totalBytesSent += bytesSent;
		}
	}

	FD_CLR(client_socket, &this->_write_master_fd);
	// DELETE RESPONSES HERE
	delete this->_responses.find(client_socket)->second;
	this->_responses.erase(client_socket);
	std::cout << "Response Sent" << std::endl;
}
