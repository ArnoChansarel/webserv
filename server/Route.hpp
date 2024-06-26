/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nesdebie <nesdebie@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/16 11:34:39 by mprofett          #+#    #+#             */
/*   Updated: 2024/04/08 12:05:00 by nesdebie         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTE_HPP
# define ROUTE_HPP

# include <string>
# include <list>
# include "unistd.h"
# include "Server.hpp"

class Server;

class Route
{
	public:
		Route();
		Route(Server *server);
		~Route();

		bool					getAutoindex(void) const;
		bool					getCgi(void) const;
		bool					getDelete(void) const;
		bool					getGet(void) const;
		bool					getPost(void) const;
		std::list<std::string>	getIndex(void) const;
		std::string				getExtension(void) const;
		std::string				getPath(void) const;
		std::string				getRedirection(void) const;
		std::string				getRoot(void) const;
		Server*					getServer(void) const;
		void					addIndex(std::string filename);
		void					setAutoindex(bool value);
		void					setCgi(bool value);
		void					setDelete(bool value);
		void					setGet(bool value);
		void					setPost(bool value);
		void					setExtension(std::string extension);
		void					setPath(std::string path);
		void					setRedirection(std::string url);
		void					setRoot(std::string path);

		void					printRoute(void) const;

	private:
		bool					_autoindex;
		bool					_cgi;
		bool					_delete;
		bool					_get;
		bool					_post;
		std::list<std::string>	_index;
		std::string				_extension;
		std::string				_path;
		std::string				_redirection;
		std::string				_root;
		Server					*_server;
};

#endif
