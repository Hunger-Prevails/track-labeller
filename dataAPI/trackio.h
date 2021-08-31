/*********************************************************************************
*     File Name           :     trackio.h
*     Created By          :     Xi Yangyang
*     Creation Date       :     [2017-01-20]
*     Last Modified       :     [2017-02-03]
*     Description         :     File IO operation
**********************************************************************************/
#ifndef __SENSETIME_TRACK_IO_H__
#define __SENSETIME_TRACK_IO_H__

#include "tracktypes.h"

bool loadLabels(IN const std::string filename, OUT Frames& frames);

bool saveLabels(IN const std::string filename, IN Frames& frames);

#endif
