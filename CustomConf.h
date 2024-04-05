#ifndef CustomConfH
#define CustomConfH

namespace Json
{
    class Value;
}

struct CustomConf
{
    unsigned int ringType;
    CustomConf(void);
    void toJson(Json::Value &jv) const;
    void fromJson(const Json::Value &jv);
};

extern CustomConf customConf;

#endif // CustomConfH
