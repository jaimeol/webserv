#include "inc/Server.hpp"
#include "inc/Config.hpp"
#include "./inc/HttpRequest.hpp"
#include "./inc/HttpResponse.hpp"
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include "./inc/HttpHandler.hpp"

volatile sig_atomic_t g_running = 1;

void signalHandler(int) {
    g_running = 0;
    std::cout << std::endl;
}

int main(int argc, char **argv) {
    try {
        signal(SIGINT, signalHandler);
        
        // Configuración del servidor
        std::string configFile = (argc > 1) ? argv[1] : "simple_config.conf";
        Config config(configFile);
        
        // Vector para almacenar los file descriptors de los servidores
        std::vector<pollfd> poll_fds;
        // Mapa para relacionar file descriptors con servidores
        std::map<int, Server*> server_map;
        
        // Inicializar todos los servidores
        for (size_t i = 0; i < config.getServerNum(); ++i) {
            Server* server = config.getServer(i);
            if (!server) {
                std::cerr << "No se pudo cargar la configuración del servidor " << i << std::endl;
                continue;
            }
            
            // Crear el servidor
            int server_fd = server->createServer();
            if (server_fd < 0) {
                std::cerr << "Error: No se pudo crear el servidor " << i << std::endl;
                continue;
            }
            
            // Configurar el socket en modo escucha
            if (listen(server_fd, 10) < 0) {
                std::cerr << "Error: No se pudo establecer el socket en modo escucha para servidor " << i << std::endl;
                close(server_fd);
                continue;
            }
            
            // Añadir el servidor al poll y al mapa
            pollfd pfd = server->getPollFd();
            poll_fds.push_back(pfd);
            server_map[server_fd] = server;
            
            std::cout << "Servidor " << i << " corriendo en http://" << server->getName() 
                     << ":" << server->getPort() << "/" << std::endl;
        }
        
        if (poll_fds.empty()) {
            throw std::runtime_error("No se pudo iniciar ningún servidor");
        }
        
        // Loop principal
        while (g_running) {
            int ready = poll(&poll_fds[0], poll_fds.size(), 1000);
            
            if (ready < 0) {
                if (errno == EINTR)
                    continue;
                std::cerr << "Error en poll: " << strerror(errno) << std::endl;
                break;
            }
            
            // Procesar eventos
            for (size_t i = 0; i < poll_fds.size(); i++) {
                if (!(poll_fds[i].revents & POLLIN))
                    continue;

                int current_fd = poll_fds[i].fd;
                
                // Comprobar si es un servidor esperando conexión
                if (server_map.find(current_fd) != server_map.end()) {
                    // Nueva conexión
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(current_fd, (struct sockaddr*)&client_addr, &client_len);
                    
                    if (client_fd < 0) {
                        std::cerr << "Error al aceptar nueva conexión" << std::endl;
                        continue;
                    }
                    
                    pollfd new_pollfd;
                    new_pollfd.fd = client_fd;
                    new_pollfd.events = POLLIN;
                    poll_fds.push_back(new_pollfd);
                } else {
                    // Manejar conexión de cliente
                    char buffer[4096] = {0};
                    int bytesRead = recv(current_fd, buffer, sizeof(buffer) - 1, 0);

                    if (bytesRead <= 0) {
                        close(current_fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        i--;
                        continue;
                    }

                    buffer[bytesRead] = '\0';
                    std::string request(buffer);

                    HttpRequest req;
                    HttpResponse res;
                    try {
                        req.parse(request);
                        
                        // Encontrar el servidor correcto basado en el host
                        Server* targetServer = NULL;
                        std::string host = req.headers["Host"];
                        for (std::map<int, Server*>::iterator it = server_map.begin(); 
                             it != server_map.end(); ++it) {
                            if (it->second->getName() == host) {
                                targetServer = it->second;
                                break;
                            }
                        }
                        
                        if (!targetServer) {
                            targetServer = server_map.begin()->second; // Servidor por defecto
                        }
                        
                        res = HttpHandler::handleRequest(req, *targetServer);
                    } catch (const std::exception& e) {
                        res.status_code = 400;
                        res.status_text = "Bad Request";
                        res.body = "<html><body><h1>400 Bad Request</h1></body></html>";
                    }

                    std::ostringstream ss;
                    ss << res.body.length();
                    res.headers["Content-Type"] = "text/html; charset=UTF-8";
                    res.headers["Content-Length"] = ss.str();

                    std::string responseText = res.toString();
                    send(current_fd, responseText.c_str(), responseText.length(), 0);

                    close(current_fd);
                    poll_fds.erase(poll_fds.begin() + i);
                    i--;
                }
            }
        }
        
        // Limpieza
        for (std::map<int, Server*>::iterator it = server_map.begin(); 
             it != server_map.end(); ++it) {
            close(it->first);
        }
        
        std::cout << "Servidores cerrados correctamente" << std::endl;
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}