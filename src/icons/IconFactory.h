#pragma once

#include <QColor>
#include <QIcon>

namespace KLinkMonitorNative {

enum class InterfaceKind;

class IconFactory
{
public:
    static QIcon ethernetIcon(const QColor &color);
    static QIcon wifiIcon(const QColor &color);
    static QIcon shieldIcon(const QColor &color);
    static QIcon virtualIcon(const QColor &color);
    static QIcon iconForKind(InterfaceKind kind, const QColor &color);
};
}
