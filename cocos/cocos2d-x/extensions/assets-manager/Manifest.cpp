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

#include "Manifest.hpp"
#include "json/prettywriter.h"
#include "json/stringbuffer.h"
#include <fstream>
#include <stdio.h>

#define KEY_VERSION             "version"
#define KEY_PACKAGE_URL         "packageUrl"
#define KEY_MANIFEST_URL        "remoteManifestUrl"
#define KEY_VERSION_URL         "remoteVersionUrl"
#define KEY_GROUP_VERSIONS      "groupVersions"
#define KEY_ENGINE_VERSION      "engineVersion"
#define KEY_UPDATING            "updating"
#define KEY_ASSETS              "assets"
#define KEY_COMPRESSED_FILES    "compressedFiles"
#define KEY_SEARCH_PATHS        "searchPaths"
#define KEY_HTTP_HOSTS          "gameUrl"
#define KEY_TCP_HOSTS           "startUrl"

#define KEY_PATH                "path"
#define KEY_MD5                 "md5"
#define KEY_GROUP               "group"
#define KEY_COMPRESSED          "compressed"
#define KEY_SIZE                "size"
#define KEY_GAME_ID             "id"
#define KEY_COMPRESSED_FILE     "compressedFile"
#define KEY_DOWNLOAD_STATE      "downloadState"

NS_CC_EXT_BEGIN


static int cmpVersion(const std::string& v1, const std::string& v2)
{
    int i;
    int oct_v1[4] = {0}, oct_v2[4] = {0};
    int filled1 = std::sscanf(v1.c_str(), "%d.%d.%d.%d", &oct_v1[0], &oct_v1[1], &oct_v1[2], &oct_v1[3]);
    int filled2 = std::sscanf(v2.c_str(), "%d.%d.%d.%d", &oct_v2[0], &oct_v2[1], &oct_v2[2], &oct_v2[3]);
    
    if (filled1 == 0 || filled2 == 0)
    {
        return strcmp(v1.c_str(), v2.c_str());
    }
    for (i = 0; i < 4; i++)
    {
        if (oct_v1[i] > oct_v2[i])
            return 1;
        else if (oct_v1[i] < oct_v2[i])
            return -1;
    }
    return 0;
}

Manifest::Manifest(const std::string& manifestUrl/* = ""*/,
                   const std::string& strPackageUrl/* = ""*/)
: _versionLoaded(false)
, _loaded(false)
, _updating(false)
, _manifestRoot("")
, _remoteManifestUrl("")
, _remoteVersionUrl("")
, _version("")
, _engineVer("")
, _packageUrl(strPackageUrl)
, _createManifest(false)
{
    _fileUtils = FileUtils::getInstance();
    initLocalGameList();
    if (manifestUrl.size() > 0)
        parseFile(manifestUrl);
}

Manifest::Manifest(const std::string& content,
                   const std::string& manifestRoot,
                   const std::string& strPackageUrl/* = ""*/)
: _versionLoaded(false)
, _loaded(false)
, _updating(false)
, _manifestRoot("")
, _remoteManifestUrl("")
, _remoteVersionUrl("")
, _version("")
, _engineVer("")
, _packageUrl(strPackageUrl)
, _createManifest(false)
{
    _fileUtils = FileUtils::getInstance();
    initLocalGameList();
    if (content.size() > 0)
        parseJSONString(content, manifestRoot);
}

void Manifest::initLocalGameList() {
    std::string fileName = "local_game_list.json";
    std::string content;
    if (_fileUtils->isFileExist(fileName))
    {
        content = _fileUtils->getStringFromFile(fileName);
        
        if (content.size() == 0)
        {
            CCLOG("Fail to retrieve local file content: %s\n", fileName.c_str());
        }
        else
        {
            if (content.size() == 0)
            {
                CCLOG("Fail to parse empty json content.");
            }
            else
            {
                rapidjson::Document jsonDoc;
                jsonDoc.Parse<0>(content.c_str());
                if (jsonDoc.HasParseError()) {
                    CCLOG("File parse error : local_game_list.json");
                } else {
                    int size = jsonDoc.Size();
                    for (int i=0; i<size; i++) {
                        if (jsonDoc[i].IsString()) {
                            _localGameList.push_back(jsonDoc[i].GetString());
                        }
                    }
                }
            }
            
        }
    }
}

void Manifest::loadJson(const std::string& url)
{
    clear();
    std::string content;
    if (_fileUtils->isFileExist(url))
    {
        content = _fileUtils->getStringFromFile(url);
        
        if (content.size() == 0)
        {
            CCLOG("Fail to retrieve local file content: %s\n", url.c_str());
        }
        else
        {
            loadJsonFromString(content);
        }
    }
}

void Manifest::loadJsonFromString(const std::string& content)
{
    if (content.size() == 0)
    {
        CCLOG("Fail to parse empty json content.");
    }
    else
    {
        _json.Parse<0>(content.c_str());
        if (_json.HasParseError()) {
            size_t offset = _json.GetErrorOffset();
            if (offset > 0)
                offset--;
            std::string errorSnippet = content.substr(offset, 10);
            CCLOG("File parse error %d at <%s>\n", _json.GetParseError(), errorSnippet.c_str());
        }
    }
}

void Manifest::parseVersion(const std::string& versionUrl)
{
    loadJson(versionUrl);
    
    if (_json.IsObject())
    {
        loadVersion(_json);
    }
}

void Manifest::parseFile(const std::string& manifestUrl)
{
    loadJson(manifestUrl);
    
    if (!_json.HasParseError() && _json.IsObject())
    {
        size_t found = manifestUrl.find_last_of("/\\");
        if (found != std::string::npos)
        {
            _manifestRoot = manifestUrl.substr(0, found+1);
        }
        loadManifest(_json);
    }
}

void Manifest::parseJSONString(const std::string& content, const std::string& manifestRoot)
{
    loadJsonFromString(content);
    
    if (!_json.HasParseError() && _json.IsObject())
    {
        _manifestRoot = manifestRoot;
        loadManifest(_json);
    }
}


bool Manifest::isVersionLoaded() const
{
    
    return _versionLoaded;
}

bool Manifest::isLoaded() const
{
    return _loaded;
}

void Manifest::setUpdating(bool updating)
{
    if (_loaded && _json.IsObject())
    {
        if (_json.HasMember(KEY_UPDATING) && _json[KEY_UPDATING].IsBool())
        {
            _json[KEY_UPDATING].SetBool(updating);
        }
        else
        {
            _json.AddMember<bool>(KEY_UPDATING, updating, _json.GetAllocator());
        }
        _updating = updating;
    }
}

bool Manifest::versionEquals(const Manifest *b) const
{
    if (_version != b->getVersion())
    {
        return false;
    }
    else
    {
        std::vector<std::string> bGroups = b->getGroups();
        std::unordered_map<std::string, std::string> bGroupVer = b->getGroupVerions();
        if (bGroups.size() != _groups.size())
            return false;
        
        for (unsigned int i = 0; i < _groups.size(); ++i) {
            std::string gid =_groups[i];
            if (gid != bGroups[i])
                return false;
            if (_groupVer.at(gid) != bGroupVer.at(gid))
                return false;
        }
    }
    return true;
}

bool Manifest::versionGreaterOrEquals(const Manifest *b, const std::function<int(const std::string& versionA, const std::string& versionB)>& handle) const
{
    std::string localVersion = getVersion();
    std::string bVersion = b->getVersion();
    bool greater;
    if (handle)
    {
        greater = handle(localVersion, bVersion) >= 0;
    }
    else
    {
        greater = cmpVersion(localVersion, bVersion) >= 0;
    }
    return greater;
}

bool Manifest::versionGreater(const Manifest *b, const std::function<int(const std::string& versionA, const std::string& versionB)>& handle) const
{
    std::string localVersion = getVersion();
    std::string bVersion = b->getVersion();
    bool greater;
    if (handle)
    {
        greater = handle(localVersion, bVersion) > 0;
    }
    else
    {
        greater = cmpVersion(localVersion, bVersion) > 0;
    }
    return greater;
}

std::unordered_map<std::string, Manifest::AssetDiff> Manifest::genDiff(const Manifest *b) const
{
    std::unordered_map<std::string, AssetDiff> diff_map;
    const std::unordered_map<std::string, Asset> &bAssets = b->getAssets();
    
    std::string key;
    Asset valueA;
    Asset valueB;
    
    std::unordered_map<std::string, Asset>::const_iterator valueIt, it;
    for (it = _assets.begin(); it != _assets.end(); ++it)
    {
        key = it->first;
        valueA = it->second;
        
        valueIt = bAssets.find(key);
        if (valueIt == bAssets.cend()) {
            AssetDiff diff;
            diff.asset = valueA;
            diff.type = DiffType::DELETED;
            diff_map.emplace(key, diff);
            cocos2d::log("delete path: %s", valueA.path.c_str());
            continue;
        }
        
        valueB = valueIt->second;
        if (valueA.md5 != valueB.md5) {
            AssetDiff diff;
            diff.asset = valueB;
            diff.type = DiffType::MODIFIED;
            std::string game_id = valueA.game_id;
            long nRet = std::count(_localGameList.begin(), _localGameList.end(), game_id);
            if ("0" == game_id || nRet > 0) {
                diff_map.emplace(key, diff);
                cocos2d::log("modify res: %s", valueA.path.c_str());
            } else {
                cocos2d::log("res game id: %s, no need to update.", game_id.c_str());
            }
            
        }
    }
    
    for (it = bAssets.begin(); it != bAssets.end(); ++it)
    {
        key = it->first;
        valueB = it->second;
        
        valueIt = _assets.find(key);
        if (valueIt == _assets.cend()) {
            AssetDiff diff;
            diff.asset = valueB;
            diff.type = DiffType::ADDED;
            std::string game_id = valueB.game_id;
            long nRet = std::count(_localGameList.begin(), _localGameList.end(), game_id);
            if ("0" == game_id || nRet > 0) {
                diff_map.emplace(key, diff);
                cocos2d::log("added res: %s", valueB.path.c_str());
            } else {
                cocos2d::log("res game id: %s, no need to update.", game_id.c_str());
            }
        }
    }
    
    return diff_map;
}

void Manifest::genResumeAssetsList(DownloadUnits *units) const
{
    for (auto it = _assets.begin(); it != _assets.end(); ++it)
    {
        Asset asset = it->second;
        
        if (asset.downloadState != DownloadState::SUCCESSED && asset.downloadState != DownloadState::UNMARKED)
        {
            DownloadUnit unit;
            unit.customId = it->first;
            unit.srcUrl = _packageUrl + asset.path;
            unit.storagePath = _manifestRoot + asset.path;
            unit.size = asset.size;
            unit.game_id = asset.game_id;
            units->emplace(unit.customId, unit);
        }
    }
}

void Manifest::genResumeAssetsList(DownloadUnits *units, const std::string remoteUrl) const
{
    for (auto it = _assets.begin(); it != _assets.end(); ++it)
    {
        Asset asset = it->second;
        
        if (asset.downloadState != DownloadState::SUCCESSED && asset.downloadState != DownloadState::UNMARKED)
        {
            DownloadUnit unit;
            unit.customId = it->first;
            unit.srcUrl = remoteUrl + asset.path;
            unit.storagePath = _manifestRoot + asset.path;
            unit.size = asset.size;
            unit.game_id = asset.game_id;
            units->emplace(unit.customId, unit);
        }
    }
}

std::vector<std::string> Manifest::getSearchPaths() const
{
    std::vector<std::string> searchPaths;
    searchPaths.push_back(_manifestRoot);
    
    for (int i = (int)_searchPaths.size()-1; i >= 0; i--)
    {
        std::string path = _searchPaths[i];
        if (path.size() > 0 && path[path.size() - 1] != '/')
            path.append("/");
        path = _manifestRoot + path;
        searchPaths.push_back(path);
    }
    return searchPaths;
}

std::vector<std::string> Manifest::getHttpHost() const{
    return _httpHost;
}

std::vector<std::string> Manifest::getWsHost() const{
    return _wsHost;
}

void Manifest::prependSearchPaths()
{
    
}


const std::string& Manifest::getPackageUrl() const
{
    return _packageUrl;
}

void Manifest::setPackageUrl(const std::string &packageUrl){
    _packageUrl = packageUrl;
    if (_packageUrl.size() > 0 && _packageUrl[_packageUrl.size() - 1] != '/')
    {
        _packageUrl.append("/");
    }
}

const std::string& Manifest::getManifestFileUrl() const
{
    return _remoteManifestUrl;
}

void Manifest::setManifestFileUrl(const std::string &manifestUrl){
    _remoteManifestUrl = manifestUrl;
}

const std::string& Manifest::getVersionFileUrl() const
{
    return _remoteVersionUrl;
}

void Manifest::setVersionFileUrl(const std::string &versionUrl){
    _remoteVersionUrl = versionUrl;
}

const std::string& Manifest::getVersion() const
{
    return _version;
}

const std::vector<std::string>& Manifest::getGroups() const
{
    return _groups;
}

const std::unordered_map<std::string, std::string>& Manifest::getGroupVerions() const
{
    return _groupVer;
}

const std::string& Manifest::getGroupVersion(const std::string &group) const
{
    return _groupVer.at(group);
}

const std::unordered_map<std::string, Manifest::Asset>& Manifest::getAssets() const
{
    return _assets;
}

void Manifest::setAssetDownloadState(const std::string &key, const Manifest::DownloadState &state)
{
    auto valueIt = _assets.find(key);
    if (valueIt != _assets.end())
    {
        valueIt->second.downloadState = state;
        
        if(_json.IsObject())
        {
            if ( _json.HasMember(KEY_ASSETS) )
            {
                rapidjson::Value &assets = _json[KEY_ASSETS];
                if (assets.IsObject())
                {
                    if (assets.HasMember(key.c_str()))
                    {
                        rapidjson::Value &entry = assets[key.c_str()];
                        if (entry.HasMember(KEY_DOWNLOAD_STATE) && entry[KEY_DOWNLOAD_STATE].IsInt())
                        {
                            entry[KEY_DOWNLOAD_STATE].SetInt((int) state);
                        }
                        else
                        {
                            entry.AddMember<int>(KEY_DOWNLOAD_STATE, (int)state, _json.GetAllocator());
                        }
                    }
                }
            }
        }
    }
}

void Manifest::clear()
{
    if (_versionLoaded || _loaded)
    {
        _groups.clear();
        _groupVer.clear();
        
        _remoteManifestUrl = "";
        _remoteVersionUrl = "";
        _version = "";
        _engineVer = "";
        
        _versionLoaded = false;
    }
    
    if (_loaded)
    {
        _assets.clear();
        _searchPaths.clear();
        _loaded = false;
    }
}

Manifest::Asset Manifest::parseAsset(const std::string &path, const rapidjson::Value &json)
{
    Asset asset;
    asset.path = path;
    
    if ( json.HasMember(KEY_MD5) && json[KEY_MD5].IsString() )
    {
        asset.md5 = json[KEY_MD5].GetString();
    }
    else asset.md5 = "";
    
    if ( json.HasMember(KEY_PATH) && json[KEY_PATH].IsString() )
    {
        asset.path = json[KEY_PATH].GetString();
    }
    
    if ( json.HasMember(KEY_COMPRESSED) && json[KEY_COMPRESSED].IsBool() )
    {
        asset.compressed = json[KEY_COMPRESSED].GetBool();
    }
    else asset.compressed = false;
    
    if ( json.HasMember(KEY_SIZE) && json[KEY_SIZE].IsInt() )
    {
        asset.size = json[KEY_SIZE].GetInt();
    }
    else asset.size = 0;
    
    if ( json.HasMember(KEY_GAME_ID) && json[KEY_GAME_ID].IsString() )
    {
        asset.game_id = json[KEY_GAME_ID].GetString();
    }
    else asset.game_id = "0";
    
    if ( json.HasMember(KEY_DOWNLOAD_STATE) && json[KEY_DOWNLOAD_STATE].IsInt() )
    {
        asset.downloadState = (json[KEY_DOWNLOAD_STATE].GetInt());
    }
    else asset.downloadState = DownloadState::UNMARKED;
    
    return asset;
}

void Manifest::loadVersion(const rapidjson::Document &json)
{
    if ( json.HasMember(KEY_MANIFEST_URL) && json[KEY_MANIFEST_URL].IsString() )
    {
        if (_packageUrl == "") {
            _remoteManifestUrl = json[KEY_MANIFEST_URL].GetString();
        }else{
            _remoteManifestUrl = _packageUrl + "resources.u3d";
        }
    }
    
    if ( json.HasMember(KEY_VERSION_URL) && json[KEY_VERSION_URL].IsString() )
    {
        if (_packageUrl == "") {
            _remoteVersionUrl = json[KEY_VERSION_URL].GetString();
        }else{
            _remoteManifestUrl = _packageUrl + "config.u3d";
        }
    }
    
    if ( json.HasMember(KEY_VERSION) && json[KEY_VERSION].IsString() )
    {
        _version = json[KEY_VERSION].GetString();
    }
    
    if ( json.HasMember(KEY_GROUP_VERSIONS) )
    {
        const rapidjson::Value& groupVers = json[KEY_GROUP_VERSIONS];
        if (groupVers.IsObject())
        {
            for (rapidjson::Value::ConstMemberIterator itr = groupVers.MemberBegin(); itr != groupVers.MemberEnd(); ++itr)
            {
                std::string group = itr->name.GetString();
                std::string version = "0";
                if (itr->value.IsString())
                {
                    version = itr->value.GetString();
                }
                _groups.push_back(group);
                _groupVer.emplace(group, version);
            }
        }
    }
    
    if ( json.HasMember(KEY_ENGINE_VERSION) && json[KEY_ENGINE_VERSION].IsString() )
    {
        _engineVer = json[KEY_ENGINE_VERSION].GetString();
    }
    
    if ( json.HasMember(KEY_UPDATING) && json[KEY_UPDATING].IsBool() )
    {
        _updating = json[KEY_UPDATING].GetBool();
    }
    
    _versionLoaded = true;
}

void Manifest::loadManifest(const rapidjson::Document &json)
{
    loadVersion(json);
    
    if ( json.HasMember(KEY_PACKAGE_URL) && json[KEY_PACKAGE_URL].IsString() )
    {
        if (_packageUrl == "") {
            _packageUrl = json[KEY_PACKAGE_URL].GetString();
        }
        if (_packageUrl.size() > 0 && _packageUrl[_packageUrl.size() - 1] != '/')
        {
            _packageUrl.append("/");
        }
    }
    
    if ( json.HasMember(KEY_ASSETS) )
    {
        const rapidjson::Value& assets = json[KEY_ASSETS];
        if (assets.IsObject())
        {
            for (rapidjson::Value::ConstMemberIterator itr = assets.MemberBegin(); itr != assets.MemberEnd(); ++itr)
            {
                std::string key = itr->name.GetString();
                Asset asset = parseAsset(key, itr->value);
                _assets.emplace(key, asset);
            }
        }
    }
    
    if ( json.HasMember(KEY_SEARCH_PATHS) )
    {
        const rapidjson::Value& paths = json[KEY_SEARCH_PATHS];
        if (paths.IsArray())
        {
            for (rapidjson::SizeType i = 0; i < paths.Size(); ++i)
            {
                if (paths[i].IsString()) {
                    _searchPaths.push_back(paths[i].GetString());
                }
            }
        }
    }
    
    if ( json.HasMember(KEY_HTTP_HOSTS)){
        const rapidjson::Value& hosts = json[KEY_HTTP_HOSTS];
        if (hosts.IsArray()) {
            for (rapidjson::SizeType i = 0; i < hosts.Size(); ++i) {
                if (hosts[i].IsString()) {
                    _httpHost.push_back(hosts[i].GetString());
                }
            }
        }
    }
    
    if ( json.HasMember(KEY_TCP_HOSTS)) {
        const rapidjson::Value& hosts = json[KEY_TCP_HOSTS];
        if (hosts.IsArray()) {
            for (rapidjson::SizeType i = 0; i < hosts.Size(); ++i) {
                if (hosts[i].IsString()) {
                    _wsHost.push_back(hosts[i].GetString());
                }
            }
        }
    }
    
    _loaded = true;
}
void Manifest::saveToFile(const std::string &filepath)
{
    if (_createManifest) {
    }
    saveToFileTest(filepath);
}
void Manifest::saveToFileTest(const std::string &filepath)
{
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    _json.Accept(writer);
    FileUtils::getInstance()->writeStringToFile(buffer.GetString(), filepath);
    
}
NS_CC_EXT_END
