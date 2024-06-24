/****************************************************************************
 Copyright (c) 2015-2016 Chukong Technologies Inc.
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#include "network/CCDownloader-curl.h"

#include <set>
#include <curl/curl.h>
#include <deque>

#include "base/CCScheduler.h"
#include "platform/CCFileUtils.h"
#include "platform/CCApplication.h"
#include "network/CCDownloader.h"


#ifndef CC_CURL_POLL_TIMEOUT_MS
#define CC_CURL_POLL_TIMEOUT_MS 50
#endif

namespace cocos2d { namespace network {
    using namespace std;


    class DownloadTaskCURL : public IDownloadTask
    {
        static int _sSerialId;

        static set<string> _sStoragePathSet;
    public:
        int serialId;

        DownloadTaskCURL()
        : serialId(_sSerialId++)
        , _fp(nullptr)
        {
            _initInternal();
            DLLOG("Construct DownloadTaskCURL %p", this);
        }

        virtual ~DownloadTaskCURL()
        {
            if (_tempFileName.length() && _sStoragePathSet.end() != _sStoragePathSet.find(_tempFileName))
            {
                DownloadTaskCURL::_sStoragePathSet.erase(_tempFileName);
            }
            if (_fp)
            {
                fclose(_fp);
                _fp = nullptr;
            }
            DLLOG("Destruct DownloadTaskCURL %p", this);
        }

        bool init(const string& filename, const string& tempSuffix)
        {
            if (0 == filename.length())
            {
                _buf.reserve(CURL_MAX_WRITE_SIZE);
                return true;
            }

            _fileName = filename;
            _tempFileName = filename;
            _tempFileName.append(tempSuffix);

            if (_sStoragePathSet.end() != _sStoragePathSet.find(_tempFileName))
            {
                _errCode = DownloadTask::ERROR_FILE_OP_FAILED;
                _errCodeInternal = 0;
                _errDescription = "More than one download file task write to same file:";
                _errDescription.append(_tempFileName);
                return false;
            }
            _sStoragePathSet.insert(_tempFileName);

            bool ret = false;
            do
            {
                string dir;
                size_t found = _tempFileName.find_last_of("/\\");
                if (found == string::npos)
                {
                    _errCode = DownloadTask::ERROR_INVALID_PARAMS;
                    _errCodeInternal = 0;
                    _errDescription = "Can't find dirname in storagePath.";
                    break;
                }

                auto util = FileUtils::getInstance();
                dir = _tempFileName.substr(0, found+1);
                if (false == util->isDirectoryExist(dir))
                {
                    if (false == util->createDirectory(dir))
                    {
                        _errCode = DownloadTask::ERROR_FILE_OP_FAILED;
                        _errCodeInternal = 0;
                        _errDescription = "Can't create dir:";
                        _errDescription.append(dir);
                        break;
                    }
                }

                _fp = fopen(util->getSuitableFOpen(_tempFileName).c_str(), "ab");
                if (nullptr == _fp)
                {
                    _errCode = DownloadTask::ERROR_FILE_OP_FAILED;
                    _errCodeInternal = 0;
                    _errDescription = "Can't open file:";
                    _errDescription.append(_tempFileName);
                }
                ret = true;
            } while (0);

            return ret;
        }

        void initProc()
        {
            lock_guard<mutex> lock(_mutex);
            _initInternal();
        }

        void setErrorProc(int code, int codeInternal, const char *desc)
        {
            lock_guard<mutex> lock(_mutex);
            _errCode = code;
            _errCodeInternal = codeInternal;
            _errDescription = desc;
        }

        size_t writeDataProc(unsigned char *buffer, size_t size, size_t count)
        {
            lock_guard<mutex> lock(_mutex);
            size_t ret = 0;
            if (_fp)
            {
                ret = fwrite(buffer, size, count, _fp);
            }
            else
            {
                ret = size * count;
                auto cap = _buf.capacity();
                auto bufSize = _buf.size();
                if (cap < bufSize + ret)
                {
                    _buf.reserve(bufSize * 2);
                }
                _buf.insert(_buf.end() , buffer, buffer + ret);
            }
            if (ret)
            {
                _bytesReceived += ret;
                _totalBytesReceived += ret;
            }
            return ret;
        }

    private:
        friend class DownloaderCURL;

        mutex _mutex;

        bool    _acceptRanges;
        bool    _headerAchieved;
        int64_t _totalBytesExpected;

        string  _header;        // temp buffer for receive header string, only used in thread proc

        int64_t _bytesReceived;
        int64_t _totalBytesReceived;

        int     _errCode;
        int     _errCodeInternal;
        string  _errDescription;

        string _fileName;
        string _tempFileName;
        vector<unsigned char> _buf;
        FILE*  _fp;

        void _initInternal()
        {
            _acceptRanges = (false);
            _headerAchieved = (false);
            _bytesReceived = (0);
            _totalBytesReceived = (0);
            _totalBytesExpected = (0);
            _errCode = (DownloadTask::ERROR_NO_ERROR);
            _errCodeInternal = (CURLE_OK);
            _header.resize(0);
            _header.reserve(384);   // pre alloc header string buffer
        }
    };
    int DownloadTaskCURL::_sSerialId;
    set<string> DownloadTaskCURL::_sStoragePathSet;

    typedef pair< shared_ptr<const DownloadTask>, DownloadTaskCURL *> TaskWrapper;

    class DownloaderCURL::Impl : public enable_shared_from_this<DownloaderCURL::Impl>
    {
    public:
        DownloaderHints hints;

        Impl()
        {
            DLLOG("Construct DownloaderCURL::Impl %p", this);
        }

        ~Impl()
        {
            DLLOG("Destruct DownloaderCURL::Impl %p %d", this, _thread.joinable());
        }

        void addTask(std::shared_ptr<const DownloadTask> task, DownloadTaskCURL* coTask)
        {
            if (DownloadTask::ERROR_NO_ERROR == coTask->_errCode)
            {
                lock_guard<mutex> lock(_requestMutex);
                _requestQueue.push_back(make_pair(task, coTask));
            }
            else
            {
                lock_guard<mutex> lock(_finishedMutex);
                _finishedQueue.push_back(make_pair(task, coTask));
            }
        }

        void run()
        {
            lock_guard<mutex> lock(_threadMutex);
            if (false == _thread.joinable())
            {
                thread newThread(&DownloaderCURL::Impl::_threadProc, this);
                _thread.swap(newThread);
            }
        }

        void stop()
        {
            lock_guard<mutex> lock(_threadMutex);
            if (_thread.joinable())
            {
                _thread.detach();
            }
        }

        bool stoped()
        {
            lock_guard<mutex> lock(_threadMutex);
            return false == _thread.joinable() ? true : false;
        }

        void getProcessTasks(vector<TaskWrapper>& outList)
        {
            lock_guard<mutex> lock(_processMutex);
            outList.reserve(_processSet.size());
            outList.insert(outList.end(), _processSet.begin(), _processSet.end());
        }

        void getFinishedTasks(vector<TaskWrapper>& outList)
        {
            lock_guard<mutex> lock(_finishedMutex);
            outList.reserve(_finishedQueue.size());
            outList.insert(outList.end(), _finishedQueue.begin(), _finishedQueue.end());
            _finishedQueue.clear();
        }

    private:
        static size_t _outputHeaderCallbackProc(void *buffer, size_t size, size_t count, void *userdata)
        {
            int strLen = int(size * count);
            DLLOG("    _outputHeaderCallbackProc: %.*s", strLen, buffer);
            DownloadTaskCURL& coTask = *((DownloadTaskCURL*)(userdata));
            coTask._header.append((const char *)buffer, strLen);
            return strLen;
        }

        static size_t _outputDataCallbackProc(void *buffer, size_t size, size_t count, void *userdata)
        {
            DownloadTaskCURL *coTask = (DownloadTaskCURL*)userdata;

            return coTask->writeDataProc((unsigned char *)buffer, size, count);
        }

        void _initCurlHandleProc(CURL *handle, TaskWrapper& wrapper, bool forContent = false)
        {
            const DownloadTask& task = *wrapper.first;
            const DownloadTaskCURL* coTask = wrapper.second;

            curl_easy_setopt(handle, CURLOPT_URL, task.requestURL.c_str());

            if (forContent)
            {
                curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, DownloaderCURL::Impl::_outputDataCallbackProc);
            }
            else
            {
                curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, DownloaderCURL::Impl::_outputHeaderCallbackProc);
            }
            curl_easy_setopt(handle, CURLOPT_WRITEDATA, coTask);

            curl_easy_setopt(handle, CURLOPT_NOPROGRESS, true);

            curl_easy_setopt(handle, CURLOPT_FAILONERROR, true);
            curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);

            if (forContent)
            {
                /** if server acceptRanges and local has part of file, we continue to download **/
                if (coTask->_acceptRanges && coTask->_totalBytesReceived > 0)
                {
                    curl_easy_setopt(handle, CURLOPT_RESUME_FROM_LARGE,(curl_off_t)coTask->_totalBytesReceived);
                }
            }
            else
            {
                curl_easy_setopt(handle, CURLOPT_HEADER, 1);
                curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
            }

            if (hints.timeoutInSeconds)
            {
                curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, hints.timeoutInSeconds);
            }

            static const long LOW_SPEED_LIMIT = 1;
            static const long LOW_SPEED_TIME = 10;
            curl_easy_setopt(handle, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
            curl_easy_setopt(handle, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);

            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, false);
            curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, false);

            static const int MAX_REDIRS = 5;
            if (MAX_REDIRS)
            {
                curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, true);
                curl_easy_setopt(handle, CURLOPT_MAXREDIRS, MAX_REDIRS);
            }
        }

        bool _getHeaderInfoProc(CURL *handle, TaskWrapper& wrapper)
        {
            DownloadTaskCURL& coTask = *wrapper.second;
            CURLcode rc = CURLE_OK;
            do
            {
                long httpResponseCode = 0;
                rc = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpResponseCode);
                if (CURLE_OK != rc)
                {
                    break;
                }
                if (200 != httpResponseCode)
                {
                    char buf[256] = {0};
                    sprintf(buf
                            , "When request url(%s) header info, return unexcept http response code(%ld)"
                            , wrapper.first->requestURL.c_str()
                            , httpResponseCode);
                    coTask.setErrorProc(DownloadTask::ERROR_IMPL_INTERNAL, CURLE_OK, buf);
                }

                double contentLen = 0;
                rc = curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLen);
                if (CURLE_OK != rc)
                {
                    break;
                }

                bool acceptRanges = (string::npos != coTask._header.find("Accept-Ranges")) ? true : false;

                int64_t fileSize = 0;
                if (acceptRanges && coTask._tempFileName.length())
                {
                    fileSize = FileUtils::getInstance()->getFileSize(coTask._tempFileName);
                }

                lock_guard<mutex> lock(coTask._mutex);
                coTask._totalBytesExpected = (int64_t)contentLen;
                coTask._acceptRanges = acceptRanges;
                if (acceptRanges && fileSize > 0)
                {
                    coTask._totalBytesReceived = fileSize;
                }
                coTask._headerAchieved = true;
            } while (0);

            if (CURLE_OK != rc)
            {
                coTask.setErrorProc(DownloadTask::ERROR_IMPL_INTERNAL, rc, curl_easy_strerror(rc));
            }
            return coTask._headerAchieved;
        }

        void _threadProc()
        {
            DLLOG("++++DownloaderCURL::Impl::_threadProc begin %p", this);
            auto holder = this->shared_from_this();
            auto thisThreadId = this_thread::get_id();
            uint32_t countOfMaxProcessingTasks = this->hints.countOfMaxProcessingTasks;
            CURLM* curlmHandle = curl_multi_init();
            unordered_map<CURL*, TaskWrapper> coTaskMap;
            int runningHandles = 0;
            CURLMcode mcode = CURLM_OK;
            int rc = 0;                 // select return code

            do
            {
                {
                    lock_guard<mutex> lock(_threadMutex);
                    if (thisThreadId != this->_thread.get_id())
                    {
                        break;
                    }
                }

                if (runningHandles)
                {
                    long timeoutMS = -1;
                    curl_multi_timeout(curlmHandle, &timeoutMS);

                    if(timeoutMS < 0)
                    {
                        timeoutMS = 1000;
                    }

                    /* get file descriptors from the transfers */
                    fd_set fdread;
                    fd_set fdwrite;
                    fd_set fdexcep;
                    int maxfd = -1;

                    FD_ZERO(&fdread);
                    FD_ZERO(&fdwrite);
                    FD_ZERO(&fdexcep);

                    mcode = curl_multi_fdset(curlmHandle, &fdread, &fdwrite, &fdexcep, &maxfd);
                    if (CURLM_OK != mcode)
                    {
                        break;
                    }

                    if(maxfd == -1)
                    {
                        this_thread::sleep_for(chrono::milliseconds(CC_CURL_POLL_TIMEOUT_MS));
                        rc = 0;
                    }
                    else
                    {
                        struct timeval timeout;

                        timeout.tv_sec = timeoutMS / 1000;
                        timeout.tv_usec = (timeoutMS % 1000) * 1000;

                        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
                    }

                    if (rc < 0)
                    {
                        DLLOG("    _threadProc: select return unexpect code: %d", rc);
                    }
                }

                if (coTaskMap.size())
                {
                    mcode = CURLM_CALL_MULTI_PERFORM;
                    while(CURLM_CALL_MULTI_PERFORM == mcode)
                    {
                        mcode = curl_multi_perform(curlmHandle, &runningHandles);
                    }
                    if (CURLM_OK != mcode)
                    {
                        break;
                    }

                    struct CURLMsg *m;
                    do {
                        int msgq = 0;
                        m = curl_multi_info_read(curlmHandle, &msgq);
                        if(m && (m->msg == CURLMSG_DONE))
                        {
                            CURL *curlHandle = m->easy_handle;
                            CURLcode errCode = m->data.result;

                            TaskWrapper wrapper = coTaskMap[curlHandle];

                            curl_multi_remove_handle(curlmHandle, curlHandle);
                            bool reinited = false;
                            do
                            {
                                if (CURLE_OK != errCode)
                                {
                                    wrapper.second->setErrorProc(DownloadTask::ERROR_IMPL_INTERNAL, errCode, curl_easy_strerror(errCode));
                                    break;
                                }

                                if (wrapper.second->_headerAchieved)
                                {
                                    break;
                                }

                                if (false == _getHeaderInfoProc(curlHandle, wrapper))
                                {
                                    break;
                                }

                                if (wrapper.second->_totalBytesReceived &&
                                    wrapper.second->_totalBytesReceived == wrapper.second->_totalBytesExpected)
                                {
                                    break;
                                }
                                curl_easy_reset(curlHandle);
                                _initCurlHandleProc(curlHandle, wrapper, true);
                                mcode = curl_multi_add_handle(curlmHandle, curlHandle);
                                if (CURLM_OK != mcode)
                                {
                                    wrapper.second->setErrorProc(DownloadTask::ERROR_IMPL_INTERNAL, mcode, curl_multi_strerror(mcode));
                                    break;
                                }
                                reinited = true;
                            } while (0);

                            if (reinited)
                            {
                                continue;
                            }
                            curl_easy_cleanup(curlHandle);
                            DLLOG("    _threadProc task clean cur handle :%p with errCode:%d",  curlHandle, errCode);

                            coTaskMap.erase(curlHandle);

                            {
                                lock_guard<mutex> lock(_processMutex);
                                if (_processSet.end() != _processSet.find(wrapper)) {
                                    _processSet.erase(wrapper);
                                }
                            }

                            {
                                lock_guard<mutex> lock(_finishedMutex);
                                _finishedQueue.push_back(wrapper);
                            }
                        }
                    } while(m);
                }

                auto size = coTaskMap.size();
                while (0 == countOfMaxProcessingTasks || size < countOfMaxProcessingTasks)
                {
                    TaskWrapper wrapper;
                    {
                        lock_guard<mutex> lock(_requestMutex);
                        if (_requestQueue.size())
                        {
                            wrapper = _requestQueue.front();
                            _requestQueue.pop_front();
                        }
                    }

                    if (! wrapper.first)
                    {
                        break;
                    }

                    wrapper.second->initProc();

                    CURL* curlHandle = curl_easy_init();

                    if (nullptr == curlHandle)
                    {
                        wrapper.second->setErrorProc(DownloadTask::ERROR_IMPL_INTERNAL, 0, "Alloc curl handle failed.");
                        lock_guard<mutex> lock(_finishedMutex);
                        _finishedQueue.push_back(wrapper);
                        continue;
                    }

                    _initCurlHandleProc(curlHandle, wrapper);

                    mcode = curl_multi_add_handle(curlmHandle, curlHandle);
                    if (CURLM_OK != mcode)
                    {
                        wrapper.second->setErrorProc(DownloadTask::ERROR_IMPL_INTERNAL, mcode, curl_multi_strerror(mcode));
                        lock_guard<mutex> lock(_finishedMutex);
                        _finishedQueue.push_back(wrapper);
                        continue;
                    }

                    DLLOG("    _threadProc task create curl handle:%p", curlHandle);
                    coTaskMap[curlHandle] = wrapper;
                    lock_guard<mutex> lock(_processMutex);
                    _processSet.insert(wrapper);
                }
            } while (coTaskMap.size());

            curl_multi_cleanup(curlmHandle);
            this->stop();
            DLLOG("----DownloaderCURL::Impl::_threadProc end");
        }

        thread _thread;
        deque<TaskWrapper>  _requestQueue;
        set<TaskWrapper>    _processSet;
        deque<TaskWrapper>  _finishedQueue;

        mutex _threadMutex;
        mutex _requestMutex;
        mutex _processMutex;
        mutex _finishedMutex;
    };


    DownloaderCURL::DownloaderCURL(const DownloaderHints& hints)
    : _impl(std::make_shared<Impl>())
    , _currTask(nullptr)
    {
        DLLOG("Construct DownloaderCURL %p", this);
        _impl->hints = hints;
        _scheduler = Application::getInstance()->getScheduler();

        _transferDataToBuffer = [this](void *buf, int64_t len)->int64_t
        {
            DownloadTaskCURL& coTask = *_currTask;
            int64_t dataLen = coTask._buf.size();
            if (len < dataLen)
            {
                return 0;
            }

            memcpy(buf, coTask._buf.data(), dataLen);
            coTask._buf.resize(0);
            return dataLen;
        };

        char key[128];
        sprintf(key, "DownloaderCURL(%p)", this);
        _schedulerKey = key;

        if(auto sche = _scheduler.lock())
        {
            sche->schedule(bind(&DownloaderCURL::_onSchedule, this, placeholders::_1),
                             this,
                             0.1f,
                             true,
                             _schedulerKey);
        }
    }

    DownloaderCURL::~DownloaderCURL()
    {
        if(auto sche = _scheduler.lock())
        {
            sche->unschedule(_schedulerKey, this);
        }

        _impl->stop();
        DLLOG("Destruct DownloaderCURL %p", this);
    }

    IDownloadTask *DownloaderCURL::createCoTask(std::shared_ptr<const DownloadTask>& task)
    {
        DownloadTaskCURL *coTask = new (std::nothrow) DownloadTaskCURL;
        coTask->init(task->storagePath, _impl->hints.tempFileNameSuffix);

        DLLOG("    DownloaderCURL: createTask: Id(%d)", coTask->serialId);

        _impl->addTask(task, coTask);
        _impl->run();
        
        if(auto sche = _scheduler.lock())
        {
            sche->resumeTarget(this);
        }
        return coTask;
    }

    void DownloaderCURL::abort(const std::unique_ptr<IDownloadTask>& task) {
        DLLOG("%s isn't implemented!\n", __FUNCTION__);
    }

    void DownloaderCURL::_onSchedule(float)
    {
        vector<TaskWrapper> tasks;

        _impl->getProcessTasks(tasks);
        for (auto& wrapper : tasks)
        {
            const DownloadTask& task = *wrapper.first;
            DownloadTaskCURL& coTask = *wrapper.second;

            lock_guard<mutex> lock(coTask._mutex);
            if (coTask._bytesReceived)
            {
                _currTask = &coTask;
                onTaskProgress(task,
                               coTask._bytesReceived,
                               coTask._totalBytesReceived,
                               coTask._totalBytesExpected,
                               _transferDataToBuffer);
                _currTask = nullptr;
                coTask._bytesReceived = 0;
            }
        }
        tasks.resize(0);

        _impl->getFinishedTasks(tasks);
        if (_impl->stoped())
        {
            if (auto sche = _scheduler.lock())
            {
                sche->pauseTarget(this);
            }
        }

        for (auto& wrapper : tasks)
        {
            const DownloadTask& task = *wrapper.first;
            DownloadTaskCURL& coTask = *wrapper.second;

            if (coTask._bytesReceived)
            {
                _currTask = &coTask;
                onTaskProgress(task,
                               coTask._bytesReceived,
                               coTask._totalBytesReceived,
                               coTask._totalBytesExpected,
                               _transferDataToBuffer);
                coTask._bytesReceived = 0;
                _currTask = nullptr;
            }

            if (coTask._fp)
            {
                fclose(coTask._fp);
                coTask._fp = nullptr;
                do
                {
                    if (0 == coTask._fileName.length())
                    {
                        break;
                    }

                    auto util = FileUtils::getInstance();
                    if (util->isFileExist(coTask._fileName))
                    {
                        if (false == util->removeFile(coTask._fileName))
                        {
                            coTask._errCode = DownloadTask::ERROR_FILE_OP_FAILED;
                            coTask._errCodeInternal = 0;
                            coTask._errDescription = "Can't remove old file: ";
                            coTask._errDescription.append(coTask._fileName);
                            break;
                        }
                    }

                    if (util->renameFile(coTask._tempFileName, coTask._fileName))
                    {
                        DownloadTaskCURL::_sStoragePathSet.erase(coTask._tempFileName);
                        break;
                    }
                    coTask._errCode = DownloadTask::ERROR_FILE_OP_FAILED;
                    coTask._errCodeInternal = 0;
                    coTask._errDescription = "Can't renamefile from: ";
                    coTask._errDescription.append(coTask._tempFileName);
                    coTask._errDescription.append(" to: ");
                    coTask._errDescription.append(coTask._fileName);
                } while (0);

            }
            onTaskFinish(task, coTask._errCode, coTask._errCodeInternal, coTask._errDescription, coTask._buf);
            DLLOG("    DownloaderCURL: finish Task: Id(%d)", coTask.serialId);
        }
    }

}}  //  namespace cocos2d::network

