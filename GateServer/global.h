#pragma once
#include <boost/beast/http.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>
#include "Singleton.h"
#include <functional>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <thread>
#include <condition_variable>
#include <queue>
#include "hiredis.h"
#include <cassert>

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

//class ConfigMgr;
//extern ConfigMgr gCfgMgr;   // 声明一个全局的 ConfigMgr 类对象

// 测试 redis 环境是否配置成功
extern void TestRedis();

// 测试 RedisMgr 类功能
extern void TestRedisMgr();