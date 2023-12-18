#include<string>
#include<iostream>
#include<exception>

#include<cstdint>
#include<random>

struct blob
{
    bool friendly=false;
    double home_x=0.0;
    double home_y=0.0;

    blob(bool Friend,double _x,double _y);
};
