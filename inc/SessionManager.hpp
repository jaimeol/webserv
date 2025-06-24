
#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <string>
#include <map>
#include <ctime>
#include <cstdlib>
#include <sstream>

class SessionManager {
private:
    std::map<std::string, std::map<std::string, std::string> > sessions;

public:
    SessionManager();

    std::string generateSessionId();
    bool hasSession(const std::string& sessionId) const;
    void createSession(const std::string& sessionId);
    std::map<std::string, std::string>& getSession(const std::string& sessionId);
};

#endif
