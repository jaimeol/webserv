#include "parseConfig/Server.hpp"
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

volatile sig_atomic_t g_running = 1;

void signalHandler(int) {
    g_running = 0;
	std::cout << std::endl;
}

int main(void) {
    try {
        signal(SIGINT, signalHandler);
        
        Server server;
        
        // Configurar el servidor
        server.setPort("8080;");
        server.setHost("127.0.0.1;");
        server.setRoot("./www;");
        server.setWebErrors();
        
        // Crear el servidor y verificar que se creó correctamente
        int server_fd = server.createServer();
        if (server_fd < 0) {
            std::cerr << "Error: No se pudo crear el servidor" << std::endl;
            return 1;
        }
        
        // Poner el socket en modo escucha
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Error: No se pudo establecer el socket en modo escucha" << std::endl;
            close(server_fd);
            return 1;
        }
        
        std::cout << "Servidor corriendo en http://localhost:" << server.getPort() << "/" << std::endl;
        std::cout << "Para probar errores, visita:" << std::endl;
        std::cout << "  - http://localhost:" << server.getPort() << "/403 (Forbidden)" << std::endl;
        std::cout << "  - http://localhost:" << server.getPort() << "/404 (Not Found)" << std::endl;
        std::cout << "  - http://localhost:" << server.getPort() << "/500 (Server Error)" << std::endl;
        
        // Vector para manejar múltiples conexiones con poll
        std::vector<pollfd> poll_fds;
        poll_fds.push_back(server.getPollFd());
        
        // Loop principal
        while (g_running) {
            int ready = poll(&poll_fds[0], poll_fds.size(), 1000);
            
            if (ready < 0) {
                if (errno == EINTR) // Interrupción por señal
                    continue;
                std::cerr << "Error en poll: " << strerror(errno) << std::endl;
                break;
            }
            
            // Procesar eventos
            for (size_t i = 0; i < poll_fds.size(); i++) {
                if (!(poll_fds[i].revents & POLLIN))
                    continue;
                
                if (poll_fds[i].fd == server_fd) {
                    // Aceptar nueva conexión
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int clientFd = accept(server_fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
                    
                    if (clientFd < 0) {
                        std::cerr << "Error aceptando conexión" << std::endl;
                        continue;
                    }
                    
                    // Configurar socket como no bloqueante
                    int flags = fcntl(clientFd, F_GETFL, 0);
                    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
                    
                    // Añadir a poll
                    pollfd client_pfd;
                    client_pfd.fd = clientFd;
                    client_pfd.events = POLLIN;
                    poll_fds.push_back(client_pfd);
                    
                    std::cout << "Nueva conexión aceptada desde " 
                              << inet_ntoa(clientAddr.sin_addr) << ":" 
                              << ntohs(clientAddr.sin_port) << std::endl;
                } else {
                    // Leer datos del cliente
                    char buffer[4096] = {0};
                    int bytesRead = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    
                    if (bytesRead <= 0) {
                        // Cliente desconectado o error
                        close(poll_fds[i].fd);
                        poll_fds.erase(poll_fds.begin() + i);
                        i--; // Ajustar índice
                        continue;
                    }
                    
                    // Añadir terminador nulo para tratarlo como string
                    buffer[bytesRead] = '\0';
                    std::string request(buffer);
                    
                    // Extraer la URI de la solicitud HTTP
                    std::string requestUri;
                    size_t uriStart = request.find("GET ") + 4;
                    if (uriStart != std::string::npos) {
                        size_t uriEnd = request.find(" HTTP/", uriStart);
                        if (uriEnd != std::string::npos) {
                            requestUri = request.substr(uriStart, uriEnd - uriStart);
                        }
                    }
                    
                    std::cout << "URI solicitada: " << requestUri << std::endl;
                    
                    // Determinar qué error mostrar según la URI
                    int errorCode = 200; // Por defecto, respuesta OK
                    std::string content;
                    
                    if (requestUri == "/404" || requestUri.find("/notfound") != std::string::npos) {
                        errorCode = 404;
                    } else if (requestUri == "/403" || requestUri.find("/forbidden") != std::string::npos) {
                        errorCode = 403;
                    } else if (requestUri == "/500" || requestUri.find("/error") != std::string::npos) {
                        errorCode = 500;
                    } else {
                        // Respuesta normal para otras rutas
                        content = "<html><body><h1>WebServ está funcionando!</h1>";
                        content += "<p>Para ver los errores, visita:</p>";
                        content += "<ul>";
                        content += "<li><a href='/403'>Error 403</a></li>";
                        content += "<li><a href='/404'>Error 404</a></li>";
                        content += "<li><a href='/500'>Error 500</a></li>";
                        content += "</ul></body></html>";
                    }
                    
                    // Generar respuesta según el código de error
                    std::string response;
                    if (errorCode != 200) {
                        try {
                            // Construir la ruta correcta al archivo de error
                            std::stringstream ss;
                            ss << errorCode;
                            std::string errorPage = server.getRoot() + "/weberrors/" + ss.str() + ".html";
                            std::cout << "Usando página de error: " << errorPage << std::endl;
                            
                            // Preparar encabezados según el código
                            switch (errorCode) {
                                case 404: response = "HTTP/1.1 404 Not Found\r\n"; break;
                                case 403: response = "HTTP/1.1 403 Forbidden\r\n"; break;
                                case 500: response = "HTTP/1.1 500 Internal Server Error\r\n"; break;
                            }
                            
                            response += "Content-Type: text/html; charset=UTF-8\r\n";
                            
                            // Leer el archivo de error
                            std::ifstream errorFile(errorPage.c_str());
                            if (!errorFile.is_open()) {
                                std::cerr << "No se pudo abrir la página de error: " << errorPage << std::endl;
                                throw std::runtime_error("No se pudo abrir la página de error");
                            }
                            std::stringstream buffer;
                            buffer << errorFile.rdbuf();
                            content = buffer.str();
                            errorFile.close();
                        } catch (const std::exception& e) {
                            std::cerr << "Error obteniendo página de error: " << e.what() << std::endl;
                            response = "HTTP/1.1 500 Internal Server Error\r\n";
                            response += "Content-Type: text/html; charset=UTF-8\r\n";
                            content = "<html><body><h1>Error 500</h1><p>Error interno del servidor</p></body></html>";
                        }
                    } else {
                        response = "HTTP/1.1 200 OK\r\n";
                        response += "Content-Type: text/html; charset=UTF-8\r\n";
                    }
                    
                    // Añadir contenido a la respuesta
                    std::stringstream ss;
                    ss << content.length();
                    response += "Content-Length: " + ss.str() + "\r\n\r\n";
                    response += content;
                    
                    // Enviar respuesta
                    send(poll_fds[i].fd, response.c_str(), response.length(), 0);
                    
                    // Cerrar la conexión
                    close(poll_fds[i].fd);
                    poll_fds.erase(poll_fds.begin() + i);
                    i--; // Ajustar índice
                }
            }
        }
        
        // Cerrar socket principal
        close(server_fd);
        std::cout << "Servidor cerrado correctamente" << std::endl;
        
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}