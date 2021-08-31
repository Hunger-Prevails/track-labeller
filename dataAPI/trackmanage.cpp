/*********************************************************************************
*     File Name     :   trackmanage.cpp
*     Created By    :   Xi Yangyang
*     Modified By   :   Liu Yinglun
*     Creation Date :   [2017-01-20]
*     Last Modified :   [2017-06-25]
*     Description   :   Implementation of trackmanage
**********************************************************************************/

#include "tracktypes.h"
#include "trackmanage.h"
#include <assert.h>
#include <vector>
#include <algorithm>
#include <QDebug>

using namespace std;

extern map<TrackIdx, vector<FrameIdx>> MOD_STORE;
extern vector<TrackIdx> activated_tracks;
extern vector<FrameIdx> activation_starts;
extern int frame_interval;

FrameIdx find_last_frame(TrackIdx tid, FrameIdx modified_frame){

    if (MOD_STORE.find(tid) == MOD_STORE.end())
        return -1;

    for(int i=frame_interval; i>0; i--){
        vector<FrameIdx>::iterator it = find(MOD_STORE[tid].begin(), MOD_STORE[tid].end(), modified_frame - i);
        if(it != MOD_STORE[tid].end())
            return modified_frame - i;
    }
    return -1;
}

FrameIdx find_next_frame(TrackIdx tid, FrameIdx modified_frame){
    if (MOD_STORE.find(tid) == MOD_STORE.end())
        return -1;
    if (MOD_STORE[tid].size() <= 1)
        return -1;

    for(int i=frame_interval; i>0; i--){
        vector<FrameIdx>::iterator it = find(MOD_STORE[tid].begin(), MOD_STORE[tid].end(), modified_frame + i);
        if(it != MOD_STORE[tid].end())
            return modified_frame + i;
    }
    return -1;
}

Rect<double> trackInterp_1frame(FrameIdx fid1, Rect<double> rect1, FrameIdx fid2, Rect<double> rect2, FrameIdx interfid){
    assert(fid1 <= interfid);
    assert(fid2 >= interfid);
    double alpha = double(fid2 - interfid)/(fid2 - fid1);
    double x = alpha*rect1.left + (1-alpha)*rect2.left;
    double y = alpha*rect1.top + (1-alpha)*rect2.top;
    double w = alpha*rect1.width + (1-alpha)*rect2.width;
    double h = alpha*rect1.height + (1-alpha)*rect2.height;
    return Rect<double>(x,y,w,h);
}

void trackInterp(int label, Frames &frames, TrackIdx tid, FrameIdx startframe, FrameIdx endframe){

    Rect<double> rect1 = frames[startframe][tid].rect;
    Rect<double> rect2 = frames[endframe][tid].rect;

    for(FrameIdx fid = startframe+1; fid < endframe; ++fid){

        if(frames[fid].find(tid)==frames[fid].end()){

            rect1 = frames[fid - 1][tid].rect;

            Rect<double> interrect = trackInterp_1frame(fid - 1, rect1, endframe, rect2, fid);

            frames[fid][tid] = Object(label, 1.0, fid, tid, interrect);

            MOD_STORE[tid].push_back(fid);
        }
        else {
            Rect<double> interrect = trackInterp_1frame(startframe, rect1, endframe, rect2, fid);

            frames[fid][tid] = Object(label, 1.0, fid, tid, interrect);
        }
    }
}

void compute_interpolation(Object trackrect, Frames& frames){

    FrameIdx fid = trackrect.frame_id;
    TrackIdx tid = trackrect.track_id;
    Label label = trackrect.label;

    FrameIdx last_frame = find_last_frame(tid, fid);
    if(last_frame >= 0)
        trackInterp(label, frames, tid, last_frame, fid);

    FrameIdx next_frame = find_next_frame(tid, fid);
    if(next_frame >= 0)
        trackInterp(label, frames, tid, fid, next_frame);

    qDebug()<<"Interpolation Computed.";
}

void addActivated(IN FrameIdx frameId, OUT Frames& frames, IN bool forward){
    bool cond1 = frameId >= frames.size();
    bool cond2 = frameId < 0;

    if(cond1|cond2)
        return;

    for(int i=0;i<activated_tracks.size();i++){

        if(frameId < activation_starts[i]){
            qDebug()<<"Generation Denied.";
            continue;
        }

        TrackIdx tid = activated_tracks[i];
        if(find(MOD_STORE[tid].begin(), MOD_STORE[tid].end(), frameId)!=MOD_STORE[tid].end())
            continue;

        if(forward){
            TrackIdx pre_fid = find_last_frame(tid, frameId);
            if(pre_fid<0)
                continue;

            Tracks1Frame& pre_frame = frames.at(pre_fid);
            if(pre_frame.find(tid) == pre_frame.end())
                continue;

            Object new_rect = pre_frame[tid];
            new_rect.frame_id = frameId;
            addObject(new_rect, frames);
        }
        else{
            TrackIdx next_fid = find_next_frame(tid, frameId);
            if(next_fid<0)
                continue;

            Tracks1Frame& next_frame = frames.at(next_fid);
            if(next_frame.find(tid) == next_frame.end())
                continue;

            Object new_rect = next_frame[tid];
            new_rect.frame_id = frameId;
            addObject(new_rect, frames);
        }
        qDebug()<<"activated items added";
    }
}

bool addObject(IN Object trackrect, OUT Frames& frames){

    FrameIdx fid = trackrect.frame_id;
    TrackIdx tid = trackrect.track_id;
    if (fid<0 || tid<0)
        return false;

    frames[fid][tid] = trackrect;
    MOD_STORE[tid].push_back(fid);

    qDebug()<<"Frame added";
    return true;
}

bool rmObjectFollowing(IN Object obj, OUT Frames& frames){

    FrameIdx fid_start = obj.frame_id;
    TrackIdx tid = obj.track_id;
    int framenum = frames.size();
    if (fid_start<0 || tid<0)
        return false;

    for(int fid = fid_start + 1; fid<framenum; ++fid){
        Tracks1Frame& frame = frames[fid];
        Tracks1Frame::iterator it = frame.find(tid);
        if(it != frame.end())
            frame.erase(it);
    }
    vector<FrameIdx>::iterator it;

    for (it = MOD_STORE[tid].begin(); it != MOD_STORE[tid].end(); it++)
    {
        if (*it > fid_start){
            MOD_STORE[tid].erase(it);
            it--;
        }
    }

    if (MOD_STORE[tid].size() == 0){
        MOD_STORE.erase(MOD_STORE.find(tid));
        vector<TrackIdx>::iterator it = find(activated_tracks.begin(), activated_tracks.end(), tid);
        if(it != activated_tracks.end()){
            int erase_index = it - activated_tracks.begin();
            activated_tracks.erase(it);
            activation_starts.erase(erase_index + activation_starts.begin());
        }
    }
    qDebug()<<"Followings Deleted";
    return true;
}

bool rmObjectCurrent(IN Object obj, OUT Frames& frames){
    FrameIdx fid = obj.frame_id;
    TrackIdx tid = obj.track_id;

    if (fid<0 || tid<0)
        return false;

    Tracks1Frame& frame = frames[fid];
    Tracks1Frame::iterator it_obj = frame.find(tid);
    if(it_obj == frame.end())
        return false;
    frame.erase(it_obj);

    vector<FrameIdx>::iterator it = find(MOD_STORE[tid].begin(), MOD_STORE[tid].end(), fid);
    if(it != MOD_STORE[tid].end())
        MOD_STORE[tid].erase(it);

    if (MOD_STORE[tid].size() == 0){
        MOD_STORE.erase(MOD_STORE.find(tid));
        vector<TrackIdx>::iterator it = find(activated_tracks.begin(), activated_tracks.end(), tid);

        if(it != activated_tracks.end()){
            int erase_index = it - activated_tracks.begin();
            activated_tracks.erase(it);
            activation_starts.erase(erase_index + activation_starts.begin());
        }
    }
    qDebug()<<"Current Frame Deleted";
    return true;
}

bool rmObjectPrevious(IN Object obj, OUT Frames& frames){

    FrameIdx fid_start = obj.frame_id;
    TrackIdx tid = obj.track_id;
    if (fid_start<0 || tid<0)
        return false;

    for(int fid = fid_start - 1; fid>=0; fid--){
        Tracks1Frame& frame = frames[fid];
        Tracks1Frame::iterator it = frame.find(tid);
        if(it != frame.end())
            frame.erase(it);
    }
    vector<FrameIdx>::iterator it;

    for (it = MOD_STORE[tid].begin(); it != MOD_STORE[tid].end(); it++)
    {
        if (*it < fid_start){
            MOD_STORE[tid].erase(it);
            it--;
        }
    }
    if (MOD_STORE[tid].size() == 0){

        MOD_STORE.erase(MOD_STORE.find(tid));
        vector<TrackIdx>::iterator it = find(activated_tracks.begin(), activated_tracks.end(), tid);
        if(it != activated_tracks.end()){
            int erase_index = it - activated_tracks.begin();
            activated_tracks.erase(it);
            activation_starts.erase(erase_index + activation_starts.begin());
        }
    }
    qDebug()<<"Previous Deleted";
    return true;
}

bool changeTrackId(IN Object obj, IN TrackIdx input_tid, OUT Frames& frames){

    TrackIdx pre_tid = obj.track_id;
    FrameIdx fid = obj.frame_id;

    if (MOD_STORE.find(input_tid) != MOD_STORE.end()){

        if(find(MOD_STORE[input_tid].begin(), MOD_STORE[input_tid].end(), fid) != MOD_STORE[input_tid].end())
            return false;
    }
    MOD_STORE[input_tid].push_back(fid);
    vector<FrameIdx>::iterator iter = find(MOD_STORE[pre_tid].begin(), MOD_STORE[pre_tid].end(), fid);

    if(iter != MOD_STORE[pre_tid].end())
        MOD_STORE[pre_tid].erase(iter);

    Tracks1Frame& frame = frames[fid];
    Tracks1Frame::iterator it = frame.find(pre_tid);
    if(it != frame.end())
        frame.erase(it);

    obj.track_id = input_tid;
    frame[input_tid] = obj;

    if (MOD_STORE[pre_tid].size() == 0){

        MOD_STORE.erase(MOD_STORE.find(pre_tid));

        vector<TrackIdx>::iterator it = find(activated_tracks.begin(), activated_tracks.end(), pre_tid);
        if(it != activated_tracks.end()){
            int erase_index = it - activated_tracks.begin();
            activated_tracks.erase(it);
            activation_starts.erase(erase_index + activation_starts.begin());
        }
    }
    qDebug()<<"TrackId Changed";
    return true;
}

void changeLabel(IN Object obj, IN int input_label, OUT Frames& frames){

    TrackIdx tid = obj.track_id;

    for(int i=0;i<MOD_STORE[tid].size();i++){

        Tracks1Frame& frame = frames.at(MOD_STORE[tid][i]);
        frame[tid].label = input_label;
    }
    qDebug()<<"Label Changed";
}

bool modifyTrackRect(IN Object trackrect, IN Rect<double> newrect, IN bool interpolates, OUT Frames& frames){

    FrameIdx fid = trackrect.frame_id;
    TrackIdx tid = trackrect.track_id;

    if(fid<0 || tid<0)
        return false;

    trackrect.rect = newrect;
    trackrect.confidence = 1.0f;
    frames[fid][tid] = trackrect;
    if(interpolates)
        compute_interpolation(trackrect, frames);
    return true;
}

TrackIdx getNewTrackId(){
    TrackIdx newTid = 1;
    while(true)
    {
        if (MOD_STORE.find(newTid) != MOD_STORE.end())
            newTid ++;
        else
            break;
    }
    return newTid;
}
