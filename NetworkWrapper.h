#pragma once
#include <string>
#include <functional>

class HTTPConnection
{
public:
    HTTPConnection();
    ~HTTPConnection();

    bool isReady() const;

    int setURL(const std::string& URL);
    int setWriter(const std::function<int(char*,int)>& fn);
    int setOutputFile(const std::string& filename);
    int setTimeout(int second);
    int setVerbos(bool v);

    int perform();

    int getLastErrCode();
    std::string getLastError();
private:
    class _impl;
    _impl* _p;
};

std::string getMD5(const std::string& filename);
