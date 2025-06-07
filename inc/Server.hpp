#pragma once

#include "WebServ.hpp"
#include "Location.hpp"

// Tipos de archivos para getPathType
#define FILE_TYPE 1
#define DIR_TYPE 2
#define OTHER_TYPE 3
#define NOT_FOUND -1

// Permisos para acceso a archivos
#define READ_PERMISSION 4
#define WRITE_PERMISSION 2
#define EXECUTE_PERMISSION 1

class Location;

class Server
{
    private:
        uint16_t    _port;
        in_addr_t   _host;
        std::string _name;
        std::string _index;
        std::string _root;
        unsigned long client_max_body_size;
        bool _autoindex;
        std::map<unsigned int, std::string> _web_errors;
        std::vector<Location> _locations;
        struct sockaddr_in server_address;
        int listen_fd;
        pollfd _pollfd;
        
        // Métodos privados auxiliares
        void validateCgiPathsAndExtensions(Location &location);
        void tryCgiLocation(Location &location);
        void tryStandardLocation(Location &location);
        std::string getCurrentWorkingDir();
        //static int checkFile(std::string const path, int mode);
        bool fileExistAndReadable(const std::string &path, const std::string &index);
        bool emptyWeberrors();
        
    public:
        Server();
        Server(Server const &copy);
        ~Server();
        void startServer();
        int createServer();

        void initWebErrors();
        static int getPathType(std::string const &path);
        void tryLocation(Location &location);
        bool validHost(const std::string& hostname) const;
        void validEOL(std::string const token);
		void addLocation(const Location& location);
        void setWebErrors();
        void setServerName(std::string name);
        void setHost(std::string hostname);
        void setRoot(std::string root);
        void setFd(int fd);
        void setPort(std::string port);
        void setClientMaxBodySize(std::string bodySize);
        void setErrorPages(std::vector<std::string> web_errors);
		void setIndex(std::string &index);
		void setAutoIndex(std::string &autoI);
		bool paramsLeft(std::string str, size_t pos);
		std::string returnParams(std::string str, size_t pos);
		std::vector<std::string> splitSpaces(std::string &methods);
		void parseExtensions(std::vector<std::string> &extensions, std::string &auxExt);
		void parseCgiPath(std::vector<std::string> &paths, std::string &auxPath);
		void setLocation(std::string name, std::vector<std::string> &src);
		

       const std::string &getName();
       const uint16_t &getPort();
       const in_addr_t &getHost();
       const std::string &getIndex();
       const std::string &getRoot();
       const unsigned long &getClientMaxBodySize();
       const bool &getAutoIndex();
       const std::map<unsigned int, std::string> &getWebErrors();
       const std::string &getWebErrorPath(int code);
	   const std::vector<Location> &getLocations() const;
       int getListenFd() const;
       const pollfd getPollFd();
       const std::vector<Location>::iterator &getLocationKey(std::string key);
       
};