#include "trackInterface.h"
#include "trackManagerThread.h"

static trackInterface *sTrackInterface = nullptr;

trackInterface *trackInterface::Instance()
{
	if (nullptr == sTrackInterface)
	{
		sTrackInterface = new trackInterface(NULL);
	}
	return sTrackInterface;
}

trackInterface::trackInterface(QObject * parent)
{
	m_trackThread = QSharedPointer<trackManagerThread>(new trackManagerThread);
	m_trackThread.data()->start();

    QMetaObject::Connection ret = connect(this, SIGNAL(signal_removeObject(Object,int)), m_trackThread.data(), SLOT(slot_removeObject(Object,int)), Qt::QueuedConnection);
	 Q_ASSERT(ret);
	 ret = connect(this, SIGNAL(signal_addObject(Object)), m_trackThread.data(), SLOT(slot_addObject(Object)), Qt::QueuedConnection);
	 Q_ASSERT(ret);
     ret = connect(this, SIGNAL(signal_modifyRect(Object, Rect<double>, bool)), m_trackThread.data(), SLOT(slot_modifyRect(Object, Rect<double>, bool)), Qt::QueuedConnection);
	 Q_ASSERT(ret);
}

trackInterface::~trackInterface()
{

}

void trackInterface::removeObject(Object obj,int mode)
{
    emit signal_removeObject(obj,mode);
}

void trackInterface::addObject(Object obj)
{
	emit signal_addObject(obj);
}

void trackInterface::modifyRect(Object obj, Rect<double> rect, bool interpolates)
{
    emit signal_modifyRect(obj, rect, interpolates);
}
