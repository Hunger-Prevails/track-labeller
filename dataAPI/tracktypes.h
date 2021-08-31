/*********************************************************************************
*     File Name           :     tracktypes.h
*     Created By          :     Xi Yangyang
*     Creation Date       :     [2017-01-20]
*     Last Modified       :     [2017-02-03]
*     Description         :     Define the primary data structure
**********************************************************************************/
#ifndef __SENSETIME_TRACKTYPE_H__
#define __SENSETIME_TRACKTYPE_H__

#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <assert.h>
#define IN
#define OUT

typedef int Label;
typedef int FrameIdx;
typedef int TrackIdx;
typedef std::string VipName;

template <typename T>
struct Rect{
    T left;
    T top;
    T width;
    T height;
    Rect():left(T(0)), top(T(0)), width(T(0)), height(T(0)){}
    Rect(T x, T y, T w, T h): left(x), top(y), width(w), height(h){}

    template<typename T1>
    Rect<T> operator + (const Rect<T1> rct) const{
        Rect<T> ret;
        ret.left    = (T)(this->left + (T)rct.left);
        ret.top     = (T)(this->top + (T)rct.top);
        ret.width   = (T)(this->width + (T)rct.width);
        ret.height  = (T)(this->height + (T)rct.height);
        return ret;
    }

    template<typename T1>
    Rect<T> operator - (const Rect<T1> rct) const{
        Rect<T> ret;
        ret.left    = (T)(this->left - (T)rct.left);
        ret.top     = (T)(this->top - (T)rct.top);
        ret.width   = (T)(this->width - (T)rct.width);
        ret.height  = (T)(this->height - (T)rct.height);
        return ret;
    }

    template<typename T1>
    Rect<T> operator * (const T1 scale) const{
        Rect<T> ret;
        ret.width   = (T)(this->width * T(scale));
        ret.height  = (T)(this->height * T(scale));
        return ret;
    }

    template<typename T1>
    Rect<T> operator / (const T1 scale) const{
        Rect<T> ret;
        ret.width   = (T)(this->width) / T(scale);
        ret.height  = (T)(this->height) / T(scale);
        return ret;
    }


    operator Rect<int> () {
        Rect<int> ret;
        ret.left    = (int)(this->left + 0.5);
        ret.top     = (int)(this->top + 0.5);
        ret.width   = (int)(this->width + 0.5);
        ret.height  = (int)(this->height + 0.5);
        return ret;
    }

    template<typename T2>
    Rect<T>& operator = (const Rect<T2>& t2rct) {
    	if (this == &t2rct)
    		return *this;
    	this->left    = (T)(t2rct.left);
    	this->top     = (T)(t2rct.top);
    	this->width   = (T)(t2rct.width);
    	this->height  = (T)(t2rct.height);
        return *this;
    }
};
template<typename T>
bool operator == (const Rect<T> & lrct, const Rect<T> & rrct) {
    if(lrct.left==rrct.left && lrct.top==rrct.top && lrct.width==rrct.width && lrct.height==rrct.height){
        return true;
    }
    else{
        return false;
    }
}

template<typename T>
bool operator != (const Rect<T> & lrct, const Rect<T> & rrct) {
    return !(lrct==rrct);
}

template<typename T>
void operator << (std::ostream& os, const Rect<T> & rct) {
    os << "[ ";
    os << rct.left << " ";
    os << rct.top << " ";
    os << rct.width << " ";
    os << rct.height << " ";
    os << "]" << std::endl;
}

struct Object {
	FrameIdx frame_id;
	TrackIdx track_id;
	Label label;
    Rect<double> rect;
    float confidence;

    explicit Object(){}

    explicit Object(Label l, float conf, FrameIdx f_idx, TrackIdx t_idx, Rect<double> r):
        frame_id(f_idx), track_id(t_idx), rect(r), label(l), confidence(conf){}
};
typedef std::map<TrackIdx, Object> Tracks1Frame; // tracks in one frame. map for fast locate trackIdx

typedef std::vector<Tracks1Frame> Frames; // all frames

#endif
