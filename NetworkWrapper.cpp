#include "NetworkWrapper.h"
#include "curl/curl.h"
extern "C"
{
#include "md5.h"
}
using namespace std;

#define invokeLib(LibFn,arg1,args...) _p->lasterr=LibFn(arg1,##args)

static int _cnt_native_lib=0;

int InitNativeLib()
{
    if(_cnt_native_lib==0 && curl_global_init(CURL_GLOBAL_ALL)!=0)
    {
        return -1;
    }
    else
    {
        _cnt_native_lib++;
        return 0;
    }
}

int CleanUpNativeLib()
{
    if(_cnt_native_lib==1)
    {
        curl_global_cleanup();
        _cnt_native_lib=0;
        return 0;
    }
    else
    {
        _cnt_native_lib--;
        return 0;
    }
}

class HTTPConnection::_impl
{
public:
    CURL* c;
    CURLcode lasterr;
    FILE* delegated_fp;
};

HTTPConnection::HTTPConnection() : _p(new _impl)
{
    _p->c=NULL;
    _p->lasterr=CURLE_OK;
    _p->delegated_fp=NULL;

    _p->c=curl_easy_init();
}

HTTPConnection::~HTTPConnection()
{
    if(_p)
    {
        curl_easy_cleanup(_p->c);

        if(_p->delegated_fp)
        {
            fclose(_p->delegated_fp);
        }

        delete _p;
    }
}

bool HTTPConnection::isReady() const
{
    return _p&&_p->c;
}

int HTTPConnection::setURL(const string& URL)
{
    return invokeLib(curl_easy_setopt,_p->c,CURLOPT_URL,URL.c_str());
}

static size_t _general_writer(char* ptr,size_t sz,size_t n,void* userfn)
{
    int sum=sz*n;
    return (*reinterpret_cast<function<int(char*,int)>*>(userfn))(ptr,sum);
}

int HTTPConnection::setWriter(const function<int(char*,int)>& fn)
{
    invokeLib(curl_easy_setopt,_p->c,CURLOPT_WRITEFUNCTION,_general_writer);
    invokeLib(curl_easy_setopt,_p->c,CURLOPT_WRITEDATA,&fn);
    return 0;
}

int HTTPConnection::setOutputFile(const string& filename)
{
    FILE* fp=fopen(filename.c_str(),"w");
    if(!fp) return -2;

    invokeLib(curl_easy_setopt,_p->c,CURLOPT_WRITEFUNCTION,fwrite);
    invokeLib(curl_easy_setopt,_p->c,CURLOPT_WRITEDATA,fp);

    _p->delegated_fp=fp;

    return 0;
}

int HTTPConnection::setTimeout(int second)
{
    return curl_easy_setopt(_p->c,CURLOPT_TIMEOUT,second);
}

int HTTPConnection::setVerbos(bool v)
{
    if(v)
    {
        return invokeLib(curl_easy_setopt,_p->c,CURLOPT_VERBOSE,1);
    }
    else
    {
        return invokeLib(curl_easy_setopt,_p->c,CURLOPT_VERBOSE,0);
    }
}

int HTTPConnection::perform()
{
    return curl_easy_perform(_p->c);
}

int HTTPConnection::getLastErrCode()
{
    return _p->lasterr;
}

string HTTPConnection::getLastError()
{
    return curl_easy_strerror(_p->lasterr);
}

string getMD5(const string& filename)
{
    MD5_CTX c;
    md5_init(&c);
    FILE* fp=fopen(filename.c_str(),"rb");
    char buff[128];
    memset(buff,0,128);
    int len;
    while((len=fread(buff,1,128,fp))>0)
    {
        md5_update(&c,(unsigned char*)buff,len);
        memset(buff,0,128);
    }
    md5_final(&c,(unsigned char*)buff);
    string result;
    for(int i=0;i<16;i++)
    {
        int v=(unsigned char)buff[i];
        result.push_back(v/16<10?v/16+'0':v/16-10+'A');
        result.push_back(v%16<10?v%16+'0':v%16-10+'A');
    }
    return result;
}
