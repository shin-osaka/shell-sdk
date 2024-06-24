/****************************************************************************
 Copyright (c) 2013 cocos2d-x.org
 Copyright (c) 2014-2016 Chukong Technologies Inc.
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

#ifndef __AssetsManagerEx__
#define __AssetsManagerEx__

#include <string>
#include <unordered_map>
#include <vector>

#include "platform/CCFileUtils.h"
#include "network/CCDownloader.h"

#include "CCEventAssetsManagerEx.h"

#include "Manifest.hpp"
#include "extensions/ExtensionMacros.h"
#include "extensions/ExtensionExport.h"
#include "json/document-wrapper.h"


NS_CC_EXT_BEGIN

/**
 * @brief   This class is used to auto update resources, such as pictures or scripts.
 */
class CC_EX_DLL AssetsManagerEx : public Ref
{
public:
    
    enum class State
    {
        UNINITED,
        UNCHECKED,
        PREDOWNLOAD_VERSION,
        DOWNLOADING_VERSION,
        VERSION_LOADED,
        PREDOWNLOAD_MANIFEST,
        DOWNLOADING_MANIFEST,
        MANIFEST_LOADED,
        NEED_UPDATE,
        READY_TO_UPDATE,
        UPDATING,
        UNZIPPING,
        UP_TO_DATE,
        FAIL_TO_UPDATE
    };
    
    const static std::string VERSION_ID;
    const static std::string MANIFEST_ID;
    
    typedef std::function<int(const std::string& versionA, const std::string& versionB)> VersionCompareHandle;
    typedef std::function<bool(const std::string& path, Manifest::Asset asset)> VerifyCallback;
    typedef std::function<void(EventAssetsManagerEx *event)> EventCallback;
    
    /** @brief Create function for creating a new AssetsManagerEx
     @param manifestUrl   The url for the local manifest file
     @param storagePath   The storage path for downloaded assets
     @warning   The cached manifest in your storage path have higher priority and will be searched first,
                only if it doesn't exist, AssetsManagerEx will use the given manifestUrl.
     */
    static AssetsManagerEx* create(const std::string &manifestUrl,
                                   const std::string &storagePath,
                                   const std::string &packageUrl = "",
                                   const std::string& strTail = "");
    
    /** @brief  Check out if there is a new version of manifest.
     *          You may use this method before updating, then let user determine whether
     *          he wants to update resources.
     */
    void checkUpdate();
    
    /** @brief Prepare the update process, this will cleanup download process flags, fill up download units with temporary manifest or remote manifest
     */
    void prepareUpdate();
    
    void setTailVersion(const std::string& tversion);
    const std::string& getTailVersion();
    
    void setPackageUrl(const std::string& packageUrl);
    const std::string& getPackageUrl();
    
    /** @brief Update with the current local manifest.
     */
    void update();
    
    /** @brief Reupdate all failed assets under the current AssetsManagerEx context
     */
    void downloadFailedAssets();
    
    /** @brief Gets the current update state.
     */
    State getState() const;
    
    /** @brief Gets storage path.
     */
    const std::string& getStoragePath() const;
    
    /** @brief Function for retrieving the local manifest object
     */
    const Manifest* getLocalManifest() const;
    
    /** @brief Load a custom local manifest object, the local manifest must be loaded already.
     * You can only manually load local manifest when the update state is UNCHECKED, it will fail once the update process is began.
     * This API will do the following things:
     * 1. Reset storage path
     * 2. Set local storage
     * 3. Search for cached manifest and compare with the local manifest
     * 4. Init temporary manifest and remote manifest
     * If successfully load the given local manifest and inited other manifests, it will return true, otherwise it will return false
     * @param localManifest    The local manifest object to be set
     * @param storagePath    The local storage path
     */
    bool loadLocalManifest(Manifest* localManifest, const std::string& storagePath);
    
    /** @brief Load a local manifest from url.
     * You can only manually load local manifest when the update state is UNCHECKED, it will fail once the update process is began.
     * This API will do the following things:
     * 1. Reset storage path
     * 2. Set local storage
     * 3. Search for cached manifest and compare with the local manifest
     * 4. Init temporary manifest and remote manifest
     * If successfully load the given local manifest and inited other manifests, it will return true, otherwise it will return false
     * @param manifestUrl    The local manifest url
     */
    bool loadLocalManifest(const std::string& manifestUrl);
    
    /** @brief Function for retrieving the remote manifest object
     */
    const Manifest* getRemoteManifest() const;
    
    /** @brief Load a custom remote manifest object, the manifest must be loaded already.
     * You can only manually load remote manifest when the update state is UNCHECKED and local manifest is already inited, it will fail once the update process is began.
     * @param remoteManifest    The remote manifest object to be set
     */
    bool loadRemoteManifest(Manifest* remoteManifest);
    
    /** @brief Gets whether the current download is resuming previous unfinished job, this will only be available after READY_TO_UPDATE state, under unknown states it will return false by default.
     */
    bool isResuming() const {return _downloadResumed;};
    
    /** @brief Gets the total byte size to be downloaded of the update, this will only be available after READY_TO_UPDATE state, under unknown states it will return 0 by default.
     */
    double getTotalBytes() const {return _totalSize;};
    
    /** @brief Gets the current downloaded byte size of the update, this will only be available after READY_TO_UPDATE state, under unknown states it will return 0 by default.
     */
    double getDownloadedBytes() const {return _totalDownloaded;};
    
    /** @brief Gets the total files count to be downloaded of the update, this will only be available after READY_TO_UPDATE state, under unknown states it will return 0 by default.
     */
    int getTotalFiles() const {return _totalToDownload;};
    
    /** @brief Gets the current downloaded files count of the update, this will only be available after READY_TO_UPDATE state, under unknown states it will return 0 by default.
     */
    int getDownloadedFiles() const {return _totalToDownload - _totalWaitToDownload;};
    
    /** @brief Function for retrieving the max concurrent task count
     */
    const int getMaxConcurrentTask() const {return _maxConcurrentTask;};
    
    /** @brief Function for setting the max concurrent task count
     */
    void setMaxConcurrentTask(const int max) {_maxConcurrentTask = max;};
    
    /** @brief Set the handle function for comparing manifests versions
     * @param handle    The compare function
     */
    void setVersionCompareHandle(const VersionCompareHandle& handle) {_versionCompareHandle = handle;};
    
    /** @brief Set the verification function for checking whether downloaded asset is correct, e.g. using md5 verification
     * @param callback  The verify callback function
     */
    void setVerifyCallback(const VerifyCallback& callback) {_verifyCallback = callback;};
    
    /** @brief Set the event callback for receiving update process events
     * @param callback  The event callback function
     */
    void setEventCallback(const EventCallback& callback) {_eventCallback = callback;};
    
CC_CONSTRUCTOR_ACCESS:
    
    AssetsManagerEx(const std::string& manifestUrl,
                    const std::string& storagePath,
                    const std::string& packageUrl = "",
                    const std::string& tailVersion = "");
    
    AssetsManagerEx(const std::string& manifestUrl,
                    const std::string& storagePath,
                    const VersionCompareHandle& handle,
                    const std::string& packageUrl = "",
                    const std::string& tailVersion = "");
    
    virtual ~AssetsManagerEx();
    
protected:
    
    void init(const std::string& manifestUrl, const std::string& storagePath);
    
    std::string basename(const std::string& path) const;
    
    std::string get(const std::string& key) const;
    
    void initManifests();
    
    void prepareLocalManifest();
    
    void setStoragePath(const std::string& storagePath);
    
    void adjustPath(std::string &path);
    
    void dispatchUpdateEvent(EventAssetsManagerEx::EventCode code, const std::string &message = "", const std::string &assetId = "", int curle_code = 0, int curlm_code = 0);
    
    void downloadVersion();
    void parseVersion();
    void downloadManifest();
    void parseManifest();
    void startUpdate();
    void updateSucceed();
    bool decompress(const std::string &filename);
    void decompressDownloadedZip(const std::string &customId, const std::string &storagePath);
    
    /** @brief Update a list of assets under the current AssetsManagerEx context
     */
    void updateAssets(const DownloadUnits& assets);
    
    /** @brief Retrieve all failed assets during the last update
     */
    const DownloadUnits& getFailedAssets() const;
    
    /** @brief Function for destroying the downloaded version file and manifest file
     */
    void destroyDownloadedVersion();
    
    /** @brief Download items in queue with max concurrency setting
     */
    void queueDowload();
    
    void fileError(const std::string& identifier, const std::string& errorStr, int errorCode = 0, int errorCodeInternal = 0);
    
    void fileSuccess(const std::string &customId, const std::string &storagePath);
    
    /** @brief  Call back function for error handling,
     the error will then be reported to user's listener registed in addUpdateEventListener
     @param error   The error object contains ErrorCode, message, asset url, asset key
     @warning AssetsManagerEx internal use only
     * @js NA
     * @lua NA
     */
    virtual void onError(const network::DownloadTask& task,
                         int errorCode,
                         int errorCodeInternal,
                         const std::string& errorStr);
    
    /** @brief  Call back function for recording downloading percent of the current asset,
     the progression will then be reported to user's listener registed in addUpdateProgressEventListener
     @param total       Total size to download for this asset
     @param downloaded  Total size already downloaded for this asset
     @param url         The url of this asset
     @param customId    The key of this asset
     @warning AssetsManagerEx internal use only
     * @js NA
     * @lua NA
     */
    virtual void onProgress(double total, double downloaded, const std::string &url, const std::string &customId);
    
    /** @brief  Call back function for success of the current asset
     the success event will then be send to user's listener registed in addUpdateEventListener
     @param srcUrl      The url of this asset
     @param customId    The key of this asset
     @warning AssetsManagerEx internal use only
     * @js NA
     * @lua NA
     */
    virtual void onSuccess(const std::string &srcUrl, const std::string &storagePath, const std::string &customId);
    
private:
    void batchDownload();

    void onDownloadUnitsFinished();
    
    std::string _eventName;
    
    FileUtils *_fileUtils;
    
    State _updateState;
    
    std::shared_ptr<network::Downloader> _downloader;
    
    const std::unordered_map<std::string, Manifest::Asset> *_assets;
    
    std::string _storagePath;
    
    std::string _tempStoragePath;
    
    std::string _tempVersionPath;
    
    std::string _cacheManifestPath;
    
    std::string _tempManifestPath;
    
    std::string _manifestUrl;
    
    Manifest *_localManifest;
    
    Manifest *_tempManifest;
    
    Manifest *_remoteManifest;
    
    enum class UpdateEntry : char
    {
        NONE,
        CHECK_UPDATE,
        DO_UPDATE
    };

    UpdateEntry _updateEntry;
    
    DownloadUnits _downloadUnits;
    
    DownloadUnits _failedUnits;
    
    std::vector<std::string> _queue;
    
    bool _downloadResumed;
    
    int _maxConcurrentTask;
    
    int _currConcurrentTask;
    
    float _percent;
    
    float _percentByFile;
    
    int _totalEnabled;
    
    int _sizeCollected;
    
    double _totalSize;
    
    double _totalDownloaded;
    
    std::unordered_map<std::string, double> _downloadedSize;
    
    int _totalToDownload;
    int _totalWaitToDownload;
    float _nextSavePoint;
    
    VersionCompareHandle _versionCompareHandle;
    
    VerifyCallback _verifyCallback;
    
    EventCallback _eventCallback;
    
    bool _inited;
    
    std::string _tailVersion;  //manifest 下载文件的后缀
    std::string _packageUrl;   //HotUpdate资源根目录
};

NS_CC_EXT_END

#endif /* defined(__AssetsManagerEx__) */
