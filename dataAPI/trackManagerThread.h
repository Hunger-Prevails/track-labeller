#ifndef _TRACK_MANAGER_THREAD_
#define _TRACK_MANAGER_THREAD_

#include <QWidget>
#include <QThread>
#include "tracktypes.h"
#include "trackmanage.h"
#include <QReadWriteLock>

class trackManagerThread : public QThread
{
    Q_OBJECT

public:
	trackManagerThread(QObject *parent = 0);
	~trackManagerThread();

	void run();
public slots :
    void slot_removeObject(Object,int);
	void slot_addObject(Object);
    void slot_modifyRect(Object, Rect<double>, bool interpolates);

private:
	Q_DISABLE_COPY(trackManagerThread)
};

#endif // trackManagerThread
