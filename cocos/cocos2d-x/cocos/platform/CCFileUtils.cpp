/****************************************************************************
Copyright (c) 2010-2013 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#include "platform/CCFileUtils.h"

#include <stack>

#include "base/CCData.h"
#include "base/ccMacros.h"
#include "platform/CCSAXParser.h"

#include "tinyxml2/tinyxml2.h"
#include "tinydir/tinydir.h"

#ifdef MINIZIP_FROM_SYSTEM
#include <minizip/unzip.h>
#else // from our embedded sources
#include "unzip/unzip.h"
#endif
#include <sys/stat.h>
#include <regex>

NS_CC_BEGIN

#define BUFFER_SIZE    8192
#define MAX_FILENAME   512


#if (CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_MAC)

typedef enum
{
    SAX_NONE = 0,
    SAX_KEY,
    SAX_DICT,
    SAX_INT,
    SAX_REAL,
    SAX_STRING,
    SAX_ARRAY
}SAXState;

typedef enum
{
    SAX_RESULT_NONE = 0,
    SAX_RESULT_DICT,
    SAX_RESULT_ARRAY
}SAXResult;

class DictMaker : public SAXDelegator
{
public:
    SAXResult _resultType;
    ValueMap _rootDict;
    ValueVector _rootArray;

    std::string _curKey;   ///< parsed key
    std::string _curValue; // parsed value
    SAXState _state;

    ValueMap*  _curDict;
    ValueVector* _curArray;

    std::stack<ValueMap*> _dictStack;
    std::stack<ValueVector*> _arrayStack;
    std::stack<SAXState>  _stateStack;

public:
    DictMaker()
        : _resultType(SAX_RESULT_NONE)
        , _state(SAX_NONE)
    {
    }

    ~DictMaker()
    {
    }

    ValueMap dictionaryWithContentsOfFile(const std::string& fileName)
    {
        _resultType = SAX_RESULT_DICT;
        SAXParser parser;

        CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
        parser.setDelegator(this);

        parser.parse(fileName);
        return _rootDict;
    }

    ValueMap dictionaryWithDataOfFile(const char* filedata, int filesize)
    {
        _resultType = SAX_RESULT_DICT;
        SAXParser parser;

        CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
        parser.setDelegator(this);

        parser.parse(filedata, filesize);
        return _rootDict;
    }

    ValueVector arrayWithContentsOfFile(const std::string& fileName)
    {
        _resultType = SAX_RESULT_ARRAY;
        SAXParser parser;

        CCASSERT(parser.init("UTF-8"), "The file format isn't UTF-8");
        parser.setDelegator(this);

        parser.parse(fileName);
        return _rootArray;
    }

    virtual void startElement(void *ctx, const char *name, const char **atts) override
    {
        CC_UNUSED_PARAM(ctx);
        CC_UNUSED_PARAM(atts);
        const std::string sName(name);
        if( sName == "dict" )
        {
            if(_resultType == SAX_RESULT_DICT && _rootDict.empty())
            {
                _curDict = &_rootDict;
            }

            _state = SAX_DICT;

            SAXState preState = SAX_NONE;
            if (! _stateStack.empty())
            {
                preState = _stateStack.top();
            }

            if (SAX_ARRAY == preState)
            {
                _curArray->push_back(Value(ValueMap()));
                _curDict = &(_curArray->rbegin())->asValueMap();
            }
            else if (SAX_DICT == preState)
            {
                CCASSERT(! _dictStack.empty(), "The state is wrong!");
                ValueMap* preDict = _dictStack.top();
                (*preDict)[_curKey] = Value(ValueMap());
                _curDict = &(*preDict)[_curKey].asValueMap();
            }

            _stateStack.push(_state);
            _dictStack.push(_curDict);
        }
        else if(sName == "key")
        {
            _state = SAX_KEY;
        }
        else if(sName == "integer")
        {
            _state = SAX_INT;
        }
        else if(sName == "real")
        {
            _state = SAX_REAL;
        }
        else if(sName == "string")
        {
            _state = SAX_STRING;
        }
        else if (sName == "array")
        {
            _state = SAX_ARRAY;

            if (_resultType == SAX_RESULT_ARRAY && _rootArray.empty())
            {
                _curArray = &_rootArray;
            }
            SAXState preState = SAX_NONE;
            if (! _stateStack.empty())
            {
                preState = _stateStack.top();
            }

            if (preState == SAX_DICT)
            {
                (*_curDict)[_curKey] = Value(ValueVector());
                _curArray = &(*_curDict)[_curKey].asValueVector();
            }
            else if (preState == SAX_ARRAY)
            {
                CCASSERT(! _arrayStack.empty(), "The state is wrong!");
                ValueVector* preArray = _arrayStack.top();
                preArray->push_back(Value(ValueVector()));
                _curArray = &(_curArray->rbegin())->asValueVector();
            }
            _stateStack.push(_state);
            _arrayStack.push(_curArray);
        }
        else
        {
            _state = SAX_NONE;
        }
    }

    virtual void endElement(void *ctx, const char *name) override
    {
        CC_UNUSED_PARAM(ctx);
        SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
        const std::string sName((char*)name);
        if( sName == "dict" )
        {
            _stateStack.pop();
            _dictStack.pop();
            if ( !_dictStack.empty())
            {
                _curDict = _dictStack.top();
            }
        }
        else if (sName == "array")
        {
            _stateStack.pop();
            _arrayStack.pop();
            if (! _arrayStack.empty())
            {
                _curArray = _arrayStack.top();
            }
        }
        else if (sName == "true")
        {
            if (SAX_ARRAY == curState)
            {
                _curArray->push_back(Value(true));
            }
            else if (SAX_DICT == curState)
            {
                (*_curDict)[_curKey] = Value(true);
            }
        }
        else if (sName == "false")
        {
            if (SAX_ARRAY == curState)
            {
                _curArray->push_back(Value(false));
            }
            else if (SAX_DICT == curState)
            {
                (*_curDict)[_curKey] = Value(false);
            }
        }
        else if (sName == "string" || sName == "integer" || sName == "real")
        {
            if (SAX_ARRAY == curState)
            {
                if (sName == "string")
                    _curArray->push_back(Value(_curValue));
                else if (sName == "integer")
                    _curArray->push_back(Value(atoi(_curValue.c_str())));
                else
                    _curArray->push_back(Value(std::atof(_curValue.c_str())));
            }
            else if (SAX_DICT == curState)
            {
                if (sName == "string")
                    (*_curDict)[_curKey] = Value(_curValue);
                else if (sName == "integer")
                    (*_curDict)[_curKey] = Value(atoi(_curValue.c_str()));
                else
                    (*_curDict)[_curKey] = Value(std::atof(_curValue.c_str()));
            }

            _curValue.clear();
        }

        _state = SAX_NONE;
    }

    virtual void textHandler(void *ctx, const char *ch, int len) override
    {
        CC_UNUSED_PARAM(ctx);
        if (_state == SAX_NONE)
        {
            return;
        }

        SAXState curState = _stateStack.empty() ? SAX_DICT : _stateStack.top();
        const std::string text = std::string((char*)ch,len);

        switch(_state)
        {
        case SAX_KEY:
            _curKey = text;
            break;
        case SAX_INT:
        case SAX_REAL:
        case SAX_STRING:
            {
                if (curState == SAX_DICT)
                {
                    CCASSERT(!_curKey.empty(), "key not found : <integer/real>");
                }

                _curValue.append(text);
            }
            break;
        default:
            break;
        }
    }
};

ValueMap FileUtils::getValueMapFromFile(const std::string& filename)
{
    const std::string fullPath = fullPathForFilename(filename);
    if (fullPath.empty())
    {
        ValueMap ret;
        return ret;
    }

    DictMaker tMaker;
    return tMaker.dictionaryWithContentsOfFile(fullPath);
}

ValueMap FileUtils::getValueMapFromData(const char* filedata, int filesize)
{
    DictMaker tMaker;
    return tMaker.dictionaryWithDataOfFile(filedata, filesize);
}

ValueVector FileUtils::getValueVectorFromFile(const std::string& filename)
{
    const std::string fullPath = fullPathForFilename(filename);
    DictMaker tMaker;
    return tMaker.arrayWithContentsOfFile(fullPath);
}


/*
 * forward statement
 */
static tinyxml2::XMLElement* generateElementForArray(const ValueVector& array, tinyxml2::XMLDocument *doc);
static tinyxml2::XMLElement* generateElementForDict(const ValueMap& dict, tinyxml2::XMLDocument *doc);

/*
 * Use tinyxml2 to write plist files
 */
bool FileUtils::writeToFile(const ValueMap& dict, const std::string &fullPath)
{
    return writeValueMapToFile(dict, fullPath);
}

bool FileUtils::writeValueMapToFile(const ValueMap& dict, const std::string& fullPath)
{
    tinyxml2::XMLDocument *doc = new (std::nothrow)tinyxml2::XMLDocument();
    if (nullptr == doc)
        return false;

    tinyxml2::XMLDeclaration *declaration = doc->NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    if (nullptr == declaration)
    {
        delete doc;
        return false;
    }

    doc->LinkEndChild(declaration);
    tinyxml2::XMLElement *docType = doc->NewElement("!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"");
    doc->LinkEndChild(docType);

    tinyxml2::XMLElement *rootEle = doc->NewElement("plist");
    if (nullptr == rootEle)
    {
        delete doc;
        return false;
    }
    rootEle->SetAttribute("version", "1.0");
    doc->LinkEndChild(rootEle);

    tinyxml2::XMLElement *innerDict = generateElementForDict(dict, doc);
    if (nullptr == innerDict)
    {
        delete doc;
        return false;
    }
    rootEle->LinkEndChild(innerDict);

    bool ret = tinyxml2::XML_SUCCESS == doc->SaveFile(getSuitableFOpen(fullPath).c_str());

    delete doc;
    return ret;
}

bool FileUtils::writeValueVectorToFile(const ValueVector& vecData, const std::string& fullPath)
{
    tinyxml2::XMLDocument *doc = new (std::nothrow)tinyxml2::XMLDocument();
    if (nullptr == doc)
        return false;

    tinyxml2::XMLDeclaration *declaration = doc->NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    if (nullptr == declaration)
    {
        delete doc;
        return false;
    }

    doc->LinkEndChild(declaration);
    tinyxml2::XMLElement *docType = doc->NewElement("!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\"");
    doc->LinkEndChild(docType);

    tinyxml2::XMLElement *rootEle = doc->NewElement("plist");
    if (nullptr == rootEle)
    {
        delete doc;
        return false;
    }
    rootEle->SetAttribute("version", "1.0");
    doc->LinkEndChild(rootEle);

    tinyxml2::XMLElement *innerDict = generateElementForArray(vecData, doc);
    if (nullptr == innerDict)
    {
        delete doc;
        return false;
    }
    rootEle->LinkEndChild(innerDict);

    bool ret = tinyxml2::XML_SUCCESS == doc->SaveFile(getSuitableFOpen(fullPath).c_str());

    delete doc;
    return ret;
}

/*
 * Generate tinyxml2::XMLElement for Object through a tinyxml2::XMLDocument
 */
static tinyxml2::XMLElement* generateElementForObject(const Value& value, tinyxml2::XMLDocument *doc)
{
    if (value.getType() == Value::Type::STRING)
    {
        tinyxml2::XMLElement* node = doc->NewElement("string");
        tinyxml2::XMLText* content = doc->NewText(value.asString().c_str());
        node->LinkEndChild(content);
        return node;
    }

    if (value.getType() == Value::Type::INTEGER)
    {
        tinyxml2::XMLElement* node = doc->NewElement("integer");
        tinyxml2::XMLText* content = doc->NewText(value.asString().c_str());
        node->LinkEndChild(content);
        return node;
    }

    if (value.getType() == Value::Type::FLOAT || value.getType() == Value::Type::DOUBLE)
    {
        tinyxml2::XMLElement* node = doc->NewElement("real");
        tinyxml2::XMLText* content = doc->NewText(value.asString().c_str());
        node->LinkEndChild(content);
        return node;
    }

    if (value.getType() == Value::Type::BOOLEAN) {
        tinyxml2::XMLElement* node = doc->NewElement(value.asString().c_str());
        return node;
    }

    if (value.getType() == Value::Type::VECTOR)
        return generateElementForArray(value.asValueVector(), doc);

    if (value.getType() == Value::Type::MAP)
        return generateElementForDict(value.asValueMap(), doc);

    CCLOG("This type cannot appear in property list");
    return nullptr;
}

/*
 * Generate tinyxml2::XMLElement for Dictionary through a tinyxml2::XMLDocument
 */
static tinyxml2::XMLElement* generateElementForDict(const ValueMap& dict, tinyxml2::XMLDocument *doc)
{
    tinyxml2::XMLElement* rootNode = doc->NewElement("dict");

    for (const auto &iter : dict)
    {
        tinyxml2::XMLElement* tmpNode = doc->NewElement("key");
        rootNode->LinkEndChild(tmpNode);
        tinyxml2::XMLText* content = doc->NewText(iter.first.c_str());
        tmpNode->LinkEndChild(content);

        tinyxml2::XMLElement *element = generateElementForObject(iter.second, doc);
        if (element)
            rootNode->LinkEndChild(element);
    }
    return rootNode;
}

/*
 * Generate tinyxml2::XMLElement for Array through a tinyxml2::XMLDocument
 */
static tinyxml2::XMLElement* generateElementForArray(const ValueVector& array, tinyxml2::XMLDocument *pDoc)
{
    tinyxml2::XMLElement* rootNode = pDoc->NewElement("array");

    for(const auto &value : array) {
        tinyxml2::XMLElement *element = generateElementForObject(value, pDoc);
        if (element)
            rootNode->LinkEndChild(element);
    }
    return rootNode;
}

#else

/* The subclass FileUtilsApple should override these two method. */
ValueMap FileUtils::getValueMapFromFile(const std::string& filename) {return ValueMap();}
ValueMap FileUtils::getValueMapFromData(const char* filedata, int filesize) {return ValueMap();}
ValueVector FileUtils::getValueVectorFromFile(const std::string& filename) {return ValueVector();}
bool FileUtils::writeToFile(const ValueMap& dict, const std::string &fullPath) {return false;}

#endif /* (CC_TARGET_PLATFORM != CC_PLATFORM_IOS) && (CC_TARGET_PLATFORM != CC_PLATFORM_MAC) */

FileUtils* FileUtils::s_sharedFileUtils = nullptr;

void FileUtils::destroyInstance()
{
    CC_SAFE_DELETE(s_sharedFileUtils);
}

void FileUtils::setDelegate(FileUtils *delegate)
{
    if (s_sharedFileUtils)
        delete s_sharedFileUtils;

    s_sharedFileUtils = delegate;
}

FileUtils::FileUtils()
    : _writablePath("")
{
}

FileUtils::~FileUtils()
{
}

bool FileUtils::writeStringToFile(const std::string& dataStr, const std::string& fullPath)
{
    Data data;
    data.fastSet((unsigned char*)dataStr.c_str(), dataStr.size());

    bool rv = writeDataToFile(data, fullPath);

    data.takeBuffer();
    return rv;
}

bool FileUtils::writeDataToFile(const Data& data, const std::string& fullPath)
{
    size_t size = 0;
    const char* mode = "wb";

    CCASSERT(!fullPath.empty() && data.getSize() != 0, "Invalid parameters.");

    auto fileutils = FileUtils::getInstance();
    do
    {
        FILE *fp = fopen(fileutils->getSuitableFOpen(fullPath).c_str(), mode);
        CC_BREAK_IF(!fp);
        size = data.getSize();

        fwrite(data.getBytes(), size, 1, fp);

        fclose(fp);

        return true;
    } while (0);

    return false;
}

bool FileUtils::init()
{
    _searchPathArray.push_back(_defaultResRootPath);
    _searchResolutionsOrderArray.push_back("");
    return true;
}

void FileUtils::purgeCachedEntries()
{
    _fullPathCache.clear();
}

std::string FileUtils::getStringFromFile(const std::string& filename)
{
    std::string s;
    getContents(filename, &s);
    return s;
}

Data FileUtils::getDataFromFile(const std::string& filename)
{
    Data d;
    getContents(filename, &d);
    return d;
}


FileUtils::Status FileUtils::getContents(const std::string& filename, ResizableBuffer* buffer)
{
    if (filename.empty())
        return Status::NotExists;

    auto fs = FileUtils::getInstance();

    std::string fullPath = fs->fullPathForFilename(filename);
    if (fullPath.empty())
        return Status::NotExists;

    FILE *fp = fopen(fs->getSuitableFOpen(fullPath).c_str(), "rb");
    if (!fp)
        return Status::OpenFailed;

#if defined(_MSC_VER)
    auto descriptor = _fileno(fp);
#else
    auto descriptor = fileno(fp);
#endif
    struct stat statBuf;
    if (fstat(descriptor, &statBuf) == -1) {
        fclose(fp);
        return Status::ReadFailed;
    }
    size_t size = statBuf.st_size;

    buffer->resize(size);
    size_t readsize = fread(buffer->buffer(), 1, size, fp);
    fclose(fp);

    if (readsize < size) {
        buffer->resize(readsize);
        return Status::ReadFailed;
    }

    return Status::OK;
}

unsigned char* FileUtils::getFileData(const std::string& filename, const char* mode, ssize_t *size)
{
    CCASSERT(!filename.empty() && size != nullptr && mode != nullptr, "Invalid parameters.");
    (void)(mode); // mode is unused, as we do not support text mode any more...

    Data d;
    if (getContents(filename, &d) != Status::OK) {
        *size = 0;
        return nullptr;
    }

    return d.takeBuffer(size);
}

unsigned char* FileUtils::getFileDataFromZip(const std::string& zipFilePath, const std::string& filename, ssize_t *size)
{
    unsigned char * buffer = nullptr;
    unzFile file = nullptr;
    *size = 0;

    do
    {
        CC_BREAK_IF(zipFilePath.empty());

        file = unzOpen(FileUtils::getInstance()->getSuitableFOpen(zipFilePath).c_str());
        CC_BREAK_IF(!file);

        #ifdef MINIZIP_FROM_SYSTEM
        int ret = unzLocateFile(file, filename.c_str(), NULL);
        #else
        int ret = unzLocateFile(file, filename.c_str(), 1);
        #endif
        CC_BREAK_IF(UNZ_OK != ret);

        char filePathA[260];
        unz_file_info fileInfo;
        ret = unzGetCurrentFileInfo(file, &fileInfo, filePathA, sizeof(filePathA), nullptr, 0, nullptr, 0);
        CC_BREAK_IF(UNZ_OK != ret);

        ret = unzOpenCurrentFile(file);
        CC_BREAK_IF(UNZ_OK != ret);

        buffer = (unsigned char*)malloc(fileInfo.uncompressed_size);
        int CC_UNUSED readedSize = unzReadCurrentFile(file, buffer, static_cast<unsigned>(fileInfo.uncompressed_size));
        CCASSERT(readedSize == 0 || readedSize == (int)fileInfo.uncompressed_size, "the file size is wrong");

        *size = fileInfo.uncompressed_size;
        unzCloseCurrentFile(file);
    } while (0);

    if (file)
    {
        unzClose(file);
    }

    return buffer;
}

std::string FileUtils::getNewFilename(const std::string &filename) const
{
    std::string newFileName;

    auto iter = _filenameLookupDict.find(filename);

    if (iter == _filenameLookupDict.end())
    {
        newFileName = filename;
    }
    else
    {
        newFileName = iter->second.asString();
    }
    return newFileName;
}

std::string FileUtils::getPathForFilename(const std::string& filename, const std::string& resolutionDirectory, const std::string& searchPath) const
{
    std::string file = filename;
    std::string file_path = "";
    size_t pos = filename.find_last_of("/");
    if (pos != std::string::npos)
    {
        file_path = filename.substr(0, pos+1);
        file = filename.substr(pos+1);
    }

    std::string path = searchPath;
    path += file_path;
    path += resolutionDirectory;

    path = getFullPathForDirectoryAndFilename(path, file);

    return path;
}

std::string FileUtils::fullPathForFilename(const std::string &filename) const
{
    if (filename.empty())
    {
        return "";
    }

    if (isAbsolutePath(filename))
    {
        return normalizePath(filename);
    }

    auto cacheIter = _fullPathCache.find(filename);
    if(cacheIter != _fullPathCache.end())
    {
        return cacheIter->second;
    }

    const std::string newFilename( getNewFilename(filename) );

    std::string fullpath;

    for (const auto& searchIt : _searchPathArray)
    {
        for (const auto& resolutionIt : _searchResolutionsOrderArray)
        {
            fullpath = this->getPathForFilename(newFilename, resolutionIt, searchIt);

            if (!fullpath.empty())
            {
                _fullPathCache.insert(std::make_pair(filename, fullpath));
                return fullpath;
            }
        }
    }

    if(isPopupNotify()){
        CCLOG("fullPathForFilename: No file found at %s. Possible missing file.", filename.c_str());
    }

    return "";
}

std::string FileUtils::fullPathFromRelativeFile(const std::string &filename, const std::string &relativeFile)
{
    return relativeFile.substr(0, relativeFile.rfind('/')+1) + getNewFilename(filename);
}

void FileUtils::setSearchResolutionsOrder(const std::vector<std::string>& searchResolutionsOrder)
{
    if (_searchResolutionsOrderArray == searchResolutionsOrder)
    {
        return;
    }

    bool existDefault = false;
    _fullPathCache.clear();
    _searchResolutionsOrderArray.clear();
    for(const auto& iter : searchResolutionsOrder)
    {
        std::string resolutionDirectory = iter;
        if (!existDefault && resolutionDirectory == "")
        {
            existDefault = true;
        }

        if (resolutionDirectory.length() > 0 && resolutionDirectory[resolutionDirectory.length()-1] != '/')
        {
            resolutionDirectory += "/";
        }

        _searchResolutionsOrderArray.push_back(resolutionDirectory);
    }

    if (!existDefault)
    {
        _searchResolutionsOrderArray.push_back("");
    }
}

void FileUtils::addSearchResolutionsOrder(const std::string &order,const bool front)
{
    std::string resOrder = order;
    if (!resOrder.empty() && resOrder[resOrder.length()-1] != '/')
        resOrder.append("/");

    if (front) {
        _searchResolutionsOrderArray.insert(_searchResolutionsOrderArray.begin(), resOrder);
    } else {
        _searchResolutionsOrderArray.push_back(resOrder);
    }
}

const std::vector<std::string>& FileUtils::getSearchResolutionsOrder() const
{
    return _searchResolutionsOrderArray;
}

const std::vector<std::string>& FileUtils::getSearchPaths() const
{
    return _searchPathArray;
}

const std::vector<std::string>& FileUtils::getOriginalSearchPaths() const
{
    return _originalSearchPaths;
}

void FileUtils::setWritablePath(const std::string& writablePath)
{
    _writablePath = writablePath;
}

const std::string& FileUtils::getDefaultResourceRootPath() const
{
    return _defaultResRootPath;
}

void FileUtils::setDefaultResourceRootPath(const std::string& path)
{
    if (_defaultResRootPath != path)
    {
        _fullPathCache.clear();
        _defaultResRootPath = path;
        if (!_defaultResRootPath.empty() && _defaultResRootPath[_defaultResRootPath.length()-1] != '/')
        {
            _defaultResRootPath += '/';
        }

        setSearchPaths(_originalSearchPaths);
    }
}

void FileUtils::setSearchPaths(const std::vector<std::string>& searchPaths)
{
    bool existDefaultRootPath = false;
    _originalSearchPaths = searchPaths;

    _fullPathCache.clear();
    _searchPathArray.clear();
    
    for (const auto& path : _originalSearchPaths)
    {
        std::string prefix;
        std::string fullPath;

        if (!isAbsolutePath(path))
        { // Not an absolute path
            prefix = _defaultResRootPath;
        }
        fullPath = prefix + path;
        if (!path.empty() && path[path.length()-1] != '/')
        {
            fullPath += "/";
        }
        if (!existDefaultRootPath && path == _defaultResRootPath)
        {
            existDefaultRootPath = true;
        }
        long nRet = std::count(_searchPathArray.begin(), _searchPathArray.end(), fullPath);
        if (nRet <= 0) {
            _searchPathArray.push_back(fullPath);
            cocos2d::log("FileUtils setSearchPaths push back: %s", fullPath.c_str());
        }
    }

    if (!existDefaultRootPath)
    {
        _searchPathArray.push_back(_defaultResRootPath);
        cocos2d::log("FileUtils setSearchPaths  push _defaultResRootPath: %s", _defaultResRootPath.c_str());
    }
}

void FileUtils::addSearchPath(const std::string &searchpath,const bool front)
{
    std::string prefix;
    if (!isAbsolutePath(searchpath))
        prefix = _defaultResRootPath;

    std::string path = prefix + searchpath;
    if (!path.empty() && path[path.length()-1] != '/')
    {
        path += "/";
    }
    if (front) {
        cocos2d::log("FileUtils addSearchPath push_begin: %s", searchpath.c_str());
        _originalSearchPaths.insert(_originalSearchPaths.begin(), searchpath);
        _searchPathArray.insert(_searchPathArray.begin(), path);
    } else {
        cocos2d::log("FileUtils addSearchPath push_back: %s", searchpath.c_str());
        _originalSearchPaths.push_back(searchpath);
        _searchPathArray.push_back(path);
    }
}

void FileUtils::setFilenameLookupDictionary(const ValueMap& filenameLookupDict)
{
    _fullPathCache.clear();
    _filenameLookupDict = filenameLookupDict;
}

void FileUtils::loadFilenameLookupDictionaryFromFile(const std::string &filename)
{
    const std::string fullPath = fullPathForFilename(filename);
    if (!fullPath.empty())
    {
        ValueMap dict = FileUtils::getInstance()->getValueMapFromFile(fullPath);
        if (!dict.empty())
        {
            ValueMap& metadata =  dict["metadata"].asValueMap();
            int version = metadata["version"].asInt();
            if (version != 1)
            {
                CCLOG("ERROR: Invalid filenameLookup dictionary version: %d. Filename: %s", version, filename.c_str());
                return;
            }
            setFilenameLookupDictionary( dict["filenames"].asValueMap());
        }
    }
}

std::string FileUtils::getFullPathForDirectoryAndFilename(const std::string& directory, const std::string& filename) const
{
    std::string ret = directory;
    if (directory.size() && directory[directory.size()-1] != '/'){
        ret += '/';
    }
    ret += filename;
    ret = normalizePath(ret);

    if (!isFileExistInternal(ret)) {
        ret = "";
    }
    return ret;
}

bool FileUtils::isFileExist(const std::string& filename) const
{
    if (isAbsolutePath(filename))
    {
        return isFileExistInternal(normalizePath(filename));
    }
    else
    {
        std::string fullpath = fullPathForFilename(filename);
        if (fullpath.empty())
            return false;
        else
            return true;
    }
}

bool FileUtils::isAbsolutePath(const std::string& path) const
{
    return (path[0] == '/');
}

bool FileUtils::isDirectoryExist(const std::string& dirPath) const
{
    CCASSERT(!dirPath.empty(), "Invalid path");

    if (isAbsolutePath(dirPath))
    {
        return isDirectoryExistInternal(normalizePath(dirPath));
    }

    auto cacheIter = _fullPathCache.find(dirPath);
    if( cacheIter != _fullPathCache.end() )
    {
        return isDirectoryExistInternal(cacheIter->second);
    }

    std::string fullpath;
    for (const auto& searchIt : _searchPathArray)
    {
        for (const auto& resolutionIt : _searchResolutionsOrderArray)
        {
            fullpath = fullPathForFilename(searchIt + dirPath + resolutionIt);
            if (isDirectoryExistInternal(fullpath))
            {
                _fullPathCache.insert(std::make_pair(dirPath, fullpath));
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> FileUtils::listFiles(const std::string& dirPath) const
{
    std::string fullpath = fullPathForFilename(dirPath);
    std::vector<std::string> files;
    if (isDirectoryExist(fullpath))
    {
        tinydir_dir dir;
#ifdef UNICODE
        unsigned int length = MultiByteToWideChar(CP_UTF8, 0, &fullpath[0], (int)fullpath.size(), NULL, 0);
        if (length != fullpath.size())
        {
            return files;
        }
        std::wstring fullpathstr(length, 0);
        MultiByteToWideChar(CP_UTF8, 0, &fullpath[0], (int)fullpath.size(), &fullpathstr[0], length);
#else
        std::string fullpathstr = fullpath;
#endif
        if (tinydir_open(&dir, &fullpathstr[0]) != -1)
        {
            while (dir.has_next)
            {
                tinydir_file file;
                if (tinydir_readfile(&dir, &file) == -1)
                {
                    break;
                }
                
#ifdef UNICODE
                std::wstring path = file.path;
                length = WideCharToMultiByte(CP_UTF8, 0, &path[0], (int)path.size(), NULL, 0, NULL, NULL);
                std::string filepath;
                if (length > 0)
                {
                    filepath.resize(length);
                    WideCharToMultiByte(CP_UTF8, 0, &path[0], (int)path.size(), &filepath[0], length, NULL, NULL);
                }
#else
                std::string filepath = file.path;
#endif
                if (file.is_dir)
                {
                    filepath.append("/");
                }
                files.push_back(filepath);
                
                if (tinydir_next(&dir) == -1)
                {
                    break;
                }
            }
        }
        tinydir_close(&dir);
    }
    return files;
}

void FileUtils::listFilesRecursively(const std::string& dirPath, std::vector<std::string> *files) const
{
    std::string fullpath = fullPathForFilename(dirPath);
    if (isDirectoryExist(fullpath))
    {
        tinydir_dir dir;
#ifdef UNICODE
        unsigned int length = MultiByteToWideChar(CP_UTF8, 0, &fullpath[0], (int)fullpath.size(), NULL, 0);
        if (length != fullpath.size())
        {
            return;
        }
        std::wstring fullpathstr(length, 0);
        MultiByteToWideChar(CP_UTF8, 0, &fullpath[0], (int)fullpath.size(), &fullpathstr[0], length);
#else
        std::string fullpathstr = fullpath;
#endif
        if (tinydir_open(&dir, &fullpathstr[0]) != -1)
        {
            while (dir.has_next)
            {
                tinydir_file file;
                if (tinydir_readfile(&dir, &file) == -1)
                {
                    break;
                }

#ifdef UNICODE
                std::wstring path = file.path;
                length = WideCharToMultiByte(CP_UTF8, 0, &path[0], (int)path.size(), NULL, 0, NULL, NULL);
                std::string filepath;
                if (length > 0)
                {
                    filepath.resize(length);
                    WideCharToMultiByte(CP_UTF8, 0, &path[0], (int)path.size(), &filepath[0], length, NULL, NULL);
                }
#else
                std::string filepath = file.path;
#endif
                if (file.name[0] != '.')
                {
                    if (file.is_dir)
                    {
                        filepath.append("/");
                        files->push_back(filepath);
                        listFilesRecursively(filepath, files);
                    }
                    else
                    {
                        files->push_back(filepath);
                    }
                }
                
                if (tinydir_next(&dir) == -1)
                {
                    break;
                }
            }
        }
        tinydir_close(&dir);
    }
}

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
bool FileUtils::isDirectoryExistInternal(const std::string& dirPath) const
{
    CCASSERT(false, "FileUtils not support isDirectoryExistInternal");
    return false;
}

bool FileUtils::createDirectory(const std::string& path)
{
    CCASSERT(false, "FileUtils not support createDirectory");
    return false;
}

bool FileUtils::removeDirectory(const std::string& path)
{
    CCASSERT(false, "FileUtils not support removeDirectory");
    return false;
}

bool FileUtils::removeFile(const std::string &path)
{
    CCASSERT(false, "FileUtils not support removeFile");
    return false;
}

bool FileUtils::renameFile(const std::string &oldfullpath, const std::string& newfullpath)
{
    CCASSERT(false, "FileUtils not support renameFile");
    return false;
}

bool FileUtils::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    CCASSERT(false, "FileUtils not support renameFile");
    return false;
}

std::string FileUtils::getSuitableFOpen(const std::string& filenameUtf8) const
{
    CCASSERT(false, "getSuitableFOpen should be override by platform FileUtils");
    return filenameUtf8;
}

long FileUtils::getFileSize(const std::string &filepath)
{
    CCASSERT(false, "getFileSize should be override by platform FileUtils");
    return 0;
}

#else
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
#include <ftw.h>
#endif

bool FileUtils::isDirectoryExistInternal(const std::string& dirPath) const
{
    struct stat st;
    if (stat(dirPath.c_str(), &st) == 0)
    {
        return S_ISDIR(st.st_mode);
    }
    return false;
}

bool FileUtils::createDirectory(const std::string& path)
{
    CCASSERT(!path.empty(), "Invalid path");

    if (isDirectoryExist(path))
        return true;

    size_t start = 0;
    size_t found = path.find_first_of("/\\", start);
    std::string subpath;
    std::vector<std::string> dirs;

    if (found != std::string::npos)
    {
        while (true)
        {
            subpath = path.substr(start, found - start + 1);
            if (!subpath.empty())
                dirs.push_back(subpath);
            start = found+1;
            found = path.find_first_of("/\\", start);
            if (found == std::string::npos)
            {
                if (start < path.length())
                {
                    dirs.push_back(path.substr(start));
                }
                break;
            }
        }
    }

    DIR *dir = NULL;

    subpath = "";
    for (const auto& iter : dirs)
    {
        subpath += iter;
        dir = opendir(subpath.c_str());

        if (!dir)
        {

            int ret = mkdir(subpath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            if (ret != 0 && (errno != EEXIST))
            {
                return false;
            }
        }
        else
        {
            closedir(dir);
        }
    }
    return true;
}

namespace
{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
    int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
    {
        int rv = remove(fpath);
        
        if (rv)
            perror(fpath);
        
        return rv;
    }
#endif
}

bool FileUtils::removeDirectory(const std::string& path)
{
#if !defined(CC_TARGET_OS_TVOS)

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)
    if (nftw(path.c_str(), unlink_cb, 64, FTW_DEPTH | FTW_PHYS) == -1)
        return false;
    else
        return true;
#else
    std::string command = "rm -r ";
    command += "\"" + path + "\"";
    if (system(command.c_str()) >= 0)
        return true;
    else
        return false;
#endif // (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)

#else
    return false;
#endif // !defined(CC_TARGET_OS_TVOS)
}

bool FileUtils::removeFile(const std::string &path)
{
    if (remove(path.c_str())) {
        return false;
    } else {
        return true;
    }
}

bool FileUtils::renameFile(const std::string &oldfullpath, const std::string &newfullpath)
{
    CCASSERT(!oldfullpath.empty(), "Invalid path");
    CCASSERT(!newfullpath.empty(), "Invalid path");

    int errorCode = rename(oldfullpath.c_str(), newfullpath.c_str());

    if (0 != errorCode)
    {
        CCLOGERROR("Fail to rename file %s to %s !Error code is %d", oldfullpath.c_str(), newfullpath.c_str(), errorCode);
        return false;
    }
    return true;
}

bool FileUtils::renameFile(const std::string &path, const std::string &oldname, const std::string &name)
{
    CCASSERT(!path.empty(), "Invalid path");
    std::string oldPath = path + oldname;
    std::string newPath = path + name;

    return this->renameFile(oldPath, newPath);
}

std::string FileUtils::getSuitableFOpen(const std::string& filenameUtf8) const
{
    return filenameUtf8;
}


long FileUtils::getFileSize(const std::string &filepath)
{
    CCASSERT(!filepath.empty(), "Invalid path");

    std::string fullpath = filepath;
    if (!isAbsolutePath(filepath))
    {
        fullpath = fullPathForFilename(filepath);
        if (fullpath.empty())
            return 0;
    }

    struct stat info;
    int result = stat(fullpath.c_str(), &info);

    if (result != 0)
    {
        return -1;
    }
    else
    {
        return (long)(info.st_size);
    }
}
#endif


/* Default to false, enable it by setPopupNotify if needed */
static bool s_popupNotify = false;

void FileUtils::setPopupNotify(bool notify)
{
    s_popupNotify = notify;
}

bool FileUtils::isPopupNotify() const
{
    return s_popupNotify;
}

std::string FileUtils::getFileExtension(const std::string& filePath) const
{
    std::string fileExtension;
    size_t pos = filePath.find_last_of('.');
    if (pos != std::string::npos)
    {
        fileExtension = filePath.substr(pos, filePath.length());

        std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
    }

    return fileExtension;
}

void FileUtils::valueMapCompact(ValueMap &valueMap)
{
}

void FileUtils::valueVectorCompact(ValueVector &valueVector)
{
}

std::string FileUtils::getFileDir(const std::string& path) const
{
    std::string ret;
    size_t pos = path.rfind("/");
    if (pos != std::string::npos)
    {
        ret = path.substr(0, pos);
    }

    normalizePath(ret);

    return ret;
}

std::string FileUtils::normalizePath(const std::string& path) const
{
    std::string ret;
    ret = std::regex_replace(path, std::regex("/\\./"), "/");
    ret = std::regex_replace(ret, std::regex("/\\.$"), "");

    size_t pos;
    while ((pos = ret.find("..")) != std::string::npos && pos > 2)
    {
        size_t prevSlash = ret.rfind("/", pos-2);
        if (prevSlash == std::string::npos)
            break;

        ret = ret.replace(prevSlash, pos - prevSlash + 2 , "");
    }
    return ret;
}

bool FileUtils::loadZIP(const std::string &zipFilename, const std::string &targetPath, const std::string &password/*""*/)
{
    std::string filename = zipFilename;
    std::string dataFilePath = FileUtils::getInstance()->getWritablePath() + filename;
    
    std::string strPath = FileUtils::getInstance()->fullPathForFilename(filename);
    Data data;
    
    data = FileUtils::getInstance()->getDataFromFile(strPath.c_str());
    
    
    bool isCopyToTarget = this->writeDataToFile(data, targetPath);
    if (!isCopyToTarget) {
        
    }
    
    return true;
}

bool FileUtils::decompress(const std::string &zip, std::string unzipPath, const std::string &password)
{
    
    size_t pos = zip.find_last_of("/\\");
    if (pos == std::string::npos)
    {
        CCLOG("FileUtils : no root path specified for zip file %s\n", zip.c_str());
        return false;
    }
    std::string zip_file_name = zip.substr(pos+1, zip.length()-1);
    size_t pos_zip = zip_file_name.find_last_of(".");
    if (pos_zip != std::string::npos) {
        zip_file_name = zip_file_name.substr(0, pos_zip);
    }
    
    if (!unzipPath.empty() && unzipPath[unzipPath.length()-1] != '/')
    {
        unzipPath += "/";
    }
    
    unzFile zipfile = unzOpen(FileUtils::getInstance()->getSuitableFOpen(zip).c_str());
    if (! zipfile)
    {
        CCLOG("FileUtils : can not open downloaded zip file %s\n", zip.c_str());
        return false;
    }
    
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
    {
        CCLOG("FileUtils : can not read file global info of %s\n", zip.c_str());
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
            CCLOG("FileUtils : can not read compressed file info\n");
            unzClose(zipfile);
            return false;
        }
        std::string res_fileName = fileName;
        if(fileName!=zip_file_name) {
            res_fileName = res_fileName.substr(zip_file_name.length(), res_fileName.length()-1);
            strcpy(fileName, res_fileName.c_str());
        }
        
        const std::string fullPath = unzipPath + fileName;
        
        const size_t filenameLength = strlen(fileName);
        if (fileName[filenameLength-1] == '/')
        {
            if ( !this->createDirectory(fuBasename(fullPath)) )
            {
                CCLOG("FileUtils : can not create directory %s\n", fullPath.c_str());
                unzClose(zipfile);
                return false;
            }
        }
        else
        {
            std::string dir = fuBasename(fullPath);
            if (!this->isDirectoryExist(dir)) {
                if (!this->createDirectory(dir)) {
                    CCLOG("FileUtils : can not create directory %s\n", fullPath.c_str());
                    unzClose(zipfile);
                    return false;
                }
            }
            
            if (password.empty()) {
                if (unzOpenCurrentFile(zipfile) != UNZ_OK)
                {
                    CCLOG("FileUtils : can not extract file %s\n", fileName);
                    unzClose(zipfile);
                    return false;
                }
            } else {
                if (unzOpenCurrentFilePassword(zipfile,password.c_str())!=UNZ_OK)
                {
                    CCLOG("can not open file %s", fileName);
                    unzClose(zipfile);
                    return false;
                }
            }
            
            FILE *out = fopen(FileUtils::getInstance()->getSuitableFOpen(fullPath).c_str(), "wb");
            if (!out)
            {
                CCLOG("FileUtils : can not create decompress destination file %s (errno: %d)\n", fullPath.c_str(), errno);
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
                    CCLOG("FileUtils : can not read zip file %s, error code is %d\n", fileName, error);
                    fclose(out);
                    unzCloseCurrentFile(zipfile);
                    unzClose(zipfile);
                    return false;
                }
                
                if (error > 0)
                {
                    fwrite(readBuffer, error, 1, out);
                }
            } while(error > 0);
            
            fclose(out);
        }
        
        unzCloseCurrentFile(zipfile);
        
        if ((i+1) < global_info.number_entry)
        {
            if (unzGoToNextFile(zipfile) != UNZ_OK)
            {
                CCLOG("FileUtils : can not read next file for decompressing\n");
                unzClose(zipfile);
                return false;
            }
        }
    }
    unzClose(zipfile);
    
    return true;
}

std::string FileUtils::fuBasename(const std::string& path) const
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

NS_CC_END
