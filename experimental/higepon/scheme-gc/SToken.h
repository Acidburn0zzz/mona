#ifndef __STOKEN_H__
#define __STOKEN_H__

#include "util/String.h"

namespace monash {

class SToken
{
public:
    SToken(int type);
    virtual ~SToken();
    ::util::String valueString();
    ::util::String typeString();

    enum
    {
        COMMENT,
        LEFT_PAREN,
        RIGHT_PAREN,
        IDENTIFIER,
        KEYWORD,
        VARIABLE,
        BOOLEAN,
        NUMBER,
        CHARCTER,
        STRING,
        VECTOR_START,
        SINGLE_QUOTE,
        BACK_QUOTE,
        CAMMA,
        CAMMA_AT,
        PERIOD
    };

    int type;
    int integer;
    ::util::String text;
};

}; // namespace monash
#endif // __STOKEN_H__
