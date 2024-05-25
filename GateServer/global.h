#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//char 转为16进制
extern unsigned char ToHex(unsigned char x);

// 16进制转为 char
extern unsigned char FromHex(unsigned char x);

// url 编码
extern std::string UrlEncode(const std::string& str);

// url 解码
extern std::string UrlDecode(const std::string& str);

enum ErrorCodes {
    Success = 0,
    Error_Json = 1001,  //Json解析错误
    RPCFailed = 1002,  //RPC请求错误
};