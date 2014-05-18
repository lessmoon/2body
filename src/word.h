#ifndef __WORD_H
#define __WORD_H
#include<string>

#include"token.h"
#include"tag.h"

namespace lexer{
class word:public token{
    public:
        typedef word    self;
        typedef self*   self_ptr;
    public:
    word(const std::string&lexme,const int tag)
    :token(tag),lexme(lexme){}
    virtual ~word(){}

    public:
    std::string lexme;
};

};//namespace std

#endif//__WORD_H
