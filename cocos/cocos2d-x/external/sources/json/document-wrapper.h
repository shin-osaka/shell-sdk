#pragma once



#include "json/rapidjson.h"

#if defined(__arm__) && defined(RAPIDJSON_GNUC) && RAPIDJSON_GNUC >= RAPIDJSON_VERSION_CODE(4,9,0) && RAPIDJSON_GNUC < RAPIDJSON_VERSION_CODE(5,0,0)
    #pragma GCC optimize ("O1")
#endif    
#include "document.h"
#if defined(__arm__) && defined(RAPIDJSON_GNUC) && RAPIDJSON_GNUC >= RAPIDJSON_VERSION_CODE(4,9,0) && RAPIDJSON_GNUC < RAPIDJSON_VERSION_CODE(5,0,0)
    #pragma GCC reset_options
#endif
