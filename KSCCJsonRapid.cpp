//
//  KSCCJsonRapid.cpp
//  DisasterGuy
//
//  Created by kisupark on 12. 11. 29..
//
//

#include "KSCCJsonRapid.h"

USING_NS_CC;
using namespace rapidjson;

NS_KSCC_BEGIN

#define kStrMax				1024*10
static char buf[kStrMax];

KSCCJsonRapid::~KSCCJsonRapid()
{
    
}


CCString* KSCCJsonRapid::jsonStringFromDictionary(CCDictionary *dic)
{
    Value *value = KSCCJsonRapid::jsonValueFromDictionary(dic);
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    value->Accept(writer);
    std::string jsonString = buffer.GetString();
	delete value;
    return CCString::create(jsonString);
}

CCString* KSCCJsonRapid::jsonStringFromArray(CCArray *arr)
{
    Value *value = KSCCJsonRapid::jsonValueFromArray(arr);
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    value->Accept(writer);
    std::string jsonString = buffer.GetString();
	delete value;
    return CCString::create(jsonString);
}

rapidjson::Value* KSCCJsonRapid::jsonValueFromArray(CCArray *arr, rapidjson::Document *document)
{
    Value *value = new Value(kArrayType);
	bool isAllocatorDocument = false;
	if (!document)
	{
		document = new Document();
		isAllocatorDocument = true;
	}

    for (unsigned int i=0; i<arr->count(); i++) {
        CCObject *obj = arr->objectAtIndex(i);
        CCString *stringVal = dynamic_cast<CCString *>(obj);
        CCArray *arrayVal = dynamic_cast<CCArray *>(obj);
        CCDictionary *dicVal = dynamic_cast<CCDictionary *>(obj);
		CCBool *boolVal = dynamic_cast<CCBool *>(obj);
        
        if (NULL != stringVal) {        // String
			Value *tempVal = KSCCJsonRapid::jsonValueFromString(stringVal, document);
			value->PushBack(tempVal, document->GetAllocator());
			delete tempVal;
        } else if (NULL != arrayVal) {  // Array
			Value *tempVal = KSCCJsonRapid::jsonValueFromArray(arrayVal, document);
			value->PushBack(tempVal, document->GetAllocator());
			delete tempVal;
        } else if (NULL != dicVal) {    // Object
			Value *tempVal = KSCCJsonRapid::jsonValueFromDictionary(dicVal, document);
			value->PushBack(tempVal, document->GetAllocator());
			delete tempVal;
        } else if (NULL != boolVal) {
			Value *tempVal = KSCCJsonRapid::jsonValueFromBool(boolVal, document);
			value->PushBack(tempVal, document->GetAllocator());
			delete tempVal;
		}
    }
	if (isAllocatorDocument)
	{
		delete document;
	}
	
    return value;
}

rapidjson::Value* KSCCJsonRapid::jsonValueFromDictionary(CCDictionary *dic, rapidjson::Document *document)
{
    Value *value = new Value(kObjectType);

	bool isAllocatorDocument = false;
	if (!document)
	{
		document = new Document();
		isAllocatorDocument = true;
	}
    
    CCDictElement *element = NULL;
    CCDICT_FOREACH(dic, element)
    {
        CCObject *obj = element->getObject();
        CCString *stringVal = dynamic_cast<CCString *>(obj);
        CCArray *arrayVal = dynamic_cast<CCArray *>(obj);
        CCDictionary *dicVal = dynamic_cast<CCDictionary *>(obj);
		CCBool *boolVal = dynamic_cast<CCBool *>(obj);
        
        if (KSCCJsonRapid::isString(obj)) {        // String
			Value *tempVal = KSCCJsonRapid::jsonValueFromString(stringVal, document);
			value->AddMember(element->getStrKey(), tempVal, document->GetAllocator());
			delete tempVal;
        } else if (NULL != arrayVal) {  // Array
			Value *tempVal = KSCCJsonRapid::jsonValueFromArray(arrayVal, document);
			value->AddMember(element->getStrKey(), tempVal, document->GetAllocator());
			delete tempVal;
        } else if (NULL != dicVal) {    // Object
			Value *tempVal = KSCCJsonRapid::jsonValueFromDictionary(dicVal, document);
			value->AddMember(element->getStrKey(), tempVal, document->GetAllocator());
			delete tempVal;
        } else if (NULL != boolVal) {
			Value *tempVal = KSCCJsonRapid::jsonValueFromBool(boolVal, document);
			value->AddMember(element->getStrKey(), tempVal, document->GetAllocator());
			delete tempVal;
		}
    }

	if (isAllocatorDocument)
	{
		delete document;
	}
    return value;
}

rapidjson::Value* KSCCJsonRapid::jsonValueFromString(CCString *str, rapidjson::Document *document)
{
	Value *value = new Value(str->getCString(), str->length());
    return value;
}

rapidjson::Value* KSCCJsonRapid::jsonValueFromBool(cocos2d::CCBool *boolObj, rapidjson::Document *document )
{
	Value *value = new Value(boolObj->getValue());
	return value;
}

cocos2d::CCObject* KSCCJsonRapid::objectFromJsonString(const std::string &jsonStr)
{
	std::string clearData(jsonStr);
	size_t pos = clearData.rfind("}");
	clearData = clearData.substr(0, pos+1);
	Document document;
	if( document.Parse<0>(clearData.c_str()).HasParseError())
	{
		CCLog(document.GetParseError());
		return NULL;
	}

    return KSCCJsonRapid::objectFromJsonValue(&document);
}

cocos2d::CCObject* KSCCJsonRapid::objectFromJsonString(CCString *jsonStr)
{
    return KSCCJsonRapid::objectFromJsonString(jsonStr->m_sString);
}

cocos2d::CCObject* KSCCJsonRapid::objectFromJsonValue(rapidjson::Value::ConstValueIterator valueIt)
{
    CCArray *arrayVal = NULL;
    CCDictionary *dicVal = NULL;
    
    if(valueIt->IsArray()) {
        arrayVal = new CCArray();
        arrayVal->init();
        arrayVal->autorelease();
    } else if (valueIt->IsObject()) {
        dicVal = new CCDictionary();
        dicVal->autorelease();
    }

    if (arrayVal) {
		int size = valueIt->Size();
		for (Value::ConstValueIterator itr = valueIt->Begin(); itr != valueIt->End(); ++itr) {
			if (itr->IsObject()) {
                arrayVal->addObject(KSCCJsonRapid::objectFromJsonValue(itr));
            } else if (itr->IsString()) {
                arrayVal->addObject(CCString::create(itr->GetString()));
            } else if (itr->IsArray()) {
                arrayVal->addObject(KSCCJsonRapid::objectFromJsonValue(itr));
            } else if (itr->IsInt()) {
				snprintf(buf, kStrMax, "%d", itr->GetInt());
                arrayVal->addObject(CCString::create(buf));
            } else if (itr->IsDouble()) {
				snprintf(buf, kStrMax, "%lf", itr->GetDouble());
				arrayVal->addObject(CCString::create(buf));
			} else if (itr->IsBool()) {
				arrayVal->addObject(CCBool::create(itr->GetBool()));
			}
		}
        return arrayVal;
    } else {
		Value::ConstMemberIterator itr = valueIt->MemberBegin();
        while (itr != valueIt->MemberEnd()) {
			if (itr->value.IsObject()) {
				CCObject *subObj = KSCCJsonRapid::objectFromJsonValue(&itr->value);
				dicVal->setObject(subObj, itr->name.GetString());
			} else if (itr->value.IsArray()) {
				CCObject *subObj = KSCCJsonRapid::objectFromJsonValue(&itr->value);
				dicVal->setObject(subObj, itr->name.GetString());
			} else if (itr->value.IsString()) {
				dicVal->setObject(CCString::create(itr->value.GetString()), itr->name.GetString());
			} else if (itr->value.IsInt()) {
				snprintf(buf, kStrMax, "%d", itr->value.GetInt());
				dicVal->setObject(CCString::create(buf), itr->name.GetString());
			} else if (itr->value.IsDouble()) {
				snprintf(buf, kStrMax, "%lf", itr->value.GetDouble());
				dicVal->setObject(CCString::create(buf), itr->name.GetString());
			} else if (itr->value.IsBool()) {
				dicVal->setObject(CCBool::create(itr->value.GetBool()), itr->name.GetString());
			}
            itr++;
        }
        
        if (NULL != dicVal) {
            return dicVal;
        }
    }
    return NULL;
}


bool KSCCJsonRapid::isString(cocos2d::CCObject *dest)
{
    CCString *str = dynamic_cast<CCString *>(dest);
    return (NULL != str);
}

bool KSCCJsonRapid::isDictionary(CCObject *dest)
{
    CCDictionary *dic = dynamic_cast<CCDictionary *>(dest);
    return (NULL != dic);
}

bool KSCCJsonRapid::isArray(CCObject *dest)
{
    CCArray *arr = dynamic_cast<CCArray *>(dest);
    return (NULL != arr);
}


NS_KSCC_END