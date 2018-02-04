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

#include "stdafx.h"
#include "JSON_serializer.h"


#if JSON_SERIALIZER_ENABLE

std::string JSONSerializer::SimpleTypeToJSON( serialisableObject_t *pObj, const serializableField_t *pf, int idx )
{
    char buf[2048];

    std::string res;
    switch (pf->type)
    {
    case TYPE_FLOAT:
        sprintf( buf, "%5.4f", *(float*)(((u8*)pObj)+pf->offset)+idx );
        res += buf;
        break;
    case TYPE_INT:
        sprintf( buf, "%d", *(int*)(((u8*)pObj)+pf->offset)+idx );
        res += buf;
        break;
    case TYPE_U32:
        sprintf( buf, "\"0x%x\"", *(u32*)(((u8*)pObj)+pf->offset)+idx );
        res += buf;
        break;
    case TYPE_STRING:
        res += "\"";
        res += *((std::string*)(((u8*)pObj)+pf->offset)+idx);
        res += "\"";
        break;
    case TYPE_BOOLEAN:
        res += *(bool*)(((u8*)pObj)+pf->offset)?"true":"false";
        break;
    case TYPE_VECT:
        {
            const vec_t *pv = (vec_t*)(((u8*)pObj)+pf->offset)+idx;
            sprintf( buf, "\"%5.4f,%5.4f,%5.4f,%5.4f\"", pv->x, pv->y, pv->z, pv->w );
            res += buf;
        }
        break;
    case TYPE_MATRIX:
        {
            const matrix_t *pm = (matrix_t*)(((u8*)pObj)+pf->offset)+idx;
			res+="\"";
			for(int dmi = 0;dmi<16;dmi++)
			{
				sprintf( buf, dmi?",%5.4f":"%5.4f", pm->m16[dmi] );
				res+=buf;
			}
            res+="\"";
        }
        break;
    case TYPE_OBJECT:
        {
            serialisableObject_t *memberObject = (serialisableObject_t*)(((u8*)pObj)+pf->offset+idx);
            res += GenerateJSON( memberObject );
        }
        break;
    case TYPE_OBJECT_PTR:
        {
            serialisableObject_t *memberObject = *(serialisableObject_t**)(((u8*)pObj)+pf->offset+idx);
            res += memberObject ? GenerateJSON( memberObject ) : "null";
        }
        break;
    default:
        break;
    }
    return res;
}

std::string JSONSerializer::GenerateJSON( serialisableObject_t *pObj )
{
    std::string res="{\n";

    const serializableField_t *pf = pObj->GetFields();
    for (int i=0;i<pObj->GetNbFields();i++, pf++)
    {
        if ( pf->flags & SF_TRANSIENT )
            continue;

        char buf[2048];
        setlocale(LC_ALL, "C");

        sprintf( buf, "%s\"%s\":", i?",\n":"", pf->name);
        res += buf;

        if (pf->container)
        {
            void *myContainer = (((u8*)pObj)+pf->offset);
            int count = pf->container->GetCount( myContainer );
            res += "[";
            for ( int j = 0 ; j < count ; j ++)
            {
                serialisableObject_t *nextObject = (serialisableObject_t *)pf->container->GetValue( myContainer, j );
                if (j)
                    res +=",\n";

                if (pf->type == TYPE_UNDEFINED)
                {
                    res += GenerateJSON( nextObject );
                }
                else
                {
                    res += SimpleTypeToJSON( pObj, pf, j );
                }
            }
            res += "]";
        }
        else
        {
            res += SimpleTypeToJSON( pObj, pf, 0 );
        }
    }

    res += "\n}\n";
    return res;
}

#define JSONASSIGNTYPE(sertype, ctype, value) if ( currentJSONState->pf && currentJSONState->pf->type == sertype ) {\
    *((ctype*) currentJSONState->pRawData) = value;\
    currentJSONState->pRawData += sizeof(ctype);\
}
//else LOG( "JSON Parser : can't affect "QUOTE(type)" to %s::%s.\n", currentJSONState->pObj->GetTypeName(), currentJSONState->pf->name );

int JSONSerializer::JSONSerializerCallback(int type, const JSON_value* value)
{
    switch(type)
    {
    case JSON_T_ARRAY_BEGIN:        break;
    case JSON_T_ARRAY_END:          break;
    case JSON_T_OBJECT_BEGIN:
        {
            serialisableObject_t* pObj;
            if (!currentJSONState)
                pObj = mRootObject;
            else if ( currentJSONState->pf->type == TYPE_OBJECT_PTR)
                pObj = *(serialisableObject_t**)currentJSONState->pRawData;
            else
                pObj = (serialisableObject_t*)currentJSONState->pRawData;
            //printf("\n %s pushed\n", pObj->GetTypeName() );

            serializableContainer_t *pCont = NULL;
            if ( currentJSONState )
            {
                pCont = currentJSONState->container;
                if ( pCont )
                    pObj = ((serialisableObject_t *)pCont->Push( currentJSONState->pRawData, currentJSONState->containerCurrentIndex++ ));
            }
            currentJSONState = mJSONObjectsPool.Push();
            currentJSONState->pObj = pObj;
        }
        break;
    case JSON_T_OBJECT_END:
        currentJSONState->pObj->FieldsHasBeenModified();
        mJSONObjectsPool.Pop();
        currentJSONState = &mJSONObjectsPool[mJSONObjectsPool.Size()-1];
        break;
    case JSON_T_KEY:
        currentJSONState->pf = currentJSONState->pObj->GetFieldByName( value->vu.str.value );
        //printf("   %s\n", value->vu.str.value );
        //ASSERT( currentJSONState->pf ); // printf(" NO %s::%s \n", currentJSONState->pObj->GetTypeName(), value->vu.str.value );
        if ( currentJSONState->pf )
        {
            currentJSONState->container = currentJSONState->pf->container;
            currentJSONState->containerCurrentIndex = 0;
            currentJSONState->pRawData = (((u8*)currentJSONState->pObj)+currentJSONState->pf->offset);
        }
        break;
    case JSON_T_INTEGER:
    case JSON_T_FLOAT:
            if ( !currentJSONState->pf )
                break;
        if ( currentJSONState->pf->type == TYPE_FLOAT )
        {
            *((float*) currentJSONState->pRawData) = (type==JSON_T_INTEGER)?(float)value->vu.integer_value:(float)value->vu.float_value;
            currentJSONState->pRawData += sizeof(float);
        }
        else if ( currentJSONState->pf->type == TYPE_INT )
        {
            *((int*) currentJSONState->pRawData) = (type==JSON_T_INTEGER)?value->vu.integer_value:(int)value->vu.float_value;
            currentJSONState->pRawData += sizeof(int);
        }
        else
            LOG( "JSON Parser : can't affect numerical value to %s::%s.\n", currentJSONState->pObj->GetTypeName(), currentJSONState->pf->name );
        break;
    case JSON_T_NULL:       JSONASSIGNTYPE(TYPE_OBJECT_PTR,serialisableObject_t *,NULL);    break;
    case JSON_T_TRUE:       JSONASSIGNTYPE(TYPE_BOOLEAN,bool,true);                            break;
    case JSON_T_FALSE:      JSONASSIGNTYPE(TYPE_BOOLEAN,bool,false);                           break;
    case JSON_T_STRING:
		if ( currentJSONState->pf->type == TYPE_MATRIX )
        {
            matrix_t val;
            sscanf( value->vu.str.value, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", 
				&val.m16[0], &val.m16[1], &val.m16[2], &val.m16[3],
				&val.m16[4], &val.m16[5], &val.m16[6], &val.m16[7],
				&val.m16[8], &val.m16[9], &val.m16[10], &val.m16[11],
				&val.m16[12], &val.m16[13], &val.m16[14], &val.m16[15] );
            *((matrix_t*) currentJSONState->pRawData) = val;
            currentJSONState->pRawData += sizeof(matrix_t);
        }
		else if ( currentJSONState->pf->type == TYPE_VECT )
        {
            vec_t val;
            sscanf( value->vu.str.value, "%f,%f,%f,%f", &val.x, &val.y, &val.z, &val.w );
            *((vec_t*) currentJSONState->pRawData) = val;
            currentJSONState->pRawData += sizeof(vec_t);
        }
        else
            if ( currentJSONState->pf->type == TYPE_U32 )
            {
                sscanf( value->vu.str.value, "0x%x", ((u32*) currentJSONState->pRawData) );
                currentJSONState->pRawData += sizeof(float);
            }
            else
            {
                JSONASSIGNTYPE(TYPE_STRING,std::string,value->vu.str.value);
            }
            break;
    default:                ASSERT_GAME(0);                                                      break;
    }
    return 1;
}

void JSONSerializer::ParseJSON( serialisableObject_t *pObj, const char *szJSONText, int JSONTextLength )
{
    currentJSONState = NULL;
    mRootObject = pObj;

    struct JSON_parser_struct* jc = new_JSON_parser(&JSONSerializerConfigParser);
    // parse
    for (int i=0;i<JSONTextLength;i++)
    {
        if (!JSON_parser_char(jc, szJSONText[i]))
        {
            LOG("JSON_parser_char: syntax error, byte %d\n", i);
            goto done;
        }
    }

    // done
    if (!JSON_parser_done(jc))
        LOG("JSON_parser_end: syntax error\n");
done:
    delete_JSON_parser(jc);
}

#endif  //  JSON_SERIALIZER_ENABLE
