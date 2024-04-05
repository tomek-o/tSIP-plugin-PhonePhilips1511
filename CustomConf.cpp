#include "CustomConf.h"
#include <json/json.h>

namespace {
    enum { RING_TYPE_MAX = 5 }; // 0...5
}

CustomConf customConf;

CustomConf::CustomConf(void):
    ringType(0)
{

}

void CustomConf::toJson(Json::Value &jv) const
{
    jv = Json::Value(Json::objectValue);
    jv["ringType"] = ringType;
}

void CustomConf::fromJson(const Json::Value &jv)
{
    if (jv.type() != Json::objectValue)
        return;
    unsigned int tmp;
    jv.getUInt("ringType", tmp);
    if (tmp < RING_TYPE_MAX)
        ringType = tmp;
}
