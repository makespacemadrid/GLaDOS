#ifndef QTNODECLIENT_H
#define QTNODECLIENT_H

#include <QObject>
#include "nodeclient.h"

class qtNodeClient : public QObject
{
    Q_OBJECT
public:
    explicit qtNodeClient(nodeClient* nc = 0,QObject* parent = 0);

protected:
    nodeClient* m_nodeClient;
    bool        m_deleteClient;

signals:

public slots:
};

#endif // QTNODECLIENT_H
