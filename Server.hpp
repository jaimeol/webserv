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
        std::vector<std::string> _locations;
        struct sockaddr_in server_address;
        int listen_fd;
        pollfd _pollfd;
        
        // Métodos privados auxiliares
        void validateCgiPathsAndExtensions(Location &location);
        void tryCgiLocation(Location &location);
        void tryStandardLocation(Location &location);
        std::string getCurrentWorkingDir();
        static int checkFile(std::string const path, int mode);
        bool fileExistAndReadable(const std::string &path, const std::string &index);
        
    public:
        Server();
        Server(Server const &copy);
        ~Server();

        void initWebErrors();
        static int getPathType(std::string const &path);
        void tryLocation(Location &location);
        bool validHost(const std::string& hostname) const;
        void validEOL(std::string const token);
        void setWebErrors();
        void setServerName(std::string name);
        void setHost(std::string hostname);
        void setRoot(std::string root);
        void setFd(int fd);
        void setPort(std::string port);
        void setClientMaxBodySize(std::string bodySize);
        void setErrorPages(std::vector<std::string> web_errors);

        std::string getWebError(int code) const;
        // Getters adicionales
        uint16_t getPort() const { return _port; }
        in_addr_t getHost() const { return _host; }
        const std::string& getName() const { return _name; }
        const std::string& getRoot() const { return _root; }
        unsigned long getClientMaxBodySize() const { return client_max_body_size; }
        bool getAutoindex() const { return _autoindex; }
        int getListenFd() const { return listen_fd; }
};