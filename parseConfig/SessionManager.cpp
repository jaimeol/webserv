
#include "../inc/SessionManager.hpp"

SessionManager::SessionManager() {
    std::srand(std::time(0));
}

std::string SessionManager::generateSessionId() {
    std::stringstream ss;
    ss << std::time(0) << "_" << std::rand();
    return ss.str();
}

bool SessionManager::hasSession(const std::string& sessionId) const {
    return sessions.find(sessionId) != sessions.end();
}

void SessionManager::createSession(const std::string& sessionId) {
    sessions[sessionId] = std::map<std::string, std::string>();
}

std::map<std::string, std::string>& SessionManager::getSession(const std::string& sessionId) {
    return sessions[sessionId];
}
