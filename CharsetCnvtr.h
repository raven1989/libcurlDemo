#include <iostream>
#include <sstream>
#include <errno.h>
#include<iconv.h>

using namespace std;

string
cnvt(const string& _source, const string& _target, const string& str)
{
    //LOG_INFO(cnvtrlogger,"str = "<<str);
    if (_target == _source) {
        return str;
    }
    string result;
    iconv_t conv = iconv_open(_target.c_str(), _source.c_str());
    if (conv == (iconv_t)-1) {
        //throw Ice::SyscallException(__FILE__, __LINE__);
        return "";
    }
    iconv(conv, NULL, NULL, NULL, NULL);

    size_t lengthIn = str.length();
    char *sin = const_cast<char*>(str.c_str());
    char bufferOut[str.length()*2+10];
    char *sout = bufferOut;
    size_t lengthOut = sizeof(bufferOut) - 5;

    while ((lengthIn > 0) && (sin <= str.c_str()+str.length())) {
        if (iconv(conv, &sin, &lengthIn, &sout, &lengthOut) == 0) {
            // ok, iconv success
            sout[0] = '\0';
            result += bufferOut;
            break;
            //return result.str();
        } else if (errno == E2BIG) {
            // the input string from's length is greater than bufferOut's,
            // so, store the result converted and convert the bytes left again.
            sout[0] = '\0';
            result += bufferOut;
            sout = bufferOut;
            lengthOut = sizeof(bufferOut) - 1;
            lengthIn -= strlen(bufferOut);
        } else {
	    // well, encounter the invalid byte, just copy it and continue.
	    sout[0] = '\0';
	    result += bufferOut;
	    sout = bufferOut;
	    ++sin;
	    --lengthIn;
        }
    }
    iconv_close(conv);
    return result;
}

