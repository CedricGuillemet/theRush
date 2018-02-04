///////////////////////////////////////////////////////////////////////////////////////////////////
//  
//      __   __                              __    
//     |  |_|  |--.-----. .----.--.--.-----.|  |--.
//   __|   _|     |  -__| |   _|  |  |__ --||     |
//  |__|____|__|__|_____| |__| |_____|_____||__|__|
//                                                 
//  Copyright (C) 2007-2013 Cedric Guillemet
//
// This file is part of .the rush//.
//
//    .the rush// is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    .the rush// is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with .the rush//.  If not, see <http://www.gnu.org/licenses/>
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef JSON_SERIALIZER_H__
#define JSON_SERIALIZER_H__


#define JSON_SERIALIZER_ENABLE  1

#if JSON_SERIALIZER_ENABLE

#include "JSON_parser.h"
#include "toolbox.h"

class JSONSerializer
{
public:
    JSONSerializer()
    {
        init_JSON_config(&JSONSerializerConfigParser);
        JSONSerializerConfigParser.depth                  = 19;
        JSONSerializerConfigParser.callback               = &StaticJSONSerializerCallback;
        JSONSerializerConfigParser.allow_comments         = 1;
        JSONSerializerConfigParser.handle_floats_manually = 0;
        JSONSerializerConfigParser.callback_ctx           = this;
    }

    ~JSONSerializer() {}

    // JSON Text to runtime objects
    void ParseJSON( serialisableObject_t *pObj, const char *szJSONText, int JSONTextLength );

    // runtime objects to JSON Text
    std::string GenerateJSON( serialisableObject_t *pObj );

private:
    // current state management
    typedef struct jsonObjectState_t
    {
        serialisableObject_t* pObj;
        u8 *pRawData;
        const serializableField_t *pf;
        serializableContainer_t *container;
        int containerCurrentIndex;
    } jsonObjectState_t;

    FastArray<jsonObjectState_t, 20> mJSONObjectsPool;
    jsonObjectState_t *currentJSONState;

    // json parser confi
    JSON_config JSONSerializerConfigParser;

    // rool object, only used once
    serialisableObject_t *mRootObject;

    // Callback
    static int StaticJSONSerializerCallback(void* ctx, int type, const JSON_value* value)
    {
        return ((JSONSerializer*)ctx)->JSONSerializerCallback( type, value );
    }
    int JSONSerializerCallback(int type, const JSON_value* value);
    std::string SimpleTypeToJSON( serialisableObject_t *pObj, const serializableField_t *pf, int idx );
};
#endif  //  JSON_SERIALIZER_ENABLE

#endif
