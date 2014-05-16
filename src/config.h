#ifndef _CONFIG_H
#define _CONFIG_H

#include"lexer.h"

struct _3DVector{
    double x;
    double y;
    double z;
    double distance()const
    {
        return x*x + y*y + z*z;
    }

    const _3DVector&operator+=(const _3DVector&off)
    {
        x += off.x;
        y += off.y;
        z += off.z;
    }

    const _3DVector&operator-=(const _3DVector&off)
    {
        x -= off.x;
        y -= off.y;
        z -= off.z;
        return *this;
    }

    _3DVector(const double x = 0,const double y = 0,const double z = 0)
        :x(x),y(y),z(z){}
};

// CChildView ´°¿Ú

struct Star{
    double Mass;
    _3DVector V;
    int Color;
    double Radius;
    Star(){}

    Star(const double Mass,const _3DVector&V,const int Color,const double Radius = 2)
        :Mass(Mass),V(V),Color(Color),Radius(Radius){}
    void Set(const double Mass,const _3DVector&V,const int Color,const double Radius = 2)
    {
        this -> Mass = Mass;
        this -> V = V;
        this -> Color = Color;
        this ->Radius = Radius;
    }
};

struct Planet:public Star{
    _3DVector Pos;
    _3DVector Acc;
    _3DVector HalfPos;
    Planet()
    {}

    Planet(const double Mass,const _3DVector&V,_3DVector Pos,const int Color,const double Radius = 2)
        :Star(Mass,V,Color,Radius),Pos(Pos)
    {}

    void Set(const double Mass,const _3DVector&V,_3DVector Pos,const int Color,const double Radius = 2)
    {
        Star::Set(Mass,V,Color,Radius);
        this -> Pos = Pos;
    }
};

class config{
public:
    typedef  std::list<Planet> PContainer;
    typedef lexer::lexer lexer_t;
    typedef lexer::tag_t tag_t;
    typedef lexer::real::real_t real_t;
    typedef lexer::num::num_t num_t;
    typedef lexer::token    tok_t;
public:
    config(lexer_t* l)
    :l(l)
    {}

    ~config()
    {}

private:
    Star*       CenterStar;
    PContainer* Planets;
    int*        zoom;
    int*        internal_per_time;

public:
    void do_parse(    Star*       CenterStar,
                      PContainer* Planets,
                      int*        zoom,
                      int*        internal_per_time)
    {
        this ->CenterStar        = CenterStar;
        this ->Planets           = Planets;
        this ->internal_per_time = internal_per_time;
        this ->zoom              = zoom;
        this -> read();
        while(true){
            switch(look -> tag){
            case lexer::tag::ADD:
                this -> get_add();
                break;
            case lexer::tag::SET:
                this -> get_set();
                break;
            case -1:
                return;
            default:
                this -> error(25);
            }
        }
    }

private:
    void error(const int i){
        throw i;
    }

    void read()
    {
        look = l -> scan();
    }

    bool match(const tag_t i)
    {
        if(look -> tag == i ){
            this -> read();
            return true;
        } else {
            this -> error(15);
            return false;//won't happen
        }
    }

private:
    int get_rgb()
    {
        int rgb[3];
        match('<');
        for( int i = 0; i < 3 ; i++ ){
            if ( look -> tag == lexer::tag::NUM ){
                rgb[i] = static_cast<lexer::num*>(look) -> value;
            } else if ( look -> tag == lexer::tag::REAL ) {
                rgb[i] = static_cast<lexer::real*>(look) -> value;
            } else {
                this -> error(16);
            }
            this -> read();
            if(i < 2)
                match(',');
        }
        match('>');

        return (rgb[0]<<16)|(rgb[1]<<8)|rgb[2];
    }

    _3DVector get_3dvector()
    {
        double vec[3];
        _3DVector tmp;
        match('<');
        for( int i = 0; i < 3 ; i++ ){
            if ( look -> tag == lexer::tag::NUM ){
                vec[i] = static_cast<lexer::num*>(look) -> value;
            } else if ( look -> tag == lexer::tag::REAL ) {
                vec[i] = static_cast<lexer::real*>(look) -> value;
            } else {
                this -> error(16);
            }
            this -> read();
            if(i < 2)
                match(',');
        }
        match('>');
        tmp.x = vec[0];
        tmp.y = vec[1];
        tmp.z = vec[2];
        return tmp;
    }

    void get_config()
    {
        double value;
        _3DVector tmp;
        int color;
        lexer::tag_t name = look -> tag;
        switch(name){
        case lexer::tag::RADIUS:
        case lexer::tag::MASS:
        {
            this -> read();
            match('=');
            if ( look -> tag == lexer::tag::NUM ){
                value = static_cast<lexer::num*>(look) -> value;
            } else if ( look -> tag == lexer::tag::REAL ) {
                value = static_cast<lexer::real*>(look) -> value;
            } else {
                this -> error(16);
            }
            this -> read();
            break;
        }
        case lexer::tag::COLOR:
        {
            this -> read();
            match('=');
            color = this -> get_rgb();
            break;
        }
        case lexer::tag::VELOCITY:
        {
            this -> read();
            match('=');
            tmp = this -> get_3dvector();
            break;
        }
        default:
            this -> error(17);
        }
        switch(name){
        case lexer::tag::RADIUS:
            CenterStar -> Radius = value;
            break;
        case lexer::tag::MASS:
            CenterStar -> Mass = value;
            break;
        case lexer::tag::COLOR:
            CenterStar -> Color = color;
            break;
        case lexer::tag::VELOCITY:
            CenterStar -> V = tmp;
            break;
        default:
            this -> error(17);
        }
    }

    void get_config2(Planet&p)
    {
        double value;
        _3DVector tmp;
        int color = 0;
        lexer::tag_t name = look -> tag;
        switch(name){
        case lexer::tag::RADIUS:
        case lexer::tag::MASS:
        {
            this -> read();
            match('=');
            if ( look -> tag == lexer::tag::NUM ){
                value = static_cast<lexer::num*>(look) -> value;
            } else if ( look -> tag == lexer::tag::REAL ) {
                value = static_cast<lexer::real*>(look) -> value;
            } else {
                this -> error(16);
            }
            this -> read();
            break;
        }
        case lexer::tag::COLOR:
        {
            this -> read();
            match('=');
            color = this -> get_rgb();
            break;
        }
        case lexer::tag::POS:
        case lexer::tag::VELOCITY:
        {
            this -> read();
            match('=');
            tmp = this -> get_3dvector();
            break;
        }
        default:
            this -> error(17);
        }
        switch(name){
        case lexer::tag::RADIUS:
            p.Radius = value;
            break;
        case lexer::tag::MASS:
            p.Mass = value;
            break;
        case lexer::tag::COLOR:
            p.Color = color;
            break;
        case lexer::tag::POS:
            p.Pos = tmp;
            break;
        case lexer::tag::VELOCITY:
            p.V = tmp;
            break;
        default:
            this -> error(17);
        }
    }

    void get_configlist()
    {
        match('<');
        while(true){
            this -> get_config();
            if(look -> tag != ','){
                break;
            } else {
                this -> read();
            }
        }
        match('>');
    }

    void get_configlist2()
    {
        Planet tmp;
        match('<');
        while(true){
            this -> get_config2(tmp);
            if(look -> tag != ','){
                break;
            } else {
                this -> read();
            }
        }
        match('>');
        Planets -> push_back(tmp);
    }

    void get_set()
    {
        double value;
        match(lexer::tag::SET);
        lexer::tag_t name = look -> tag;
        switch(name){
        case lexer::tag::ZOOM:
        case lexer::tag::SPEED:
        {
            this -> read();
            this -> match('=');
            if ( look -> tag == lexer::tag::NUM ){
                value = static_cast<lexer::num*>(look) -> value;
            } else if ( look -> tag == lexer::tag::REAL ) {
                value = static_cast<lexer::num*>(look) -> value;
            } else {
                this -> error(16);
            }
            this -> read();
            break;
        }
        case lexer::tag::CENTER:
        {
            this -> read();
            this -> match('=');
            this -> get_configlist();
            break;
        }
        default:
            this -> error(19);
        }

        if(name == lexer::tag::SPEED){
            *internal_per_time = value;
        } else if (name == lexer::tag::ZOOM){
            *zoom = value;
        }
    }

    void get_add()
    {
        match(lexer::tag::ADD);
        switch(look -> tag){
        case lexer::tag::PLANET:
        {
            this -> read();
            this -> match('=');
            this -> get_configlist2();
            break;
        }
        default:
            this -> error(20);
        }
    }
private:
    tok_t*          look;
    lexer_t*        l;

};//class config

#endif // _CONFIG_H
