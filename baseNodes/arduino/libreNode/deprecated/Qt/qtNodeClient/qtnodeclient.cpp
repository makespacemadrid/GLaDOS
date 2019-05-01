#include "qtnodeclient.h"

qtNodeClient::qtNodeClient(nodeClient *nc, QObject *parent) : QObject(parent)
{
    if(nc)
    {
        m_nodeClient = nc;
    }
    else
    {
        m_nodeClient = new nodeClient();
    }
}
