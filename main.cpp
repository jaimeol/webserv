#include "inc/Server.hpp"
#include "inc/Config.hpp"
#include "./inc/HttpRequest.hpp"
#include "./inc/HttpResponse.hpp"
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
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
        
        // Usar archivo de configuración
        std::string configFile = (argc > 1) ? argv[1] : "simple_config.conf";
        Config config(configFile);
        // Obtener el primer servidor configurado
        Server* serverPtr = config.getServer(0);
        if (!serverPtr) {
            std::cerr << "No se pudo cargar la configuración del servidor" << std::endl;
            return 1;
        }
        Server& ref_server = *serverPtr;
        
        // Crear el servidor y verificar que se creó correctamente
        int server_fd = ref_server.createServer();
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
        
        std::cout << "Servidor corriendo en http://" << ref_server.getHost() << ":" << ref_server.getPort() << "/" << std::endl;
        std::cout << "Para probar errores, visita:" << std::endl;
        std::cout << "  - http://" << ref_server.getHost() << ":" << ref_server.getPort() << "/403 (Forbidden)" << std::endl;
        std::cout << "  - http://" << ref_server.getHost() << ":" << ref_server.getPort() << "/404 (Not Found)" << std::endl;
        std::cout << "  - http://" << ref_server.getHost() << ":" << ref_server.getPort() << "/500 (Server Error)" << std::endl;
        
        // Vector para manejar múltiples conexiones con poll
        std::vector<pollfd> poll_fds;
        poll_fds.push_back(ref_server.getPollFd());
        
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
					struct sockaddr_in client_addr;
					socklen_t client_len = sizeof(client_addr);
					int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
					if (client_fd < 0) {
						std::cerr << "Error al aceptar nueva conexión" << std::endl;
						continue;
					}
					pollfd new_pollfd;
					new_pollfd.fd = client_fd;
					new_pollfd.events = POLLIN;
					poll_fds.push_back(new_pollfd);
				} else {
					// Leer datos del cliente
					char buffer[4096] = {0};
					int bytesRead = recv(poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

					if (bytesRead <= 0) {
						close(poll_fds[i].fd);
						poll_fds.erase(poll_fds.begin() + i);
						i--;
						continue;
					}

					buffer[bytesRead] = '\0';
					std::string request(buffer);
					std::cout << "Petición recibida:\n" << request << std::endl; // <-- Añade esto


					HttpRequest req;
					HttpResponse res;
					try {
						req.parse(request);
						std::cout << "Método: " << req.method << ", URI: " << req.uri << std::endl;

						// Determinar si es una ruta de error
						if (req.uri == "/404" || req.uri.find("/notfound") != std::string::npos) {
							res.status_code = 404;
							res.status_text = "Not Found";
							std::ifstream errorFile(ref_server.getWebErrorPath(404).c_str());
							if (errorFile.is_open()) {
								std::stringstream buffer;
								buffer << errorFile.rdbuf();
								res.body = buffer.str();
							} else {
								res.body = "<html><body><h1>404 Not Found</h1></body></html>";
							}
						} else if (req.uri == "/403" || req.uri.find("/forbidden") != std::string::npos) {
							res.status_code = 403;
							res.status_text = "Forbidden";
							std::ifstream errorFile(ref_server.getWebErrorPath(403).c_str());
							if (errorFile.is_open()) {
								std::stringstream buffer;
								buffer << errorFile.rdbuf();
								res.body = buffer.str();
							} else {
								res.body = "<html><body><h1>403 Forbidden</h1></body></html>";
							}
						} else if (req.uri == "/500" || req.uri.find("/error") != std::string::npos) {
							res.status_code = 500;
							res.status_text = "Internal Server Error";
							std::ifstream errorFile(ref_server.getWebErrorPath(500).c_str());
							if (errorFile.is_open()) {
								std::stringstream buffer;
								buffer << errorFile.rdbuf();
								res.body = buffer.str();
							} else {
								res.body = "<html><body><h1>500 Internal Server Error</h1></body></html>";
							}
						} else {
							// Respuesta normal
							res = HttpHandler::handleRequest(req, ref_server);
						}
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
					send(poll_fds[i].fd, responseText.c_str(), responseText.length(), 0);

					close(poll_fds[i].fd);
					poll_fds.erase(poll_fds.begin() + i);
					i--;
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