#pragma once
#include "base/ccMacros.h"
#include "base/CCRef.h"

#include <vector>
#include <string>
#include <atomic>
#include <mutex>
#include <map>
#include <time.h>

using namespace cocos2d;

NS_CC_BEGIN
namespace network
{

    class CC_DLL NetDoctor : public Ref
    {
    public:
        struct Domain
        {
            std::string domain;
            long latency;
        };
        class NetDoctorCallback : public Ref
        {
        public:
            NetDoctorCallback();
            virtual ~NetDoctorCallback();

            void onSorted(NetDoctor *netdoctor, std::map<std::string, std::vector<Domain>> sortedDomains);
            void setJSDelegate(const se::Value &jsDelegate);

        private:
            se::Value _JSDelegate;
        };

    public:
        NetDoctor();
        NetDoctor(std::map<std::string, std::vector<std::string>> &domains);
        virtual ~NetDoctor();
        long interval_milliseconds;
        long timeout_milliseconds;
        int addr_port;
        std::string addr_path;

        void update(std::map<std::string, std::vector<std::string>> &domains);
        void start();
        void stop();
        void setCallback(NetDoctorCallback &callback);
        std::vector<Domain> getSortedDomains(std::string key);
        std::map<std::string, std::vector<Domain>> getAllSortedDomains();

    private:
        void diagnose();
        std::map<std::string, std::vector<Domain>> _sortedDomains;
        std::map<std::string, std::vector<std::string>> _domains;

        bool _isThreadRunning;
        std::atomic_bool _threadStopFlag;
        std::mutex _domainsMutex;
        NetDoctorCallback _callback;
        std::shared_ptr<std::atomic<bool>> _isDestroyed;
        struct timeval _last_time;
        long _total_count;
    };

}
NS_CC_END