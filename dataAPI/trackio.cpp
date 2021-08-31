/*********************************************************************************
*     File Name           :     trackio.cpp
*     Created By          :     Xi Yangyang
*     Modified By         :     Liu Yinglun
*     Creation Date       :     [2017-01-20]
*     Last Modified       :     [2017-06-23]
*     Description         :     Implementation of trackio.h
**********************************************************************************/
#include <QDebug>
#include "trackio.h"
#include <sstream>
#include <fstream>
#include <iostream>

using namespace std;

map<TrackIdx, vector<FrameIdx> > MOD_STORE;
vector<TrackIdx> activated_tracks;
vector<FrameIdx> activation_starts;
/*
 * function created by liuyinglun
 * fill MOD_STORE with all loaded data
 */
void aggregate_mod_store(Frames& frames){

    MOD_STORE.clear();
    size_t len = frames.size();
    for(unsigned int i=0; i < len; ++i){
        Tracks1Frame::iterator it;
        Tracks1Frame& tracks1frame = frames[i];
        for(it=tracks1frame.begin(); it!=tracks1frame.end(); ++it){
            Object trackrect = it->second;
            FrameIdx fid = trackrect.frame_id;
            TrackIdx tid = trackrect.track_id;
            MOD_STORE[tid].push_back(fid);
        }
    }
}

bool loadLabels(const std::string filename, Frames& frames) {
    std::ifstream f(filename.c_str());
    if (!f.is_open()) {
        std::cerr << "fail to open label file: " << filename << std::endl;
        return false;
    }

    std::string line;
    while (getline(f, line)) {
        std::istringstream iss(line);
        FrameIdx fid;
        TrackIdx tid;
        int label;
        float confidence;
        double x, y, w, h;
        iss >> fid;
        if (fid < 0)
        {
            std::cerr << "fid is bellow to 0." << std::endl;
            return false;
        }
        if (fid >= frames.size()) {
            std::cerr << "fid is beyond frame_count." << std::endl;
            return false;
        }
        iss >> tid;
        if (tid < 0)
        {
            std::cerr << "tid is bellow to 0." << std::endl;
            return false;
        }
//        iss >> label;
//        iss >> confidence;

        iss >> x;
        iss >> y;
        iss >> w;
        iss >> h;

        iss >> confidence;
        iss >> label;

        Rect<double> bbox;
        bbox.left = double(x);
        bbox.top = double(y);
        bbox.width = double(w);
        bbox.height = double(h);

        frames[fid][tid] = Object(label, confidence, fid, tid, bbox);
    }
    aggregate_mod_store(frames);
    return true;
}

bool saveLabels(const std::string filename, Frames& frames){
    std::ofstream f(filename.c_str());
    if(!f.is_open()){
        std::cerr << "fail to open file: " << filename << std::endl;
        return false;
    }
    for(size_t i=0; i<frames.size(); ++i){
        Tracks1Frame& tracks1frame = frames[i];
        for(Tracks1Frame::iterator it=tracks1frame.begin(); it!=tracks1frame.end(); ++it){

            Object trackrect = it->second;

            char linebuf[255];

            Rect<int> rt = (Rect<int>)trackrect.rect;
            int label = trackrect.label;
            float confidence = trackrect.confidence;
            FrameIdx fid = trackrect.frame_id;
            TrackIdx tid = trackrect.track_id;

            int x = rt.left;
            int y = rt.top;
            int w = rt.width;
            int h = rt.height;

            sprintf(linebuf, "%d %d %d %d %d %d %f %d\n", fid, tid, x, y, w, h, confidence, label);
            f << linebuf;
        }
    }
    return true;
}
