/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: achansar <achansar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/19 16:58:55 by achansar          #+#    #+#             */
/*   Updated: 2024/03/26 17:47:38 by achansar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <filesystem>

// ============================================================================== CONSTRUCTORS

Response::Response(Server* server, int statusCode, Request* request, const int socket) :
        _clientSocket(socket),
        _method(request->getMethod()),
        _statusCode(statusCode),
        _path("/"),
        _server(server),
        _request(request) {

    return;
}

Response::~Response() {
    return;
}

// ============================================================================== MEMBER FUNCTIONS

// ==================================================================== FILE TRANSFER

int Response::sendFile() {

	std::cout << "\nIN SENDFILE :\n" << std::endl;

	std::ifstream	infile(_path, std::ios::binary | std::ios::in);
	if (!infile) {
		std::cout << "Le fichier ne s'ouvre pas." << std::endl;
		return 500;
	} else {
		infile.seekg(0, std::ios::end);
		std::streampos fileSize = infile.tellg();
		infile.seekg(0, std::ios::beg);

        std::string fileName = extractFileName();
        std::cout << "right before, filename is : " << fileName << std::endl;

		std::stringstream responseHeaders;
			responseHeaders << "HTTP/1.1 200 OK\r\n";
			responseHeaders << "Content-Type: " << getMimeType() << "\r\n";
			responseHeaders << "Content-Disposition: attachment; filename=\"" << fileName << "\"\r\n";
            responseHeaders << "Content-Length: " << fileSize << "\r\n\r\n";
			
		write(_clientSocket, responseHeaders.str().c_str(), responseHeaders.str().length());
		const std::streamsize bufferSize = 8192;
		char buffer[bufferSize];

        while (!infile.eof()) 
            {
                infile.read(buffer, sizeof(buffer));
                ssize_t result = write(_clientSocket, buffer, infile.gcount());
                if (result == -1) 
                {
                    std::cerr << "Error writing to socket." << std::endl;
                    break;
                }
            }

        infile.close();
	}
	return 200;
}

/*

TO DO (Arno)
	build receive() using read() from client socket, by chunks
	work on 300 reidrections
	do autoindexes
*/

// void send100Continue(int socket) {
//     const char* response = "HTTP/1.1 100 Continue\r\n\r\n";
//     int bytesSent = send(socket, response, strlen(response), 0);
//     if (bytesSent == -1) {
//         std::cerr << "Error sending 100 Continue response: " << strerror(errno) << std::endl;
//     } else {
//         std::cout << "Sent 100 Continue response" << std::endl;
//     }
// }

    // std::cout << "\n\nLet's print POST request elements !:\n"
    //             << "Raw :\n" << _request->getRaw() << std::endl
    //             << "Uri : " << _request->getPath() << std::endl
    //             << "Body :\n" << _request->getBody() << std::endl
    //             << "Boundary : " << _request->getBoundaryString() << std::endl;

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sstream>
// #include <iostream>
// #include <vector>
    
int Response::receiveFile() {

    std::stringstream rawRequest(_request->getRaw());
    std::string line;
    std::string fileName;
    std::string contentType;
    std::string boundaryString;
    size_t  lengthRequest = 0;

    int kq = kqueue();
    if (kq == -1) {
        // Error handling
        return -1;
    }

    while (std::getline(rawRequest, line) && !line.empty()) {
        size_t Pos = 0;
        if (line.find("------WebKitFormBoundary") != std::string::npos) {
            boundaryString = line;
        }
        else if (line.find("Content-Length") != std::string::npos) {
            Pos = line.find_last_of(" ");
            if (Pos != std::string::npos) {
                lengthRequest = stoi(line.substr(Pos + 1, std::string::npos));
            }
        }
        else if (line.find("Content-Disposition") != std::string::npos) {
            Pos = line.find_last_of("=");
            if (Pos != std::string::npos) {
                fileName = line.substr(Pos + 2, std::string::npos - 1);
            }
        }
        else if (line.find("Content-Type:") != std::string::npos) {
            Pos = line.find_last_of(" ");
            if (Pos != std::string::npos) {
                contentType = line.substr(Pos + 1, std::string::npos);
                break;
            }
        }
    }

    struct kevent ev;
    EV_SET(&ev, _clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    kevent(kq, &ev, 1, NULL, 0, NULL);

    std::vector<char> fileRequestBody(lengthRequest, 0);
    size_t totalRead = 0;
    int i = 0;
    while (totalRead < lengthRequest) {
        struct kevent events[1];
        std::cout << "BEFORE\n";
        int nevents = kevent(kq, NULL, 0, events, 1, NULL);
        std::cout << "AFTER\n";
        if (nevents == -1) {
            close(kq);
            return -1;
        }
        int bytesRead = read(_clientSocket, fileRequestBody.data() + totalRead, lengthRequest - totalRead);
        if (bytesRead > 0) {
            std::cerr << "Au moins 1...\n";
        }
        if (bytesRead == 0) {
            std::cerr << "READ = 0\n";
        } else if (bytesRead == -1) {
            std::cerr << "READ = -1\n";
            std::cerr << "Error reading from file descriptor: " << strerror(errno) << std::endl;
        }
        i++;
        if (i > 10) {break;}
    }

	return 200;
}

    // std::cout << "\n\nHere's what we got from our parsing :\n"
    //             << "Boundary stirng : " << boundaryString << std::endl
    //             << "Length : " << lengthRequest << std::endl
    //             << "Content type : " << contentType << std::endl
    //             << "File name : " << fileName << std::endl
    //             << "Our Vector : \n";
    // for (size_t i = 0; i < fileRequestBody.size(); i++) {
    //     std::cout << fileRequestBody[i];
    // } std::cout << std::endl;
    /*
    use a std::getline() on headers first, to get content-type and content-length
    use the boundary string to get a size_t boundary_pos and place at begining of body
    use vector of char to store the body.
    in a while loop, read each chunk a leur tour
    sans oublier de checker si le retour est de 0 ET de 1
    */

// #include <sys/types.h>
// #include <sys/event.h>
// #include <sys/time.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sstream>
// #include <iostream>
// #include <vector>

// #define BUFSIZE 4096

// int Response::receiveFile() {

//     std::cout << "\n\nIn receive\n";
    
//     int kq = kqueue();
//     if (kq == -1) {
//         // Error handling
//         return -1;
//     }

//     std::stringstream rawRequest(_request->getRaw());
//     std::string line;
//     std::string boundaryString;
//     size_t lengthRequest = 0;

//     while (std::getline(rawRequest, line) && !line.empty()) {
//         // Extract boundary string
//         int i = 0;
//         i++;
//         if (line.find("------WebKitFormBoundary") != std::string::npos) {
//             boundaryString = line;
//         }
//         // Extract content length
//         else if (line.find("Content-Length") != std::string::npos) {
//             size_t Pos = line.find_last_of(" ");
//             if (Pos != std::string::npos) {
//                 lengthRequest = stoi(line.substr(Pos + 1));
//             }
//         }
//         std::cout << "LINE : " << line << std::endl;
//         if (i > 5) {return -1;}
//     }

//     // Register socket descriptor for read events
//     struct kevent ev;
//     EV_SET(&ev, _clientSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
//     kevent(kq, &ev, 1, NULL, 0, NULL);

//     // Read file data using kqueue
//     std::vector<char> fileRequestBody(lengthRequest, 0);
//     size_t totalRead = 0;
//     while (totalRead < lengthRequest) {
//         struct kevent events[1];
//         int nevents = kevent(kq, NULL, 0, events, 1, NULL);
//         if (nevents == -1) {
//             // Error handling
//             close(kq);
//             return -1;
//         }
//         int bytesRead = read(_clientSocket, fileRequestBody.data() + totalRead, lengthRequest - totalRead);
//         if (bytesRead == -1) {
//             // Error handling
//             close(kq);
//             std::cout << "C'est le 1\n";
//             return -1;
//         } else if (bytesRead == 0) {
//             std::cout << "C'est le 0\n";
//             return 0;
//         }
//         totalRead += bytesRead;
        
//     }

//     close(kq);

//     // Process the file data
//     // ...

//     std::cout << "\n\nHere's what we got from our parsing :\n"
//                 << "Boundary stirng : " << boundaryString << std::endl
//                 << "Length : " << lengthRequest << std::endl
//                 // << "Content type : " << contentType << std::endl
//                 // << "File name : " << fileName << std::endl
//                 << "Our Vector : \n";
//     for (size_t i = 0; i < fileRequestBody.size(); i++) {
//         std::cout << fileRequestBody[i];
//     } std::cout << std::endl;

//     return 200;
// }


int Response::fileTransfer() {

    switch (_method) {
        case GET:       return sendFile();
        case POST:      return receiveFile();
        case DELETE:    return 200;
        default:        return 200;
    }
}

// ==================================================================== POST METHOD


// ==================================================================== GET METHOD

std::string Response::getReason(int sc) {

    std::map<int, std::string> reasons;
    reasons.insert(std::make_pair(200, "OK"));
    reasons.insert(std::make_pair(201, "Created"));
    reasons.insert(std::make_pair(400, "Bad Request"));
    reasons.insert(std::make_pair(403, "Forbidden"));
    reasons.insert(std::make_pair(404, "Not Found"));
    reasons.insert(std::make_pair(500, "Internal Server Error"));

    std::map<int, std::string>::iterator it = reasons.find(sc);
    if (it == reasons.end()) {
        std::cout << "Status code not implemented !" << std::endl;
    }
    return it->second;
}

std::string Response::getHeaders(const int s) {//          which header is important ?
    std::string h;

    if (_method == GET) {
        h += "Content-Type: " + getMimeType() + "\r\n";
    }
    h += "Content-Length: " + std::to_string(s) + "\r\n";// virer tostirng
    return h;
}

std::string Response::getBody() {

	std::ifstream			myfile;
    std::string             line;
    std::string             body;

    if (_method == GET) {
        myfile.open(_path);
        if (myfile.fail()) {
            _statusCode = 500;
        }

        while (std::getline(myfile, line)) {
            body += line;
        }

        myfile.close();
    } else if (_method == POST) {
        body = "";
        // might be infos :
        // resource creation & confirmation message
    } else if (_method == DELETE) {
        body = "";
    }
    return body;
}

// ==================================================================== SWITCH


void      Response::buildResponse(Route *route) {

    _extension = extractExtension(_request->getPath());
    getFullPath(route, _request->getPath());
    
    std::stringstream   ss;

    std::cout << "\nREQUEST IN BUILD:\n" << _request->getRaw() << std::endl;
    // std::cout << "Before sending file, extension is : " << _extension << std::endl;
    if ((!_extension.empty() && _extension.compare(".html")) || _method != GET) {
            _statusCode = fileTransfer();
    }
    if (_statusCode == 200) {  
        _body = getBody();
        _headers = getHeaders(_body.length()) + "\n";
        ss << _statusCode;
        _statusLine = "HTTP/1.0 " + ss.str() + " " + getReason(_statusCode) + "\n";
    } else {
        buildErrorResponse();
    }
    _responseLine = _statusLine + _headers + _body;
    std::cout << "\nRESPONSE :: \n" << _responseLine << std::endl;
    return;
}

// ============================================================================== GETTER & SETTER

std::string     Response::getResponse() {
    return _responseLine;
}

std::string     Response::getPath() {
    return _path;
}

int             Response::getStatusCode() {
    return _statusCode;
}

void            Response::setPath(std::string& str) {
    _path = str;
    return;
}

void            Response::setErrorPath(std::string& str) {
    _errorPath = str;
    return;
}

// ============================================================================== UTILS

std::string Response::extractExtension(std::string uri) {
    
    std::cout << "In extract extension, path : " << uri << std::endl;
    size_t extPos = uri.find_last_of(".");
    if (extPos != std::string::npos) {
        std::string extension = uri.substr(extPos, std::string::npos);
        return extension;
        std::cout << "EXTENSION IS : " << extension << std::endl;    
    } else {
        std::cerr << "Couldn't extract extension.\n";
        return "";
    }
}

std::string Response::extractFileName() {
    
    size_t extPos = _path.find_last_of("/");
    if (extPos != std::string::npos) {
        std::string fileName = _path.substr(extPos + 1, std::string::npos);
        return fileName;    
    } else {
        std::cerr << "Couldn't extract file name.\n";
        return NULL;
    }
}

std::string     Response::getMimeType() {

    if (!_extension.empty()) {
        return _server->getMimeType(_extension);
    }
    return _server->getMimeType("default");
}



void	Response::getFullPath(Route *route, std::string uri) {

	if (route) {
		std::string root = route->getRoot() + "/";
		std::string index = route->getIndex().front();//         only gets first index. it's temporary
		_path = root + index;
	} else {
		std::ifstream			myfile;

        if (_extension.compare(".html")) {
            _path = "./download" + uri;
        } else {
            _path = "./docs" + uri;
        }
        // std::cout << "Because URI : " << uri << ", _extension : " << _extension << "and _path became : " << _path << std::endl;
		myfile.open(_path);
		if (myfile.fail()) {
			_statusCode = 404;
			_path = "/";
		} else {
			myfile.close();
		}
	}
	return;
}
