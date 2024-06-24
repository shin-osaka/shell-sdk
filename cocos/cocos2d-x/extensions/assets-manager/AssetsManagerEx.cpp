/****************************************************************************
 Copyright (c) 2014 cocos2d-x.org
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
#include "AssetsManagerEx.h"
#include "base/ccUTF8.h"
#include "CCAsyncTaskPool.h"

#include <stdio.h>
#include <errno.h>

#ifdef MINIZIP_FROM_SYSTEM
#include <minizip/unzip.h>
#else // from our embedded sources
#include "unzip/unzip.h"
#endif

NS_CC_EXT_BEGIN

#define VERSION_FILENAME "config.u3d"
#define TEMP_MANIFEST_FILENAME "resources.u3d.temp"
#define TEMP_PACKAGE_SUFFIX "_temp"
#define MANIFEST_FILENAME "resources.u3d"

#define BUFFER_SIZE 8192
#define MAX_FILENAME 512

#define DEFAULT_CONNECTION_TIMEOUT 45

#define SAVE_POINT_INTERVAL 0.1

const std::string AssetsManagerEx::VERSION_ID = "@version";
const std::string AssetsManagerEx::MANIFEST_ID = "@manifest";

void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr)
{
    size_t pos = data.find(toSearch);

    while (pos != std::string::npos)
    {
        data.replace(pos, toSearch.size(), replaceStr);
        pos = data.find(toSearch, pos + toSearch.size());
    }
}

AssetsManagerEx::AssetsManagerEx(const std::string &manifestUrl,
                                 const std::string &storagePath,
                                 const std::string &packageUrl /* = ""*/,
                                 const std::string &tailVersion /* = ""*/)
    : _updateState(State::UNINITED), _assets(nullptr), _storagePath(""), _tempVersionPath(""), _cacheManifestPath(""), _tempManifestPath(""), _localManifest(nullptr), _tempManifest(nullptr), _remoteManifest(nullptr), _updateEntry(UpdateEntry::NONE), _percent(0), _percentByFile(0), _totalSize(0), _sizeCollected(0), _totalDownloaded(0), _totalToDownload(0), _totalWaitToDownload(0), _nextSavePoint(0.0), _downloadResumed(false), _maxConcurrentTask(32), _currConcurrentTask(0), _verifyCallback(nullptr), _inited(false), _packageUrl(packageUrl), _tailVersion(tailVersion)
{
    if (_packageUrl.size() > 0 && _packageUrl[_packageUrl.size() - 1] != '/')
    {
        _packageUrl.append("/");
    }
    init(manifestUrl, storagePath);
}

AssetsManagerEx::AssetsManagerEx(const std::string &manifestUrl,
                                 const std::string &storagePath,
                                 const VersionCompareHandle &handle,
                                 const std::string &packageUrl /* = ""*/,
                                 const std::string &tailVersion /* = ""*/)
    : _updateState(State::UNINITED), _assets(nullptr), _storagePath(""), _tempVersionPath(""), _cacheManifestPath(""), _tempManifestPath(""), _localManifest(nullptr), _tempManifest(nullptr), _remoteManifest(nullptr), _updateEntry(UpdateEntry::NONE), _percent(0), _percentByFile(0), _totalSize(0), _sizeCollected(0), _totalDownloaded(0), _totalToDownload(0), _totalWaitToDownload(0), _nextSavePoint(0.0), _downloadResumed(false), _maxConcurrentTask(32), _currConcurrentTask(0), _versionCompareHandle(handle), _verifyCallback(nullptr), _eventCallback(nullptr), _inited(false), _packageUrl(packageUrl), _tailVersion(tailVersion)
{
    cocos2d::log("_packageUrl=%s, manifestUrl=%s, storagePath=%s", _packageUrl.c_str(), manifestUrl.c_str(), storagePath.c_str());
    if (_packageUrl.size() > 0 && _packageUrl[_packageUrl.size() - 1] != '/')
    {
        _packageUrl.append("/");
    }

    init(manifestUrl, storagePath);
}

void AssetsManagerEx::init(const std::string &manifestUrl, const std::string &storagePath)
{
    std::string pointer = StringUtils::format("%p", this);
    _eventName = "__cc_assets_manager_" + pointer;
    _fileUtils = FileUtils::getInstance();

    network::DownloaderHints hints =
        {
            static_cast<uint32_t>(_maxConcurrentTask),
            DEFAULT_CONNECTION_TIMEOUT,
            ".tmp"};
    _downloader = std::shared_ptr<network::Downloader>(new network::Downloader(hints));
    _downloader->onTaskError = std::bind(&AssetsManagerEx::onError, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    _downloader->onTaskProgress = [this](const network::DownloadTask &task,
                                         int64_t /*bytesReceived*/,
                                         int64_t totalBytesReceived,
                                         int64_t totalBytesExpected)
    {
        this->onProgress(totalBytesExpected, totalBytesReceived, task.requestURL, task.identifier);
    };
    _downloader->onFileTaskSuccess = [this](const network::DownloadTask &task)
    {
        this->onSuccess(task.requestURL, task.storagePath, task.identifier);
    };
    setStoragePath(storagePath);
    _tempVersionPath = _tempStoragePath + VERSION_FILENAME;
    _cacheManifestPath = _storagePath + MANIFEST_FILENAME;
    _tempManifestPath = _tempStoragePath + TEMP_MANIFEST_FILENAME;

    if (manifestUrl.size() > 0)
    {
        loadLocalManifest(manifestUrl);
    }
}

AssetsManagerEx::~AssetsManagerEx()
{
    _downloader->onTaskError = (nullptr);
    _downloader->onFileTaskSuccess = (nullptr);
    _downloader->onTaskProgress = (nullptr);
    CC_SAFE_RELEASE(_localManifest);
    if (_tempManifest != _localManifest && _tempManifest != _remoteManifest)
        CC_SAFE_RELEASE(_tempManifest);
    CC_SAFE_RELEASE(_remoteManifest);
}

AssetsManagerEx *AssetsManagerEx::create(const std::string &manifestUrl,
                                         const std::string &storagePath,
                                         const std::string &packageUrl /*=""*/,
                                         const std::string &strTail /* = ""*/)
{
    AssetsManagerEx *ret = new (std::nothrow) AssetsManagerEx(manifestUrl, storagePath, packageUrl, strTail);
    if (ret)
    {
        ret->autorelease();
    }
    else
    {
        CC_SAFE_DELETE(ret);
    }
    return ret;
}

void AssetsManagerEx::initManifests()
{
    _inited = true;
    _tempManifest = new (std::nothrow) Manifest();
    if (_tempManifest)
    {
        _tempManifest->parseFile(_tempManifestPath);
        _tempManifest->setPackageUrl(_packageUrl);
        if (_fileUtils->isFileExist(_tempManifestPath))
        {
            if (!_tempManifest->isLoaded())
            {
                _fileUtils->removeDirectory(_tempStoragePath);
                CC_SAFE_RELEASE(_tempManifest);
                _tempManifest = nullptr;
            }
        }
    }
    else
    {
        _inited = false;
    }

    _remoteManifest = new (std::nothrow) Manifest();
    if (!_remoteManifest)
    {
        _inited = false;
    }

    if (!_inited)
    {
        CC_SAFE_RELEASE(_localManifest);
        CC_SAFE_RELEASE(_tempManifest);
        CC_SAFE_RELEASE(_remoteManifest);
        _localManifest = nullptr;
        _tempManifest = nullptr;
        _remoteManifest = nullptr;
    }
}

void AssetsManagerEx::prepareLocalManifest()
{
    _assets = &(_localManifest->getAssets());

    _localManifest->prependSearchPaths();
}

bool AssetsManagerEx::loadLocalManifest(Manifest *localManifest, const std::string &storagePath)
{
    if (_updateState > State::UNINITED)
    {
        return false;
    }
    if (!localManifest || !localManifest->isLoaded())
    {
        return false;
    }
    _inited = true;
    if (storagePath.size() > 0)
    {
        setStoragePath(storagePath);
        _tempVersionPath = _tempStoragePath + VERSION_FILENAME;
        _cacheManifestPath = _storagePath + MANIFEST_FILENAME;
        _tempManifestPath = _tempStoragePath + TEMP_MANIFEST_FILENAME;
    }
    if (_localManifest)
    {
        CC_SAFE_RELEASE(_localManifest);
    }
    _localManifest = localManifest;
    _localManifest->retain();
    Manifest *cachedManifest = nullptr;
    if (_fileUtils->isFileExist(_cacheManifestPath))
    {
        cachedManifest = new (std::nothrow) Manifest();
        if (cachedManifest)
        {
            cachedManifest->parseFile(_cacheManifestPath);
            if (!cachedManifest->isLoaded())
            {
                _fileUtils->removeFile(_cacheManifestPath);
                CC_SAFE_RELEASE(cachedManifest);
                cachedManifest = nullptr;
            }
        }
    }
    if (cachedManifest)
    {
        bool localNewer = _localManifest->versionGreater(cachedManifest, _versionCompareHandle);
        if (localNewer)
        {
            _fileUtils->removeDirectory(_storagePath);
            _fileUtils->createDirectory(_storagePath);
            CC_SAFE_RELEASE(cachedManifest);
        }
        else
        {
            CC_SAFE_RELEASE(_localManifest);
            _localManifest = cachedManifest;
            if (_packageUrl != "")
            {
                _localManifest->setPackageUrl(_packageUrl);
                _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
                std::string _verManifestUrl = _manifestUrl;
                findAndReplaceAll(_verManifestUrl, "resources", "config");

                _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
            }
        }
    }
    prepareLocalManifest();

    initManifests();

    if (!_inited)
    {
        return false;
    }
    else
    {
        _updateState = State::UNCHECKED;
        return true;
    }
}

bool AssetsManagerEx::loadLocalManifest(const std::string &manifestUrl)
{
    if (manifestUrl.size() == 0)
    {
        return false;
    }
    if (_updateState > State::UNINITED)
    {
        return false;
    }
    _manifestUrl = manifestUrl;
    _localManifest = new (std::nothrow) Manifest();
    if (!_localManifest)
    {
        return false;
    }
    Manifest *cachedManifest = nullptr;
    if (_fileUtils->isFileExist(_cacheManifestPath))
    {
        cachedManifest = new (std::nothrow) Manifest();
        if (cachedManifest)
        {
            if (_packageUrl != "")
            {
                cachedManifest->setPackageUrl(_packageUrl);
                _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
                std::string _verManifestUrl = _manifestUrl;
                findAndReplaceAll(_verManifestUrl, "resources", "config");
                _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
            }
            cachedManifest->parseFile(_cacheManifestPath);
            if (!cachedManifest->isLoaded())
            {
                _fileUtils->removeFile(_cacheManifestPath);
                CC_SAFE_RELEASE(cachedManifest);
                cachedManifest = nullptr;
            }
        }
    }

    std::vector<std::string> searchPaths = _fileUtils->getSearchPaths();
    if (cachedManifest)
    {
        std::vector<std::string> cacheSearchPaths = cachedManifest->getSearchPaths();
        std::vector<std::string> trimmedPaths = searchPaths;
        for (auto path : cacheSearchPaths)
        {
            const auto pos = std::find(trimmedPaths.begin(), trimmedPaths.end(), path);
            if (pos != trimmedPaths.end())
            {
                trimmedPaths.erase(pos);
            }
        }
    }
    _localManifest->parseFile(_manifestUrl);
    if (_packageUrl != "")
    {
        _localManifest->setPackageUrl(_packageUrl);
        _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
        std::string _verManifestUrl = _manifestUrl;
        findAndReplaceAll(_verManifestUrl, "resources", "config");
        _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
    }
    if (cachedManifest)
    {
    }
    if (_localManifest->isLoaded())
    {
        if (cachedManifest)
        {
            bool localNewer = _localManifest->versionGreater(cachedManifest, _versionCompareHandle);
            if (localNewer)
            {
                _fileUtils->removeDirectory(_storagePath);
                _fileUtils->createDirectory(_storagePath);
                CC_SAFE_RELEASE(cachedManifest);
            }
            else
            {
                CC_SAFE_RELEASE(_localManifest);
                _localManifest = cachedManifest;
                if (_packageUrl != "")
                {
                    _localManifest->setPackageUrl(_packageUrl);
                    _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
                    std::string _verManifestUrl = _manifestUrl;
                    findAndReplaceAll(_verManifestUrl, "resources", "config");
                    _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
                }
            }
        }
        prepareLocalManifest();
    }

    if (!_localManifest->isLoaded())
    {
        CCLOG("AssetsManagerEx : No local manifest file found error.\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST);
        return false;
    }
    initManifests();
    _updateState = State::UNCHECKED;
    return true;
}

bool AssetsManagerEx::loadRemoteManifest(Manifest *remoteManifest)
{
    if (!_inited || _updateState > State::UNCHECKED)
    {
        return false;
    }
    if (!remoteManifest || !remoteManifest->isLoaded())
    {
        return false;
    }
    if (_remoteManifest)
    {
        CC_SAFE_RELEASE(_remoteManifest);
    }
    _remoteManifest = remoteManifest;
    if (_packageUrl != "")
    {
        _remoteManifest->setPackageUrl(_packageUrl);
        _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
        std::string _verManifestUrl = _manifestUrl;
        findAndReplaceAll(_verManifestUrl, "resources", "config");
        _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
    }
    _remoteManifest->retain();
    if (_localManifest->versionGreaterOrEquals(_remoteManifest, _versionCompareHandle))
    {
        _updateState = State::UP_TO_DATE;
        _fileUtils->removeDirectory(_tempStoragePath);
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE);
    }
    else
    {
        _updateState = State::NEED_UPDATE;
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND);
    }
    return true;
}

std::string AssetsManagerEx::basename(const std::string &path) const
{
    size_t found = path.find_last_of("/\\");

    if (std::string::npos != found)
    {
        return path.substr(0, found);
    }
    else
    {
        return path;
    }
}

std::string AssetsManagerEx::get(const std::string &key) const
{
    auto it = _assets->find(key);
    if (it != _assets->cend())
    {
        return _storagePath + it->second.path;
    }
    else
        return "";
}

const Manifest *AssetsManagerEx::getLocalManifest() const
{
    return _localManifest;
}

const Manifest *AssetsManagerEx::getRemoteManifest() const
{
    return _remoteManifest;
}

const std::string &AssetsManagerEx::getStoragePath() const
{
    return _storagePath;
}

void AssetsManagerEx::setStoragePath(const std::string &storagePath)
{
    _storagePath = storagePath;
    adjustPath(_storagePath);
    _fileUtils->createDirectory(_storagePath);

    _tempStoragePath = _storagePath;
    _tempStoragePath.insert(_storagePath.size() - 1, TEMP_PACKAGE_SUFFIX);
    _fileUtils->createDirectory(_tempStoragePath);
}

void AssetsManagerEx::adjustPath(std::string &path)
{
    if (path.size() > 0 && path[path.size() - 1] != '/')
    {
        path.append("/");
    }
}

bool AssetsManagerEx::decompress(const std::string &zip)
{
    size_t pos = zip.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        CCLOG("AssetsManagerEx : no root path specified for zip file %s\n", zip.c_str());
        return false;
    }
    const std::string rootPath = zip.substr(0, pos + 1);

    unzFile zipfile = unzOpen(FileUtils::getInstance()->getSuitableFOpen(zip).c_str());
    if (!zipfile)
    {
        CCLOG("AssetsManagerEx : can not open downloaded zip file %s\n", zip.c_str());
        return false;
    }

    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
    {
        CCLOG("AssetsManagerEx : can not read file global info of %s\n", zip.c_str());
        unzClose(zipfile);
        return false;
    }

    char readBuffer[BUFFER_SIZE];
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i)
    {
        unz_file_info fileInfo;
        char fileName[MAX_FILENAME];
        if (unzGetCurrentFileInfo(zipfile,
                                  &fileInfo,
                                  fileName,
                                  MAX_FILENAME,
                                  NULL,
                                  0,
                                  NULL,
                                  0) != UNZ_OK)
        {
            CCLOG("AssetsManagerEx : can not read compressed file info\n");
            unzClose(zipfile);
            return false;
        }
        const std::string fullPath = rootPath + fileName;

        const size_t filenameLength = strlen(fileName);
        if (fileName[filenameLength - 1] == '/')
        {
            if (!_fileUtils->createDirectory(basename(fullPath)))
            {
                CCLOG("AssetsManagerEx : can not create directory %s\n", fullPath.c_str());
                unzClose(zipfile);
                return false;
            }
        }
        else
        {
            std::string dir = basename(fullPath);
            if (!_fileUtils->isDirectoryExist(dir))
            {
                if (!_fileUtils->createDirectory(dir))
                {
                    CCLOG("AssetsManagerEx : can not create directory %s\n", fullPath.c_str());
                    unzClose(zipfile);
                    return false;
                }
            }
            if (unzOpenCurrentFile(zipfile) != UNZ_OK)
            {
                CCLOG("AssetsManagerEx : can not extract file %s\n", fileName);
                unzClose(zipfile);
                return false;
            }

            FILE *out = fopen(FileUtils::getInstance()->getSuitableFOpen(fullPath).c_str(), "wb");
            if (!out)
            {
                CCLOG("AssetsManagerEx : can not create decompress destination file %s (errno: %d)\n", fullPath.c_str(), errno);
                unzCloseCurrentFile(zipfile);
                unzClose(zipfile);
                return false;
            }

            int error = UNZ_OK;
            do
            {
                error = unzReadCurrentFile(zipfile, readBuffer, BUFFER_SIZE);
                if (error < 0)
                {
                    CCLOG("AssetsManagerEx : can not read zip file %s, error code is %d\n", fileName, error);
                    fclose(out);
                    unzCloseCurrentFile(zipfile);
                    unzClose(zipfile);
                    return false;
                }

                if (error > 0)
                {
                    fwrite(readBuffer, error, 1, out);
                }
            } while (error > 0);

            fclose(out);
        }

        unzCloseCurrentFile(zipfile);

        if ((i + 1) < global_info.number_entry)
        {
            if (unzGoToNextFile(zipfile) != UNZ_OK)
            {
                CCLOG("AssetsManagerEx : can not read next file for decompressing\n");
                unzClose(zipfile);
                return false;
            }
        }
    }

    unzClose(zipfile);
    return true;
}

void AssetsManagerEx::decompressDownloadedZip(const std::string &customId, const std::string &storagePath)
{
    struct AsyncData
    {
        std::string customId;
        std::string zipFile;
        bool succeed;
    };

    AsyncData *asyncData = new AsyncData;
    asyncData->customId = customId;
    asyncData->zipFile = storagePath;
    asyncData->succeed = false;

    std::function<void(void *)> decompressFinished = [this](void *param)
    {
        auto dataInner = reinterpret_cast<AsyncData *>(param);
        if (dataInner->succeed)
        {
            fileSuccess(dataInner->customId, dataInner->zipFile);
        }
        else
        {
            std::string errorMsg = "Unable to decompress file " + dataInner->zipFile;
            _fileUtils->removeFile(dataInner->zipFile);
            dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_DECOMPRESS, "", errorMsg);
            fileError(dataInner->customId, errorMsg);
        }
        delete dataInner;
    };
    AsyncTaskPool::getInstance()->enqueue(AsyncTaskPool::TaskType::TASK_OTHER, decompressFinished, (void *)asyncData, [this, asyncData]()
                                          {
        if (decompress(asyncData->zipFile))
        {
            asyncData->succeed = true;
        }
        _fileUtils->removeFile(asyncData->zipFile); });
}

void AssetsManagerEx::dispatchUpdateEvent(EventAssetsManagerEx::EventCode code, const std::string &assetId /* = ""*/, const std::string &message /* = ""*/, int curle_code /* = CURLE_OK*/, int curlm_code /* = CURLM_OK*/)
{
    switch (code)
    {
    case EventAssetsManagerEx::EventCode::ERROR_UPDATING:
    case EventAssetsManagerEx::EventCode::ERROR_PARSE_MANIFEST:
    case EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST:
    case EventAssetsManagerEx::EventCode::ERROR_DECOMPRESS:
    case EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST:
    case EventAssetsManagerEx::EventCode::UPDATE_FAILED:
    case EventAssetsManagerEx::EventCode::UPDATE_FINISHED:
    case EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE:
        _updateEntry = UpdateEntry::NONE;
        break;
    case EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION:
        break;
    case EventAssetsManagerEx::EventCode::ASSET_UPDATED:
        break;
    case EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND:
        if (_updateEntry == UpdateEntry::CHECK_UPDATE)
        {
            _updateEntry = UpdateEntry::NONE;
        }
        break;
    default:
        break;
    }

    if (_eventCallback != nullptr)
    {
        EventAssetsManagerEx *event = new (std::nothrow) EventAssetsManagerEx(_eventName, this, code, assetId, message, curle_code, curlm_code);
        _eventCallback(event);
        event->release();
    }
}

AssetsManagerEx::State AssetsManagerEx::getState() const
{
    return _updateState;
}

void AssetsManagerEx::setTailVersion(const std::string &tversion)
{
    _tailVersion = tversion;
}

const std::string &AssetsManagerEx::getTailVersion()
{
    return _tailVersion;
}

void AssetsManagerEx::setPackageUrl(const std::string &packageUrl)
{
    _packageUrl = packageUrl;
}

const std::string &AssetsManagerEx::getPackageUrl()
{
    return _packageUrl;
}

void AssetsManagerEx::downloadVersion()
{
    if (_updateState > State::PREDOWNLOAD_VERSION)
        return;

    std::string versionUrl = _localManifest->getVersionFileUrl();

    if (versionUrl.size() > 0)
    {
        _updateState = State::DOWNLOADING_VERSION;
        _downloader->createDownloadFileTask(versionUrl, _tempVersionPath, VERSION_ID);
    }
    else
    {
        CCLOG("AssetsManagerEx : No version file found, step skipped\n");
        _updateState = State::PREDOWNLOAD_MANIFEST;
        downloadManifest();
    }
}

void AssetsManagerEx::parseVersion()
{
    if (_updateState != State::VERSION_LOADED)
        return;

    _remoteManifest->parseVersion(_tempVersionPath);

    if (!_remoteManifest->isVersionLoaded())
    {
        CCLOG("AssetsManagerEx : Fail to parse version file, step skipped\n");
        _updateState = State::PREDOWNLOAD_MANIFEST;
        downloadManifest();
    }
    else
    {
        if (_localManifest->versionGreaterOrEquals(_remoteManifest, _versionCompareHandle))
        {
            _updateState = State::UP_TO_DATE;
            _fileUtils->removeDirectory(_tempStoragePath);
            dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE);
        }
        else
        {
            _updateState = State::NEED_UPDATE;

            if (_updateEntry == UpdateEntry::DO_UPDATE)
            {
                dispatchUpdateEvent(EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND);
                _updateState = State::PREDOWNLOAD_MANIFEST;
                downloadManifest();
            }
            else
            {
                dispatchUpdateEvent(EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND);
            }
        }
    }
}

void AssetsManagerEx::downloadManifest()
{
    if (_updateState != State::PREDOWNLOAD_MANIFEST)
        return;

    std::string manifestUrl = _localManifest->getManifestFileUrl();

    if (manifestUrl.size() > 0)
    {
        _updateState = State::DOWNLOADING_MANIFEST;
        _downloader->createDownloadFileTask(manifestUrl, _tempManifestPath, MANIFEST_ID);
    }
    else
    {
        CCLOG("AssetsManagerEx : No manifest file found, check update failed\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST);
        _updateState = State::UNCHECKED;
    }
}

void AssetsManagerEx::parseManifest()
{
    if (_updateState != State::MANIFEST_LOADED)
        return;

    _remoteManifest->parseFile(_tempManifestPath);

    if (!_remoteManifest->isLoaded())
    {
        CCLOG("AssetsManagerEx : Error parsing manifest file, %s", _tempManifestPath.c_str());
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_PARSE_MANIFEST);
        _updateState = State::UNCHECKED;
    }
    else
    {
        if (_localManifest->versionGreaterOrEquals(_remoteManifest, _versionCompareHandle))
        {
            _updateState = State::UP_TO_DATE;
            _fileUtils->removeDirectory(_tempStoragePath);
            dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE);
        }
        else
        {
            _updateState = State::NEED_UPDATE;
            dispatchUpdateEvent(EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND);

            if (_updateEntry == UpdateEntry::DO_UPDATE)
            {
                CCLOG("AssetsManagerEx parseManifest");
                startUpdate();
            }
        }
    }
}

void AssetsManagerEx::prepareUpdate()
{
    if (_updateState != State::NEED_UPDATE)
        return;
    _failedUnits.clear();
    _downloadUnits.clear();
    _totalWaitToDownload = _totalToDownload = 0;
    _nextSavePoint = 0;
    _percent = _percentByFile = _sizeCollected = _totalDownloaded = _totalSize = 0;
    _downloadResumed = false;
    _downloadedSize.clear();
    _totalEnabled = false;

    bool isloaded = _tempManifest->isLoaded();
    bool isupdating = _tempManifest->isUpdating();
    bool isversionequls = _tempManifest->versionEquals(_remoteManifest);
    if (_tempManifest && isloaded && isupdating && isversionequls)
    {
        _tempManifest->saveToFile(_tempManifestPath);
        _tempManifest->genResumeAssetsList(&_downloadUnits);
        _totalWaitToDownload = _totalToDownload = (int)_downloadUnits.size();
        _downloadResumed = true;
    }
    else
    {
        if (_tempManifest)
        {
            _fileUtils->removeDirectory(_tempStoragePath);
            CC_SAFE_RELEASE(_tempManifest);
            _fileUtils->createDirectory(_tempStoragePath);
            _remoteManifest->saveToFile(_tempManifestPath);
        }

        _tempManifest = _remoteManifest;
        if (_packageUrl != "")
        {
            _tempManifest->setPackageUrl(_packageUrl);
            _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
            std::string _verManifestUrl = _manifestUrl;
            findAndReplaceAll(_verManifestUrl, "resources", "config");
            _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
        }
        std::unordered_map<std::string, Manifest::AssetDiff> diff_map = _localManifest->genDiff(_remoteManifest);
        if (diff_map.size() == 0)
        {
            updateSucceed();
            return;
        }
        else
        {
            std::string packageUrl = _remoteManifest->getPackageUrl();
            for (auto it = diff_map.begin(); it != diff_map.end(); ++it)
            {
                Manifest::AssetDiff diff = it->second;
                if (diff.type == Manifest::DiffType::DELETED)
                {
                }
                else
                {
                    std::string path = diff.asset.path;
                    DownloadUnit unit;
                    unit.customId = it->first;
                    unit.srcUrl = packageUrl + path;
                    unit.storagePath = _tempStoragePath + path;
                    unit.size = diff.asset.size;
                    _downloadUnits.emplace(unit.customId, unit);
                    _tempManifest->setAssetDownloadState(it->first, Manifest::DownloadState::UNSTARTED);
                }
            }
            _tempManifest->setUpdating(true);
            _tempManifest->saveToFile(_tempManifestPath);

            _totalWaitToDownload = _totalToDownload = (int)_downloadUnits.size();
        }
    }
    _updateState = State::READY_TO_UPDATE;
}

void AssetsManagerEx::startUpdate()
{
    if (_updateState == State::NEED_UPDATE)
    {
        prepareUpdate();
    }
    if (_updateState == State::READY_TO_UPDATE)
    {
        _updateState = State::UPDATING;
        std::string msg;
        if (_downloadResumed)
        {
            msg = StringUtils::format("Resuming from previous unfinished update, %d files remains to be finished.", _totalToDownload);
        }
        else
        {
            msg = StringUtils::format("Start to update %d files from remote package.", _totalToDownload);
        }
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION, "", msg);
        batchDownload();
    }
}

void AssetsManagerEx::updateSucceed()
{
    _tempManifest->setUpdating(false);

    _fileUtils->renameFile(_tempStoragePath, TEMP_MANIFEST_FILENAME, MANIFEST_FILENAME);
    if (_fileUtils->isDirectoryExist(_tempStoragePath))
    {
        std::vector<std::string> files;
        _fileUtils->listFilesRecursively(_tempStoragePath, &files);
        int baseOffset = (int)_tempStoragePath.length();
        std::string relativePath, dstPath;
        for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it)
        {
            relativePath.assign((*it).substr(baseOffset));
            dstPath.assign(_storagePath + relativePath);
            if (relativePath.back() == '/')
            {
                _fileUtils->createDirectory(dstPath);
            }
            else
            {
                if (_fileUtils->isFileExist(dstPath))
                {
                    _fileUtils->removeFile(dstPath);
                }
                _fileUtils->renameFile(*it, dstPath);
            }
        }
        _fileUtils->removeDirectory(_tempStoragePath);
    }
    CC_SAFE_RELEASE(_localManifest);
    _localManifest = _remoteManifest;
    if (_packageUrl != "")
    {
        _localManifest->setPackageUrl(_packageUrl);
        _localManifest->setManifestFileUrl(_packageUrl + _manifestUrl);
        std::string _verManifestUrl = _manifestUrl;
        findAndReplaceAll(_verManifestUrl, "resources", "config");
        _localManifest->setVersionFileUrl(_packageUrl + _verManifestUrl);
    }
    _localManifest->setManifestRoot(_storagePath);
    _remoteManifest = nullptr;
    prepareLocalManifest();
    _updateState = State::UP_TO_DATE;
    dispatchUpdateEvent(EventAssetsManagerEx::EventCode::UPDATE_FINISHED);
}

void AssetsManagerEx::checkUpdate()
{
    if (_updateEntry != UpdateEntry::NONE)
    {
        CCLOGERROR("AssetsManagerEx::checkUpdate, updateEntry isn't NONE");
        return;
    }

    if (!_inited)
    {
        CCLOG("AssetsManagerEx : Manifests uninited.\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST);
        return;
    }
    if (!_localManifest->isLoaded())
    {
        CCLOG("AssetsManagerEx : No local manifest file found error.\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST);
        return;
    }

    _updateEntry = UpdateEntry::CHECK_UPDATE;

    switch (_updateState)
    {
    case State::FAIL_TO_UPDATE:
        _updateState = State::UNCHECKED;
    case State::UNCHECKED:
    case State::PREDOWNLOAD_VERSION:
    {
        downloadVersion();
    }
    break;
    case State::UP_TO_DATE:
    {
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ALREADY_UP_TO_DATE);
    }
    break;
    case State::NEED_UPDATE:
    {
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::NEW_VERSION_FOUND);
    }
    break;
    default:
        break;
    }
}

void AssetsManagerEx::update()
{
    if (_updateEntry != UpdateEntry::NONE)
    {
        CCLOGERROR("AssetsManagerEx::update, updateEntry isn't NONE");
        return;
    }

    if (!_inited)
    {
        CCLOG("AssetsManagerEx : Manifests uninited.\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST);
        return;
    }
    if (!_localManifest->isLoaded())
    {
        CCLOG("AssetsManagerEx : No local manifest file found error.\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST);
        return;
    }

    _updateEntry = UpdateEntry::DO_UPDATE;

    switch (_updateState)
    {
    case State::UNCHECKED:
    {
        _updateState = State::PREDOWNLOAD_VERSION;
    }
    case State::PREDOWNLOAD_VERSION:
    {
        downloadVersion();
    }
    break;
    case State::VERSION_LOADED:
    {
        parseVersion();
    }
    break;
    case State::PREDOWNLOAD_MANIFEST:
    {
        downloadManifest();
    }
    break;
    case State::MANIFEST_LOADED:
    {
        parseManifest();
    }
    break;
    case State::FAIL_TO_UPDATE:
    case State::NEED_UPDATE:
    {
        if (!_remoteManifest->isLoaded())
        {
            _updateState = State::PREDOWNLOAD_MANIFEST;
            downloadManifest();
        }
        else
        {
            startUpdate();
        }
    }
    break;
    case State::UP_TO_DATE:
    case State::UPDATING:
    case State::UNZIPPING:
        _updateEntry = UpdateEntry::NONE;
        break;
    default:
        break;
    }
}

void AssetsManagerEx::updateAssets(const DownloadUnits &assets)
{
    if (!_inited)
    {
        CCLOG("AssetsManagerEx : Manifests uninited.\n");
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_NO_LOCAL_MANIFEST);
        return;
    }

    if (_updateState != State::UPDATING && _localManifest->isLoaded() && _remoteManifest->isLoaded())
    {
        _updateState = State::UPDATING;
        _downloadUnits.clear();
        _downloadedSize.clear();
        _percent = _percentByFile = _sizeCollected = _totalDownloaded = _totalSize = 0;
        _totalWaitToDownload = _totalToDownload = (int)assets.size();
        _nextSavePoint = 0;
        _totalEnabled = false;
        if (_totalToDownload > 0)
        {
            _downloadUnits = assets;
            this->batchDownload();
        }
        else if (_totalToDownload == 0)
        {
            onDownloadUnitsFinished();
        }
    }
}

const DownloadUnits &AssetsManagerEx::getFailedAssets() const
{
    return _failedUnits;
}

void AssetsManagerEx::downloadFailedAssets()
{
    CCLOG("AssetsManagerEx : Start update %lu failed assets.\n", static_cast<unsigned long>(_failedUnits.size()));
    updateAssets(_failedUnits);
}

void AssetsManagerEx::fileError(const std::string &identifier, const std::string &errorStr, int errorCode, int errorCodeInternal)
{
    auto unitIt = _downloadUnits.find(identifier);
    if (unitIt != _downloadUnits.end())
    {
        _totalWaitToDownload--;

        DownloadUnit unit = unitIt->second;
        _failedUnits.emplace(unit.customId, unit);
    }
    dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_UPDATING, identifier, errorStr, errorCode, errorCodeInternal);
    _tempManifest->setAssetDownloadState(identifier, Manifest::DownloadState::UNSTARTED);

    _currConcurrentTask = std::max(0, _currConcurrentTask - 1);
    queueDowload();
}

void AssetsManagerEx::fileSuccess(const std::string &customId, const std::string &storagePath)
{
    _tempManifest->setAssetDownloadState(customId, Manifest::DownloadState::SUCCESSED);

    auto unitIt = _failedUnits.find(customId);
    if (unitIt != _failedUnits.end())
    {
        _failedUnits.erase(unitIt);
    }

    unitIt = _downloadUnits.find(customId);
    if (unitIt != _downloadUnits.end())
    {
        _totalWaitToDownload--;

        _percentByFile = 100 * (float)(_totalToDownload - _totalWaitToDownload) / _totalToDownload;
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION, "");
    }
    dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ASSET_UPDATED, customId);

    _currConcurrentTask = std::max(0, _currConcurrentTask - 1);
    queueDowload();
}

void AssetsManagerEx::onError(const network::DownloadTask &task,
                              int errorCode,
                              int errorCodeInternal,
                              const std::string &errorStr)
{
    if (task.identifier == VERSION_ID)
    {
        CCLOG("AssetsManagerEx : Fail to download version file, step skipped\n");
        _updateState = State::PREDOWNLOAD_MANIFEST;
        downloadManifest();
    }
    else if (task.identifier == MANIFEST_ID)
    {
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::ERROR_DOWNLOAD_MANIFEST, task.identifier, errorStr, errorCode, errorCodeInternal);
        _updateState = State::FAIL_TO_UPDATE;
    }
    else
    {
        fileError(task.identifier, errorStr, errorCode, errorCodeInternal);
    }
}

void AssetsManagerEx::onProgress(double total, double downloaded, const std::string & /*url*/, const std::string &customId)
{
    if (customId == VERSION_ID || customId == MANIFEST_ID)
    {
        _percent = 100 * downloaded / total;
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION, customId);
        return;
    }
    else
    {
        bool found = false;
        _totalDownloaded = 0;
        for (auto it = _downloadedSize.begin(); it != _downloadedSize.end(); ++it)
        {
            if (it->first == customId)
            {
                it->second = downloaded;
                found = true;
            }
            _totalDownloaded += it->second;
        }
        if (!found)
        {
            _tempManifest->setAssetDownloadState(customId, Manifest::DownloadState::DOWNLOADING);
            _downloadedSize.emplace(customId, downloaded);
            if (_downloadUnits[customId].size == 0)
            {
                _totalSize += total;
                _sizeCollected++;
                if (_sizeCollected == _totalToDownload)
                {
                    _totalEnabled = true;
                }
            }
        }

        if (_totalEnabled && _updateState == State::UPDATING)
        {
            float currentPercent = 100 * _totalDownloaded / _totalSize;
            if ((int)currentPercent != (int)_percent)
            {
                _percent = currentPercent;
                dispatchUpdateEvent(EventAssetsManagerEx::EventCode::UPDATE_PROGRESSION, customId);
            }
        }
    }
}

void AssetsManagerEx::onSuccess(const std::string & /*srcUrl*/, const std::string &storagePath, const std::string &customId)
{
    if (customId == VERSION_ID)
    {
        _updateState = State::VERSION_LOADED;
        parseVersion();
    }
    else if (customId == MANIFEST_ID)
    {
        _updateState = State::MANIFEST_LOADED;
        parseManifest();
    }
    else
    {
        bool ok = true;
        auto &assets = _remoteManifest->getAssets();
        auto assetIt = assets.find(customId);
        if (assetIt != assets.end())
        {
            Manifest::Asset asset = assetIt->second;
            if (_verifyCallback != nullptr)
            {
                ok = _verifyCallback(storagePath, asset);
            }
        }

        if (ok)
        {
            bool compressed = assetIt != assets.end() ? assetIt->second.compressed : false;
            if (compressed)
            {
                decompressDownloadedZip(customId, storagePath);
            }
            else
            {
                fileSuccess(customId, storagePath);
            }
        }
        else
        {
            fileError(customId, "Asset file verification failed after downloaded");
        }
    }
}

void AssetsManagerEx::destroyDownloadedVersion()
{
    _fileUtils->removeDirectory(_storagePath);
    _fileUtils->removeDirectory(_tempStoragePath);
}

void AssetsManagerEx::batchDownload()
{
    _queue.clear();
    for (auto iter : _downloadUnits)
    {
        const DownloadUnit &unit = iter.second;
        if (unit.size > 0)
        {
            _totalSize += unit.size;
            _sizeCollected++;
        }

        _queue.push_back(iter.first);
    }
    if (_sizeCollected == _totalToDownload)
    {
        _totalEnabled = true;
    }

    queueDowload();
}

void AssetsManagerEx::queueDowload()
{
    if (_totalWaitToDownload == 0)
    {
        this->onDownloadUnitsFinished();
        return;
    }

    while (_currConcurrentTask < _maxConcurrentTask && _queue.size() > 0)
    {
        std::string key = _queue.back();
        _queue.pop_back();

        _currConcurrentTask++;
        DownloadUnit &unit = _downloadUnits[key];
        _fileUtils->createDirectory(basename(unit.storagePath));

        std::string tmpFilePath = unit.storagePath + ".tmp";
        if (_fileUtils->isFileExist(tmpFilePath))
        {
            _fileUtils->removeFile(tmpFilePath);
            cocos2d::log("AssetsManagerEx  download ==> remove tmp file before download: %s", tmpFilePath.c_str());
        }
        else
        {
        }
        _downloader->createDownloadFileTask(unit.srcUrl, unit.storagePath, unit.customId);

        _tempManifest->setAssetDownloadState(key, Manifest::DownloadState::DOWNLOADING);
    }
    if (_percentByFile / 100 > _nextSavePoint)
    {
        _tempManifest->saveToFile(_tempManifestPath);
        _nextSavePoint += SAVE_POINT_INTERVAL;
    }
}

void AssetsManagerEx::onDownloadUnitsFinished()
{
    if (_failedUnits.size() > 0)
    {
        _tempManifest->saveToFile(_tempManifestPath);

        _updateState = State::FAIL_TO_UPDATE;
        dispatchUpdateEvent(EventAssetsManagerEx::EventCode::UPDATE_FAILED);
    }
    else if (_updateState == State::UPDATING)
    {
        updateSucceed();
    }
}

NS_CC_EXT_END
