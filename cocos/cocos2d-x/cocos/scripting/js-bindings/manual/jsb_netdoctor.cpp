/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "base/ccConfig.h"
#include "jsb_netdoctor.hpp"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC || CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"
#include "cocos/scripting/js-bindings/manual/jsb_conversions.hpp"
#include "cocos/scripting/js-bindings/manual/jsb_global.h"

#include "base/ccUTF8.h"
#include "platform/CCApplication.h"
#include "base/CCScheduler.h"
#include <vector>
#include <string>
#include <numeric> // 引入头文件以使用std::accumulate
#include <iostream>
#include <thread>

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/time.h>

#include <queue>
#include <sstream>
#include <errno.h>

#define DEFAULT_TIMEOUT 5 * 1000   // 单个域名进行网络连接的超时时间，单位：毫秒
#define DEFAULT_INTERVAL 60 * 1000 // 所有域名完成测速到下次测速开始的间隔时间，单位：毫秒
#define DEFAULT_PORT 80
#define DEFAULT_PATH "/"
#define BUFSIZE 0xF000

using namespace cocos2d;
using namespace cocos2d::network;

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

#include "platform/android/jni/JniHelper.h"
#ifndef JCLS_UTILS
#define JCLS_UTILS "eggy/cocos2dx/custom/Utils"
#endif

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)

#include "platform/ios/CCReachability.h"
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS

char *time_to_string(struct timeval tv)
{
    struct tm *tm_ptr;
    static char time_str[26];
    memset(time_str, 0, sizeof(char) * 26);
    tm_ptr = localtime(&tv.tv_sec);
    strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_ptr);
    snprintf(time_str + 20, 6, ".%06ld", tv.tv_usec);

    return time_str;
}

static std::string convert_to_string(long long int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

static std::string vector_to_stirng(std::vector<std::string> vector)
{
    std::string strings = std::accumulate(vector.begin() + 1, vector.end(), vector.front(),
                                          [](std::string a, std::string b)
                                          { return a + ", " + b; });
    return strings;
}

static std::string vector_to_json_array(std::vector<std::string> vector)
{
    std::string json_string;
    json_string.append("[");
    for (size_t i = 0; i < vector.size(); ++i)
    {
        json_string.append("\"").append(vector[i]).append("\"");
        if (i != vector.size() - 1)
        {
            json_string.append(",");
        }
    }
    json_string.append("]");
    return json_string;
}

static std::string map_to_json(std::map<std::string, std::vector<std::string>> map)
{
    std::string map_string;
    std::map<std::string, std::vector<std::string>>::iterator iter;
    map_string.append("{\n");

    for (iter = map.begin(); iter != map.end(); ++iter)
    {
        std::string key = iter->first;
        std::vector<std::string> value = iter->second;
        std::string json_array = vector_to_json_array(value);
        map_string.append("\"").append(key).append("\"").append(": ").append(json_array);
        map_string.append(",\n");
    }
    map_string.append("}");
    return map_string;
}

static bool args_to_map(const se::Value &args, std::map<std::string, std::vector<std::string>> *map)
{
    assert(map != nullptr);
    assert(args.isObject());
    se::Object *argObj = args.toObject();

    std::vector<std::string> keys;
    bool ok = true;
    ok &= argObj->getAllKeys(&keys);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor fail to get all keys from object \n");

    for (const auto &key : keys)
    {
        se::Value value;
        if (argObj->getProperty(key.c_str(), &value) && value.isObject())
        {
            se::Object *array_obj = value.toObject();
            std::vector<std::string> domains_vector;
            if (array_obj->isArray())
            {
                uint32_t len = 0;
                array_obj->getArrayLength(&len);
                for (uint32_t i = 0; i < len; ++i)
                {
                    se::Value val;
                    if (array_obj->getArrayElement(i, &val) && val.isString())
                    {
                        std::string domain;
                        ok &= seval_to_std_string(val, &domain);
                        SE_PRECONDITION2(ok, false, "JSB: NetDoctor parse domain fail, Error processing arguments \n");
                        if (ok)
                        {
                            domains_vector.push_back(domain);
                        }
                    }
                    else
                    {
                        SE_LOGE("JSB: NetDoctor object[%s]->domain[%d] is not string \n", key.c_str(), i);
                    }
                }
            }
            else
            {
                SE_LOGD("JSB: NetDoctor object is not array for key: %s \n", key.c_str());
            }

            map->insert(std::map<std::string, std::vector<std::string>>::value_type(key, domains_vector));
        }
    }
    return true;
}

bool check_network_connected()
{
    bool connected = true;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)

    JniMethodInfo methodInfo;
    if (JniHelper::getStaticMethodInfo(methodInfo,
                                       JCLS_UTILS,
                                       "isNetworkConnected",
                                       "()Z"))
    {
        connected = methodInfo.env->CallStaticBooleanMethod(methodInfo.classID, methodInfo.methodID);
        methodInfo.env->DeleteLocalRef(methodInfo.classID);
    }
    else
    {
        CCLOGERROR("NetDoctor::%s failed! \n", __FUNCTION__);
    }
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    static Reachability *__reachability = nullptr;
    if (__reachability == nullptr)
    {
        __reachability = Reachability::createForInternetConnection();
        __reachability->retain();
    }

    Reachability::NetworkStatus status = __reachability->getCurrentReachabilityStatus();
    if (status == Reachability::NetworkStatus::NOT_REACHABLE)
    {
        connected = false;
    }
#endif // CC_TARGET_PLATFORM == CC_PLATFORM_IOS
    return connected;
}

void print_hostnet(struct hostent *phostnet)
{

    SE_LOGD("CPP: NetDoctor =》host(%s) info as below: \n", phostnet->h_name);
    SE_LOGD("CPP: NetDoctor =》h_addrtype: %d \n", phostnet->h_addrtype);
    SE_LOGD("CPP: NetDoctor =》h_length: %d  \n", phostnet->h_length);
    char **patr = phostnet->h_aliases;
    for (int index = 0; *patr != NULL; patr++, index++)
    {
        SE_LOGD("CPP: NetDoctor =》h_aliases[%d]: %s \n", index, *patr);
    }
    switch (phostnet->h_addrtype)
    {
    case AF_INET:
    case AF_INET6:
    {
        SE_LOGD("CPP: NetDoctor =》h_addr: %s \n", inet_ntoa(*((struct in_addr *)phostnet->h_addr)));
        char **pptr = phostnet->h_addr_list;
        for (int index = 0; *pptr != NULL; pptr++, index++)
        {
            SE_LOGD("CPP: NetDoctor =》h_addr_list[%d]: %s \n", index, inet_ntoa(*((struct in_addr *)pptr)));
        }
        break;
    }
    default:
        SE_LOGD("CPP: NetDoctor =》unhandled address type: %d \n", phostnet->h_addrtype);
        break;
    }
}

/**
 * 分离url中的主机地址、端口号、相对路径
 */
void parse_url(const char *url, char *host, int *port, char *path)
{
    int char_len = strlen(url) + 1; // 算上 '\0'
    char *myurl = new char[char_len];
    char *pHost = 0;
    char host_tmp[char_len], port_tmp[char_len], path_tmp[char_len];
    memset(myurl, 0, sizeof(char) * char_len);
    memset(host_tmp, 0, sizeof(char) * char_len);
    memset(port_tmp, 0, sizeof(char) * char_len);
    memset(path_tmp, 0, sizeof(char) * char_len);
    strcpy(myurl, url);
    bool portFound = false;
    bool pathFound = false;
    int host_len = 0;
    int port_len = 0;
    int path_len = 0;
    for (pHost = myurl; *pHost != '\0'; ++pHost)
    {
        if (*pHost == ':') // 端口号
        {
            portFound = true;
            continue;
        }
        else if (*pHost == '/') // 路径
        {
            pathFound = true;
            path_tmp[path_len++] = *pHost;
            continue;
        }

        if (!portFound && !pathFound)
        {
            host_tmp[host_len++] = *pHost;
        }
        else if (portFound && !pathFound)
        {
            port_tmp[port_len++] = *pHost;
        }
        else if (pathFound)
        {
            path_tmp[path_len++] = *pHost;
        }
    };
    host_tmp[host_len] = '\0';
    port_tmp[port_len] = '\0';
    path_tmp[path_len] = '\0';
    if (port_tmp[0] == '\0')
    {
        strcpy(port_tmp, "0");
    }
    delete[] myurl; // delete 防止内存泄漏

    strcpy(host, host_tmp);
    strcpy(path, path_tmp);
    int port_int = atoi(port_tmp);
    *port = port_int;
}

/**
 * 检查域名连接耗时
 *
 * return 返回值如下所示：
 * 连接耗时(单位：毫秒)：域名连接成功的情况
 *
 * -1: 设备已连网，但域名无法连接时，包括以下情况：
 *     情况1：设备只是连接wifi并不能上网，此时无法访问外网
 *     情况2：域名无法通过DNS解析，可能被劫持或者域名不存在
 *     情况3；域名服务器不可达，服务器问题
 *
 * -2: 设备未连网，指的是设备没有接入任何wifi或有线网络。
 */
int check_network_latency(const char *host, const int port, const char *path, long timeout_milliseconds)
{
    int fd;
    char response[BUFSIZE];
    struct addrinfo hints, *res;
    struct sockaddr_in servaddr;
    struct hostent *phostnet;
    unsigned int inaddr = 0l;
    struct in_addr *ipaddr;
    struct timeval start;
    struct timeval end;
    struct timeval timeout;
    /* （timeout_seconds * 1000 + timeout_microseconds / 1000) = timeout_milliseconds */
    long timeout_seconds = timeout_milliseconds / 1000;
    long timeout_microseconds = (timeout_milliseconds % 1000) * 1000;

    SE_LOGD("CPP: NetDoctor =》start checking network latency for host: %s, port: %d, path: %s, timeout: %lus + %luus = %lums \n",
            host, port, path, timeout_seconds, timeout_microseconds, timeout_milliseconds);

    /*判断设备的网络是否连接*/
    if (!check_network_connected())
    {
        SE_LOGD("CPP: NetDoctor =》network disconnected \n");
        return -2;
    }

    /*先检查域名能否进行DNS解析*/
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // 无特定地址族要求
    hints.ai_socktype = SOCK_STREAM; // 流式套接字

    int status = getaddrinfo(host, nullptr, &hints, &res);
    if (status != 0)
    {
        SE_LOGE("CPP: NetDoctor =》getaddrinfo(%s) error: %s \n", host, gai_strerror(status));
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        SE_LOGE("CPP: NetDoctor =》create socket(%s) failed! \n", host);
        return -1;
    }

    /*设置默认服务器的信息*/
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    memset(servaddr.sin_zero, 0, sizeof(servaddr.sin_zero));

    /*判断是主机名还是ip地址*/
    if ((inaddr = inet_addr(host)) == INADDR_NONE) /*是主机名*/
    {
        if ((phostnet = gethostbyname(host)) == NULL)
        {
            SE_LOGE("CPP: NetDoctor =》gethostbyname(%s) error \n", host);
            return -1;
        }
        // print_hostnet(phostnet);
        // 方法1：
        ipaddr = (struct in_addr *)phostnet->h_addr;
        servaddr.sin_addr.s_addr = (ipaddr->s_addr);
        // 方法2：这个算是多此一举了！多了一句无用功
        // inaddr = inet_addr(inet_ntoa(*ipaddr));
        // servaddr.sin_addr.s_addr = inaddr;
    }
    else /*是ip地址*/
    {
        servaddr.sin_addr.s_addr = inaddr;
    }
    gettimeofday(&start, nullptr);

    // 以下操作主要为connect函数设置超时
    // 设置为非阻塞
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    /* connect 函数 */
    if (connect(fd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0)
    {
        if (errno != EINPROGRESS)
        {
            SE_LOGE("CPP: NetDoctor =》connect host(%s) failed! \n ", host);
            close(fd);
            return -1;
        }
    }

    // 恢复阻塞模式
    fcntl(fd, F_SETFL, flags);

    // 使用select来检查连接是否在指定时间内完成
    struct timeval tv;
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    tv.tv_sec = timeout_seconds;
    tv.tv_usec = timeout_microseconds;

    if (select(fd + 1, NULL, &writefds, NULL, &tv) > 0)
    {
        int error = 0;
        socklen_t len = sizeof(error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
        {
            SE_LOGE("CPP: NetDoctor =》getsockopt for host(%s) failed! \n", host);
            close(fd);
            return -1;
        }
        if (error != 0)
        {
            SE_LOGE("CPP: NetDoctor =》connect host(%s) failed: %s \n", host, strerror(error));
            close(fd);
            return -1;
        }
        // 连接成功
        SE_LOGD("CPP: NetDoctor =》connect host(%s) success \n", host);
    }
    else
    {
        // 连接超时
        if (tv.tv_sec <= 0)
        {
            SE_LOGE("CPP: NetDoctor =》connect host(%s) timeout! \n", host);
            close(fd);
            return -1;
        }
        // 连接失败
        SE_LOGE("CPP: NetDoctor =》connect host(%s) failed: %s \n", host, strerror(errno));
        close(fd);
        return -1;
    }

    // 向服务器发送url请求的request
    char *request;
    const char *REQUEST_FORMAT =
        "GET %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n";

    // 合成请求报文
    int request_len = asprintf(&request, REQUEST_FORMAT, path, host, port);
    if (request_len < 0)
    {
        SE_LOGE("CPP: NetDoctor =》assemble request for host(%s) failed! \n", host);
        close(fd);
        return -1;
    }
    SE_LOGD("CPP: NetDoctor =》send data to host(%s): ==>%s<==length: %d \n", host, request, request_len);
    int cs;
    if (-1 == (cs = send(fd, request, request_len, 0)))
    {
        SE_LOGE("CPP: NetDoctor =》send data to host(%s) failed, reason: %d \n", host, cs);
        close(fd);
        return -1;
    }
    SE_LOGD("CPP: NetDoctor =》send data to host(%s) success: %dbytes \n", host, cs);

    // Set timeout for recv() function
    // struct timeval 结构表示时长是用秒和微秒两个值的相加（tv_sec + tv_usec）
    timeout.tv_sec = timeout_seconds;       // timeout in seconds
    timeout.tv_usec = timeout_microseconds; // timeout in microseconds
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof timeout) < 0)
    {
        SE_LOGE("CPP: NetDoctor =》setsockopt to host(%s) failed! \n", host);
        close(fd);
        return -1;
    }

    // 客户端接收服务器的返回信息
    int recv_bytes;
    memset(response, 0, BUFSIZE);
    if (-1 == (recv_bytes = recv(fd, response, BUFSIZE, 0)))
    {
        SE_LOGD("CPP: NetDoctor《= receive data from host(%s) failed! \n", host);
        close(fd);
        return -1;
    }
    SE_LOGD("CPP: NetDoctor《= receive data from host(%s) success: %dbytes \n", host, recv_bytes);
    gettimeofday(&end, nullptr);

    int cost_sec = end.tv_sec - start.tv_sec;
    int cost_usec = end.tv_usec - start.tv_usec;
    int cost_msec = cost_sec * 1000 + (float)cost_usec / 1000;
    SE_LOGD("CPP: NetDoctor =》connect to host(%s) ok: %d ms \n", host, cost_msec);

    // float total_bytes_kb = (float)recv_bytes / 1024;
    // float total_sec = (float)cost_msec / 1000;
    // float speed = total_bytes_kb / total_sec;
    // SE_LOGD("CPP: NetDoctor =》%s: receive %d bytes in %d ms [%f kb/s] \n", host, recv_bytes, cost_msec, speed);

    close(fd);
    return cost_msec;
}

NS_CC_BEGIN
namespace network
{
    NetDoctor::NetDoctor()
    {
        SE_LOGD("CPP: NetDoctor (%p) constructor invoked: (no parameters) \n", this);
        _threadStopFlag.store(true);
        _isThreadRunning = false;
        interval_milliseconds = DEFAULT_INTERVAL;
        timeout_milliseconds = DEFAULT_TIMEOUT;
        addr_port = DEFAULT_PORT;
        addr_path = DEFAULT_PATH;
        _isDestroyed = std::make_shared<std::atomic<bool>>(false);
        gettimeofday(&_last_time, nullptr);
        _total_count = 0;
    }

    NetDoctor::NetDoctor(std::map<std::string, std::vector<std::string>> &domains)
    {
        std::string domains_json = map_to_json(domains);
        SE_LOGD("CPP: NetDoctor (%p) constructor invoked: %s \n", this, domains_json.c_str());
        _domains = domains;
        _threadStopFlag.store(true);
        _isThreadRunning = false;
        interval_milliseconds = DEFAULT_INTERVAL;
        timeout_milliseconds = DEFAULT_TIMEOUT;
        addr_port = DEFAULT_PORT;
        addr_path = DEFAULT_PATH;
        _isDestroyed = std::make_shared<std::atomic<bool>>(false);
        gettimeofday(&_last_time, nullptr);
        _total_count = 0;
    }

    NetDoctor::~NetDoctor()
    {
        SE_LOGD("CPP: NetDoctor (%p) destructor invoked \n", this);
        _domains.clear();
        _sortedDomains.clear();
        _threadStopFlag.store(true);
        _isThreadRunning = false;
        *_isDestroyed = true;
    }

    void NetDoctor::update(std::map<std::string, std::vector<std::string>> &domains)
    {
        std::string domains_json = map_to_json(domains);
        SE_LOGD("CPP: NetDoctor (%p) update domains: %s \n", this, domains_json.c_str());

        _domainsMutex.lock();
        _domains = domains;
        _domainsMutex.unlock();
    }

    // 线程的工作函数
    void NetDoctor::diagnose()
    {
        SE_LOGD("CPP: NetDoctor (%p) start diagnosis thread \n", this);

        while (!_threadStopFlag.load())
        {
            _isThreadRunning = true;
            struct timeval now_time;
            memset(&now_time, 0, sizeof(struct timeval));
            gettimeofday(&now_time, nullptr);

            char *now_time_str = time_to_string(now_time);
            char *last_time_str = time_to_string(this->_last_time);
            SE_LOGD("CPP: NetDoctor (%p) diagnosis thread is running, last_time: %s, total_count: %lu \n",
                    this, last_time_str, this->_total_count);

            _domainsMutex.lock();
            std::map<std::string, std::vector<std::string>>::iterator iter;
            for (iter = _domains.begin(); iter != _domains.end(); ++iter)
            {
                std::string game_id = iter->first;
                std::vector<std::string> domain_vector = iter->second;

                std::vector<NetDoctor::Domain> check_result_vector;
                for (size_t i = 0; i < domain_vector.size(); ++i)
                {
                    std::string url = domain_vector[i];
                    /**
                     * 1. 先从url分离出host, port, path
                     * 2. 进行网络测速时，优先使用url自带的端口和路径
                     * 3. url没有带的时候，使用从js层传入的预设端口和路径
                     */
                    int char_len = url.size() + 1; // +1是考虑到url只有host的情况，需要包含 '\0'
                    char host[char_len], path[char_len];
                    int port = 0;
                    memset(host, 0, char_len);
                    memset(path, 0, char_len);
                    parse_url(url.c_str(), host, &port, path);
                    SE_LOGD("CPP: NetDoctor (%p) parse url(%s) finish: host = %s, port = %d, path = %s \n", this, url.c_str(), host, port, path);

                    if (port <= 0)
                    {
                        int default_port = this->addr_port;
                        if (default_port <= 0)
                        {
                            default_port = DEFAULT_PORT;
                        }
                        port = default_port;
                    }

                    if (strlen(path) == 0)
                    {
                        std::string default_path = this->addr_path;
                        if (default_path.empty())
                        {
                            default_path = DEFAULT_PATH;
                        }
                        strcpy(path, default_path.c_str());
                    }
                    long timeout_milliseconds = this->timeout_milliseconds;
                    if (timeout_milliseconds <= 0)
                    {
                        timeout_milliseconds = DEFAULT_TIMEOUT;
                    }

                    long latency = check_network_latency(host, port, path, timeout_milliseconds);
                    /* 即使传进来的url带有port和path，输出到js层的结果总是以host作为key */
                    check_result_vector.push_back({host, latency});

                    if (latency > 0)
                    {
                        SE_LOGD("CPP: NetDoctor (%p) check url(%s) latency finish: latency = %lums \n", this, url.c_str(), latency);
                    }
                    else
                    {
                        SE_LOGE("CPP: NetDoctor (%p) check url(%s) latency finish: domain is unreachable \n", this, url.c_str());
                    }
                }

                if (check_result_vector.size() > 0)
                {
                    // 使用lambda表达式进行自定义排序（按延迟时间升序）
                    std::sort(check_result_vector.begin(), check_result_vector.end(), [](const NetDoctor::Domain &a, const NetDoctor::Domain &b)
                              { if(a.latency >0 && b.latency <= 0){
                                    return true;
                                 } else if(a.latency <= 0 && b.latency > 0){
                                   return false;
                                 }else {
                                    return a.latency < b.latency;
                                 } });

                    // 以下代码只是打印排序日志
                    // std::vector<std::string> log_string;
                    // for (size_t i = 0; i < check_result_vector.size(); i++)
                    // {
                    //     NetDoctor::Domain result = check_result_vector[i];
                    //     // 拼接日志
                    //     std::string log;
                    //     log.append(result.domain).append("(").append(convert_to_string(result.latency));
                    //     if (result.latency > 0)
                    //     {
                    //         log.append("ms");
                    //     }
                    //     log.append(")");
                    //     log_string.push_back(log);
                    // }
                    // SE_LOGD("CPP: NetDoctor (%p) sorted domains for gameId(%s): %s \n", this, game_id.c_str(), vector_to_stirng(log_string).c_str());

                    _sortedDomains[game_id] = check_result_vector;
                }
            }
            std::shared_ptr<std::atomic<bool>> isDestroyed = _isDestroyed;
            cocos2d::Application::getInstance()->getScheduler()->performFunctionInCocosThread([this, isDestroyed]()
                                                                                              {
                if (*isDestroyed)
                {
                    SE_LOGD("CPP: NetDoctor instance was destroyed!\n");
                }
                else
                {
                    _callback.onSorted(this, _sortedDomains);
                } });

            _domainsMutex.unlock();

            // 记录下本次完成检测的时间和检测次数
            memset(&this->_last_time, 0, sizeof(struct timeval));
            gettimeofday(&this->_last_time, nullptr);
            _total_count++;

            long sleep_milliseconds = this->interval_milliseconds;
            if (sleep_milliseconds <= 0)
            {
                sleep_milliseconds = DEFAULT_INTERVAL;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_milliseconds));
        }

        _isThreadRunning = false;
        SE_LOGD("CPP: NetDoctor (%p) stop diagnosis thread \n", this);
    }

    void NetDoctor::setCallback(NetDoctorCallback &callback)
    {
        _callback = callback;
    }

    void NetDoctor::start()
    {
        _threadStopFlag.store(false);
        // 不要重复开启线程
        if (!_isThreadRunning)
        {
            // 创建线程对象，它将执行diagnose方法
            std::thread diagnoseThread(&NetDoctor::diagnose, this);
            diagnoseThread.detach();
        }
    }

    void NetDoctor::stop()
    {
        _threadStopFlag.store(true);
    }

    std::vector<NetDoctor::Domain> NetDoctor::getSortedDomains(std::string key)
    {
        return _sortedDomains[key];
    }

    std::map<std::string, std::vector<NetDoctor::Domain>> NetDoctor::getAllSortedDomains()
    {
        return _sortedDomains;
    }

    NetDoctor::NetDoctorCallback::NetDoctorCallback()
    {
    }

    NetDoctor::NetDoctorCallback::~NetDoctorCallback()
    {
    }

    void NetDoctor::NetDoctorCallback::setJSDelegate(const se::Value &jsDelegate)
    {
        assert(jsDelegate.isObject());
        _JSDelegate = jsDelegate;
    }

    void NetDoctor::NetDoctorCallback::onSorted(NetDoctor *netdoctor, std::map<std::string, std::vector<NetDoctor::Domain>> sortedDomains)
    {
        se::ScriptEngine::getInstance()->clearException();
        se::AutoHandleScope hs;

        if (cocos2d::Application::getInstance() == nullptr)
            return;

        auto iter = se::NativePtrToObjectMap::find(netdoctor);
        if (iter == se::NativePtrToObjectMap::end())
            return;

        se::Object *netdoctorObj = iter->second;
        se::HandleObject jsObj(se::Object::createPlainObject());
        se::Value target;
        native_ptr_to_seval<NetDoctor>(netdoctor, &target);
        jsObj->setProperty("target", target);

        se::Value func;
        bool ok = _JSDelegate.toObject()->getProperty("onsorted", &func);
        if (ok && func.isObject() && func.toObject()->isFunction())
        {
            se::ValueArray args;
            args.push_back(se::Value(jsObj));

            se::Value retValue;
            ok &= std_map_string_std_vector_domain_to_seval(sortedDomains, &retValue);
            jsObj->setProperty("domains", retValue);

            func.toObject()->call(args, netdoctorObj);
        }
        else
        {
            SE_REPORT_ERROR("CPP: NetDoctorCallback (%p) Can't get onsorted function!", this);
        }
    }

}
NS_CC_END

se::Class *__jsb_NetDoctor_class = nullptr;

static bool NetDoctor_finalize(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_LOGD("JSB: finalizing JS object NetDoctor (%p) \n", cobj);

    if (cobj->getReferenceCount() == 1)
    {
        cobj->autorelease();
    }
    else
    {
        cobj->release();
    }

    return true;
}
SE_BIND_FINALIZE_FUNC(NetDoctor_finalize)

static bool NetDoctor_constructor(se::State &s)
{
    se::Object *object = s.thisObject();
    const auto &args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    if (argc == 0)
    {
        // 无参数构造器
        NetDoctor *cobj = new (std::nothrow) NetDoctor();
        NetDoctor::NetDoctorCallback *callback = new (std::nothrow) NetDoctor::NetDoctorCallback();
        callback->setJSDelegate(se::Value(object, true));
        cobj->setCallback(*callback);

        cobj->retain();     // release in finalize function and onClose delegate method
        callback->retain(); // release in finalize function and onClose delegate method

        object->setPrivateData(cobj);
        return true;
    }
    else if (argc == 1)
    {
        // 有参数构造器
        std::map<std::string, std::vector<std::string>> args_map;
        args_to_map(args[0], &args_map);
        NetDoctor *cobj = new (std::nothrow) NetDoctor(args_map);
        NetDoctor::NetDoctorCallback *callback = new (std::nothrow) NetDoctor::NetDoctorCallback();
        callback->setJSDelegate(se::Value(object, true));
        cobj->setCallback(*callback);

        cobj->retain();     // release in finalize function and onClose delegate method
        callback->retain(); // release in finalize function and onClose delegate method

        object->setPrivateData(cobj);
        return true;
    }

    SE_REPORT_ERROR("JSB: NetDoctor construct fail, wrong number of arguments: %d, was expecting %d or %d \n", argc, 0, 1);

    return false;
}
SE_BIND_CTOR(NetDoctor_constructor, __jsb_NetDoctor_class, NetDoctor_finalize)

static bool NetDoctor_update(se::State &s)
{
    const auto &args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    if (argc == 1)
    {
        std::map<std::string, std::vector<std::string>> args_map;
        args_to_map(args[0], &args_map);
        cobj->update(args_map);
        return true;
    }
    SE_REPORT_ERROR("JSB: NetDoctor (%p) update domains fail, wrong number of arguments: %d, was expecting %d \n", cobj, argc, 1);
    return false;
}
SE_BIND_FUNC(NetDoctor_update)

static bool NetDoctor_start(se::State &s)
{
    SE_LOGD("JSB: NetDoctor start diagnosis thread \n");
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    cobj->start();
    return true;
}
SE_BIND_FUNC(NetDoctor_start)

static bool NetDoctor_stop(se::State &s)
{
    SE_LOGD("JSB: NetDoctor stop diagnosis thread \n");
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    cobj->stop();
    return true;
}
SE_BIND_FUNC(NetDoctor_stop)

static bool NetDoctor_get_sorted_domains(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_get_sorted_domains : Invalid Native Object \n");
    const auto &args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    if (argc == 1)
    {
        std::string arg0;
        ok &= seval_to_std_string(args[0], &arg0);
        SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_sorted_domains : Error processing arguments \n");

        std::string key_str = arg0;
        std::vector<NetDoctor::Domain> sorted_domains = cobj->getSortedDomains(key_str);
        // 拷贝一份，避免污染原始数据
        std::vector<NetDoctor::Domain> ret_sorted_domains(sorted_domains);
        ok &= std_vector_domain_to_seval(ret_sorted_domains, &s.rval());
        SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_sorted_domains : Error processing return value \n");
        return true;
    }
    SE_REPORT_ERROR("JSB: NetDoctor (%p) NetDoctor_get_sorted_domains fail, wrong number of arguments: %d, was expecting %d \n", cobj, argc, 1);
    return false;
}
SE_BIND_FUNC(NetDoctor_get_sorted_domains)

static bool NetDoctor_get_all_sorted_domains(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_get_all_sorted_domains : Invalid Native Object \n");
    const auto &args = s.args();
    int argc = (int)args.size();
    bool ok = true;
    if (argc == 0)
    {
        std::map<std::string, std::vector<NetDoctor::Domain>> all_sorted_domains = cobj->getAllSortedDomains();
        // 拷贝一份，避免污染原始数据
        std::map<std::string, std::vector<NetDoctor::Domain>> ret_all_sorted_domains(all_sorted_domains);
        ok &= std_map_string_std_vector_domain_to_seval(ret_all_sorted_domains, &s.rval());

        SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_all_sorted_domains : Error processing return value \n");
        return true;
    }
    SE_REPORT_ERROR("JSB: NetDoctor (%p) NetDoctor_get_all_sorted_domains fail, wrong number of arguments: %d, was expecting %d \n", cobj, argc, 0);
    return false;
}
SE_BIND_FUNC(NetDoctor_get_all_sorted_domains)

static bool NetDoctor_get_interval_milliseconds(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_get_interval_milliseconds : Invalid Native Object \n");

    CC_UNUSED bool ok = true;
    se::Value jsret;
    ok &= long_to_seval(cobj->interval_milliseconds, &jsret);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_interval_milliseconds : Error processing return value \n");
    s.rval() = jsret;
    SE_LOGD("JSB: NetDoctor_get_interval_milliseconds: %lu \n", cobj->interval_milliseconds);
    return true;
}
SE_BIND_PROP_GET(NetDoctor_get_interval_milliseconds)

static bool NetDoctor_set_interval_milliseconds(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_set_interval_milliseconds : Invalid Native Object \n");
    const auto &args = s.args();
    int argc = (int)args.size();

    CC_UNUSED bool ok = true;
    long arg0;
    ok &= seval_to_long(args[0], &arg0);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_set_interval_milliseconds : Error processing new value \n");
    cobj->interval_milliseconds = arg0;
    SE_LOGD("JSB: NetDoctor_set_interval_milliseconds: %lu \n", arg0);
    return true;
}
SE_BIND_PROP_SET(NetDoctor_set_interval_milliseconds)

static bool NetDoctor_get_timeout_milliseconds(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_get_timeout_milliseconds : Invalid Native Object \n");

    CC_UNUSED bool ok = true;
    se::Value jsret;
    ok &= long_to_seval(cobj->timeout_milliseconds, &jsret);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_timeout_milliseconds : Error processing return value \n");
    s.rval() = jsret;
    SE_LOGD("JSB: NetDoctor_get_timeout_milliseconds: %lu \n", cobj->timeout_milliseconds);
    return true;
}
SE_BIND_PROP_GET(NetDoctor_get_timeout_milliseconds)

static bool NetDoctor_set_timeout_milliseconds(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_set_timeout_milliseconds : Invalid Native Object \n");
    const auto &args = s.args();
    int argc = (int)args.size();

    CC_UNUSED bool ok = true;
    long arg0;
    ok &= seval_to_long(args[0], &arg0);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_set_timeout_milliseconds : Error processing new value \n");
    cobj->timeout_milliseconds = arg0;
    SE_LOGD("JSB: NetDoctor_set_timeout_milliseconds: %lu \n", arg0);
    return true;
}
SE_BIND_PROP_SET(NetDoctor_set_timeout_milliseconds)

static bool NetDoctor_get_addr_port(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_get_addr_port : Invalid Native Object \n");

    CC_UNUSED bool ok = true;
    se::Value jsret;
    ok &= int32_to_seval(cobj->addr_port, &jsret);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_addr_port : Error processing return value \n");
    s.rval() = jsret;
    SE_LOGD("JSB: NetDoctor_get_addr_port: %d \n", cobj->addr_port);
    return true;
}
SE_BIND_PROP_GET(NetDoctor_get_addr_port)

static bool NetDoctor_set_addr_port(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_set_addr_port : Invalid Native Object \n");
    const auto &args = s.args();
    int argc = (int)args.size();

    CC_UNUSED bool ok = true;
    int arg0;
    ok &= seval_to_int32(args[0], &arg0);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_set_addr_port : Error processing new value \n");
    cobj->addr_port = arg0;
    SE_LOGD("JSB: NetDoctor_set_addr_port: %d \n", arg0);
    return true;
}
SE_BIND_PROP_SET(NetDoctor_set_addr_port)

static bool NetDoctor_get_addr_path(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_get_addr_path : Invalid Native Object \n");

    CC_UNUSED bool ok = true;
    se::Value jsret;
    ok &= std_string_to_seval(cobj->addr_path, &jsret);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_get_addr_path : Error processing return value \n");
    s.rval() = jsret;
    SE_LOGD("JSB: NetDoctor_get_addr_path: %s \n", cobj->addr_path.c_str());
    return true;
}
SE_BIND_PROP_GET(NetDoctor_get_addr_path)

static bool NetDoctor_set_addr_path(se::State &s)
{
    NetDoctor *cobj = (NetDoctor *)s.nativeThisObject();
    SE_PRECONDITION2(cobj, false, "JSB: NetDoctor_set_addr_path : Invalid Native Object \n");
    const auto &args = s.args();
    int argc = (int)args.size();

    CC_UNUSED bool ok = true;
    std::string arg0;
    ok &= seval_to_std_string(args[0], &arg0);
    SE_PRECONDITION2(ok, false, "JSB: NetDoctor_set_addr_path : Error processing new value \n");
    cobj->addr_path = arg0;
    SE_LOGD("JSB: NetDoctor_set_addr_path: %s \n", arg0.c_str());
    return true;
}
SE_BIND_PROP_SET(NetDoctor_set_addr_path)

bool register_all_netdoctor(se::Object *global)
{
    se::Class *cls = se::Class::create("NetDoctor", global, nullptr, _SE(NetDoctor_constructor));
    cls->defineFinalizeFunction(_SE(NetDoctor_finalize));

    cls->defineFunction("update", _SE(NetDoctor_update));
    cls->defineFunction("start", _SE(NetDoctor_start));
    cls->defineFunction("stop", _SE(NetDoctor_stop));
    cls->defineFunction("getSortedDomains", _SE(NetDoctor_get_sorted_domains));
    cls->defineFunction("getAllSortedDomains", _SE(NetDoctor_get_all_sorted_domains));
    cls->defineProperty("interval_milliseconds", _SE(NetDoctor_get_interval_milliseconds), _SE(NetDoctor_set_interval_milliseconds));
    cls->defineProperty("timeout_milliseconds", _SE(NetDoctor_get_timeout_milliseconds), _SE(NetDoctor_set_timeout_milliseconds));

    cls->defineProperty("addr_port", _SE(NetDoctor_get_addr_port), _SE(NetDoctor_set_addr_port));
    cls->defineProperty("addr_path", _SE(NetDoctor_get_addr_path), _SE(NetDoctor_set_addr_path));

    cls->install();

    JSBClassType::registerClass<NetDoctor>(cls);

    __jsb_NetDoctor_class = cls;

    // 注册静态成员变量和静态成员函数(for test)
    // se::Value ctorVal;
    // if (global->getProperty("NetDoctor", &ctorVal) && ctorVal.isObject())
    // {
    //     ctorVal.toObject()->defineFunction("static_fun", _SE(NetDoctor_static_fun));
    // }

    se::ScriptEngine::getInstance()->clearException();

    return true;
}
#endif // #if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID || CC_TARGET_PLATFORM == CC_PLATFORM_IOS || CC_TARGET_PLATFORM == CC_PLATFORM_MAC || CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
