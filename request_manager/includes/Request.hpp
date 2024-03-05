/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mprofett <mprofett@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/16 11:08:01 by nesdebie          #+#    #+#             */
/*   Updated: 2024/03/05 10:24:33 by mprofett         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# define SPACE 32

# include <string>
# include <cstring>
# include <sstream>
# include <vector>
# include <iostream>
# include <map>
# include <exception>
# include "../includes/RequestLine.hpp"

class RequestLine;

class Request{
    private:
        std::string _raw;
        RequestLine _req;
        std::map<std::string, std::string>  _headers;
        std::string _body;

    public:
        Request();
        Request(std::string & req);
        Request(Request const &copy);
        ~Request();

        Request & operator=(Request const &op);

        std::string getRaw() const;
        RequestLine getRequestLine() const;
        std::string getHeader(std::string const &name);
        std::map<std::string, std::string> getHeaders() const;
        std::string getBody() const;

        std::vector<std::string> vectorSplit(std::string str, char sep);
        void setData(std::string head, std::string val);
        std::string ft_strtrim(std::string & s);

		class	HeaderNotFoundException : public std::exception {
            public:
                const char *what() const throw();
		};
};

std::ostream & operator<<(std::ostream &o, Request const &obj);

#endif
