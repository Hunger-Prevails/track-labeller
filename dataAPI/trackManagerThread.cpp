#include "trackManagerThread.h"
#include <QDebug>

extern Frames g_frames;
extern QReadWriteLock* g_pTrackInfosLock;

trackManagerThread::trackManagerThread(QObject *parent) :
	QThread(parent)
{
	moveToThread(this);
}

trackManagerThread::~trackManagerThread()
{

}

void trackManagerThread::run()
{
	exec();
}

void trackManagerThread::slot_removeObject(Object obj,int mode)
{
	g_pTrackInfosLock->lockForRead();

	switch (mode)
	{
	case 0:
        rmObjectCurrent(obj, g_frames);
		break;
	case 1:
        rmObjectPrevious(obj, g_frames);
		break;
	case 2:
        rmObjectFollowing(obj, g_frames);
		break;
	default:
        rmObjectCurrent(obj, g_frames);
		break;
	}
	g_pTrackInfosLock->unlock();
}

void trackManagerThread::slot_addObject(Object obj)
{
	g_pTrackInfosLock->lockForRead();
    addObject(obj, g_frames);
	g_pTrackInfosLock->unlock();
}

void trackManagerThread::slot_modifyRect(Object obj, Rect<double> newrect, bool interpolates)
{
	g_pTrackInfosLock->lockForRead();
    modifyTrackRect(obj, newrect, interpolates, g_frames);
	g_pTrackInfosLock->unlock();
}
