#include "IconFactory.h"
#include "monitor/InterfaceMonitor.h"

#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPainterPath>

namespace KLinkMonitorNative {

QIcon IconFactory::ethernetIcon(const QColor &color)
{
    QPixmap pix(64, 64);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(QStringLiteral("#111111")), 3));
    p.setBrush(color);
    p.drawRoundedRect(8, 14, 48, 36, 6, 6);
    p.setBrush(color.lighter(125));
    p.drawRoundedRect(20, 6, 24, 14, 4, 4);
    p.setPen(QPen(QColor(QStringLiteral("#111111")), 2));
    for (int x : {16, 22, 28, 34, 40, 46}) p.drawLine(x, 34, x, 44);
    p.setPen(Qt::transparent);
    p.setBrush(QColor(255, 255, 255, 70));
    p.drawRoundedRect(14, 18, 22, 8, 4, 4);
    return QIcon(pix);
}

QIcon IconFactory::wifiIcon(const QColor &color)
{
    QPixmap pix(64, 64);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 7, Qt::SolidLine, Qt::RoundCap);
    p.setPen(pen);
    p.drawArc(10, 14, 44, 44, 35 * 16, 110 * 16);
    p.drawArc(20, 26, 24, 24, 35 * 16, 110 * 16);
    p.setBrush(color);
    p.setPen(QPen(QColor(QStringLiteral("#111111")), 2));
    p.drawEllipse(27, 46, 10, 10);
    return QIcon(pix);
}

QIcon IconFactory::shieldIcon(const QColor &color)
{
    QPixmap pix(64, 64);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.moveTo(32, 6);
    path.lineTo(52, 14);
    path.lineTo(48, 38);
    path.quadTo(43, 52, 32, 58);
    path.quadTo(21, 52, 16, 38);
    path.lineTo(12, 14);
    path.closeSubpath();
    p.setPen(QPen(QColor(QStringLiteral("#111111")), 3));
    p.setBrush(color);
    p.drawPath(path);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255,255,255,70));
    p.drawEllipse(23, 15, 16, 10);
    return QIcon(pix);
}

QIcon IconFactory::virtualIcon(const QColor &color)
{
    QPixmap pix(64, 64);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(QStringLiteral("#111111")), 3));
    p.setBrush(color);
    p.drawRoundedRect(10, 14, 44, 32, 5, 5);
    p.drawLine(25, 50, 39, 50);
    p.drawLine(32, 46, 32, 54);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255,255,255,70));
    p.drawRoundedRect(16, 20, 22, 7, 3, 3);
    return QIcon(pix);
}

QIcon IconFactory::iconForKind(InterfaceKind kind, const QColor &color)
{
    switch (kind) {
    case InterfaceKind::Wifi: return wifiIcon(color);
    case InterfaceKind::WireGuard:
    case InterfaceKind::Vpn: return shieldIcon(color);
    case InterfaceKind::Virtual: return virtualIcon(color);
    case InterfaceKind::Ethernet:
    case InterfaceKind::Unknown:
    default: return ethernetIcon(color);
    }
}

}
