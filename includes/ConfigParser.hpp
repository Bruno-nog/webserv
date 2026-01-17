#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Config.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>

class ConfigParser {
private:
    std::string              _filePath;
    std::vector<std::string> _tokens;
    size_t                   _currentTokenIndex;
    void tokenize();
    void parseServer(ServerConfig &server);
    void parseLocation(ServerConfig &server);
    std::string nextToken();
    std::string peekToken();
    bool expect(const std::string& token);

public:
    ConfigParser(const std::string& filePath);
    ~ConfigParser();
    std::vector<ServerConfig> parse();
};

#endif