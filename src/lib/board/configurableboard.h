#ifndef CONFIGURABLEBOARD_H
#define CONFIGURABLEBOARD_H

class QLocale;

#include "abstractboard.h"

#include <BTranslation>

#include <QString>

class OLOLORD_EXPORT ConfigurableBoard : public AbstractBoard
{
private:
    const BTranslation DefaultUserName;
    const QString Name;
    const BTranslation Title;
private:
    bool mshowWhois;
public:
    explicit ConfigurableBoard(const QString &name, const BTranslation &title,
                               const BTranslation &defaultUserName = BTranslation());
public:
    QString defaultUserName(const QLocale &l) const;
    QString name() const;
    void setShowWhois(bool show);
    bool showWhois() const;
    QString title(const QLocale &l) const;
};

#endif // CONFIGURABLEBOARD_H
