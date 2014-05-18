#ifndef _TAG_H
#define _TAG_H
    namespace lexer{
        typedef  int  tag_t;
        namespace tag{
            const tag_t     MASS = 256,VELOCITY = 257,COLOR = 258,
                            ID = 259,RADIUS = 260,POS = 261,ZOOM = 262,
                            SPEED = 263,CENTER = 264,PLANET = 265,
                            ADD = 266,SET = 267,REAL = 268,NUM = 269;
        };
    };
#endif // _TAG_H
