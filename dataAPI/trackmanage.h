/*********************************************************************************
*     File Name           :     trackmanage.h
*     Created By          :     Xi Yangyang
*     Modified By         :     Liu Yinglun
*     Creation Date       :     [2017-01-20]
*     Last Modified       :     [2017-06-25]
*     Description         :     APIs for managing Object
**********************************************************************************/
#ifndef __SENSETIME_TRACK_MANAGE_H__
#define __SENSETIME_TRACK_MANAGE_H__

#include "tracktypes.h"

// decides the startframe of the interpolation before the newly modified frame
FrameIdx find_last_frame(TrackIdx tid, FrameIdx modified_frame);

// decides the endframe of the interpolation after the newly modified frame
FrameIdx find_next_frame(TrackIdx tid, FrameIdx modified_frame);

// apply interpolation to all frames within the section (start, end).
void trackInterp(int label, Frames& frames, TrackIdx tid, FrameIdx start, FrameIdx end);

// decides where to commit this interpolation
void compute_interpolation(Object trackrect, Frames& frames);

// apply one single act of interpolation.
Rect<double> trackInterp_1frame(FrameIdx fid1, Rect<double> rect1, FrameIdx fid2, Rect<double> rect2, FrameIdx interfid);

// remove all following frames.
bool rmObjectFollowing(IN Object trackrect, OUT Frames& frames);

// remove the current frame.
bool rmObjectCurrent(IN Object trackrect, OUT Frames& frames);

// remove all previous frame.
bool rmObjectPrevious(IN Object trackrect, OUT Frames& frames);

// add all activated tracks to this frame.
void addActivated(IN FrameIdx frameId, OUT Frames& frames, IN bool forward);

//add a new object for one frame.
bool addObject(IN Object trackrect, OUT Frames& frames);

//change trackId for one frame.
bool changeTrackId(IN Object trackrect, IN TrackIdx input_tid, OUT Frames& frames);

//change label for all frames.
void changeLabel(IN Object trackrect, IN int input_label, OUT Frames& frames);

//modify a object's bbox for one frame.
bool modifyTrackRect(IN Object trackrect, IN Rect<double> newrect, IN bool interpolates, OUT Frames& frames);

//returns a track id that has not been occupied.
TrackIdx getNewTrackId();

#endif
