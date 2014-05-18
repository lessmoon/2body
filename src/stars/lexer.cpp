#include "stdafx.h"
#include"lexer.h"
#include<cstdio>

#define CH_IS(C1,C2) ((C1)==(C2))
#define CH_IS_NOT(C1,C2) (!CH_IS(C1,C2))
#define HEX_CONV(C) ((C>='0'&&C<='9')?(C-'0'):((C>='a'&&C<='f')?(C-'a'+0xa):(C-'A'+0xa)))
#define OCT_CONV(C) ((C) - '0')
#define DO_OCT_CONV(C,peek) {C <<= 3; \
                             C |= OCT_CONV(peek);}

namespace lexer{
    const int MAX_LEXME_STR   =   0x100;
};

namespace {
    inline bool is_digit(const char c)
    {
        return (c >= '0' && c <= '9');
    }

    inline bool is_alpha(const char c)
    {
        return (c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z');
    }

    inline bool is_hex(const char c)
    {
        return is_digit(c)|| (c >= 'a' && c <= 'f');
    }

    inline bool is_oct(const char c)
    {
        return (c >= '0' && c <= '7');
    }
};

namespace lexer{
    int lexer::lineno = 1;

    lexer::lexer(iol_ptr sys_io)
    :peek(' '),sys_io(sys_io)
    {
        this -> reserve(new word("mass",tag::MASS));
        this -> reserve(new word("velocity",tag::VELOCITY));
        this -> reserve(new word("color",tag::COLOR));
        this -> reserve(new word("radius",tag::RADIUS));
        this -> reserve(new word("pos",tag::POS));
        this -> reserve(new word("zoom",tag::ZOOM));
        this -> reserve(new word("speed",tag::SPEED));
        this -> reserve(new word("center",tag::CENTER));
        this -> reserve(new word("planet",tag::PLANET));
        this -> reserve(new word("add",tag::ADD));
        this -> reserve(new word("set",tag::SET));
    }

    lexer::~lexer()
    {
        for(tab_iter iter = words.begin();
            iter != words.end();
            iter++)
            delete iter -> second;
        words.clear();
        storage.clear();
    }

    //stdz::string buf;
   // buf.reserve(MAX_LEXME_STR);

    bool lexer::check(const elem_t c)
    {
        return this -> readch(c);
    }

/*
 * NOTE: this function scan the input buffer and return the token
 * And because of the C++ no-auto do the clean job,So we should delete
 * the token(it is in the heap) manually.But I am working on the memory
 * manager.
 * TODO:create another token manager and do the collection job
 */
    lexer::token_ptr lexer::scan()
    {
        //num_t num_v = 0;
        real_t real_v = 0;
        real_t real_t = 0.1;
        int    e_      = 0;
        tab_iter iter;
        token_ptr tok = NULL;
        string buf;
        buf.reserve(10);

        for(;;readch()){
            if(CH_IS(peek,' ') || CH_IS(peek , '\t') || CH_IS(peek , '\r'))
                continue;
            else if(CH_IS(peek , '\n'))
                lineno++;
            else if(CH_IS(peek , '/'))
                if(check('*')){//for the block comment
                    readch();//Just because the peek is ' ' now...
                    do{
                        while(CH_IS_NOT(peek,'*')){
                            if(peek == '\n')
                                lineno++;
                            readch();
                        }
                        readch();
                    }while(CH_IS_NOT(peek,'/'));
                }
                else if(CH_IS(peek,'/')){//for the line comment
                    while(!check('\n'));
                    lineno++;
                }
                else
                    return new token('/');
            else
                break;
        }

        if(is_digit(peek)){
            int flag = true;

            do{
                real_v = real_v*10 + peek - '0';
                readch();
            }while(is_digit(peek));
            if(CH_IS_NOT(peek,'.')){
                flag = false;
            } else {
                readch();
                while(is_digit(peek)){
                    real_v += real_t*(peek - '0');
                    real_t /= 10;
                    readch();
                }
            }
            // if it has exponent
            if(CH_IS(peek,'e')|CH_IS(peek,'E')){
                readch();
                while(is_digit(peek)){
                    e_ = e_*10 + (peek - '0');
                    readch();
                }
            }
            for(int i = 0;i < e_;i++)
                    real_v *= 10;
            if(flag)
                return store(new real(real_v));
            else    
                return store(new num(real_v));
        }

        if(is_alpha(peek)||CH_IS(peek,'_')){
            do{
                buf += peek;
                readch();
            }while(is_alpha(peek)||is_digit(peek)||CH_IS(peek,'_'));
            iter = words.find(buf);
            if(iter != words.end())//If this word is in the table
                return iter -> second;
            return store(new token('\0'));
        }

        tok = new token(peek);peek = ' ';
        return store(tok);
    }

    void lexer::readch()
    {
        peek = sys_io -> readch();
    }

    bool lexer::readch(const char c)
    {
        this -> readch();
        if(CH_IS_NOT(peek,c))
            return false;
        peek = ' ';
        return true;
    }

    void lexer::reserve(word_ptr w)
    {
        words.insert(pair(w -> lexme,w));
        /*Put the word token into the table*/
    }
};//namespace lexer
