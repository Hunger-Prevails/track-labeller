#ifndef _TRACK_INTERFACE_
#define _TRACK_INTERFACE_
#include <QObject>
#include "trackManagerThread.h"

class trackInterface : public QObject
{
	Q_OBJECT
public:
	trackInterface(QObject * parent);
	~trackInterface();

	static trackInterface * Instance();

public:
    void removeObject(Object obj,int mode);
	void addObject(Object obj);
    void modifyRect(Object obj, Rect<double> rect, bool interpolates);
signals:
    void signal_removeObject(Object, int);
	void signal_addObject(Object);
    void signal_modifyRect(Object, Rect<double>, bool);
	void signal_modifyObject(Object);

private:
	QSharedPointer<trackManagerThread> m_trackThread;

	Q_DISABLE_COPY(trackInterface)
};

#endif //_TRACK_INTERFACE_

