#include "configurableboard.h"

#include "translator.h"

#include <BTranslation>

#include <QByteArray>
#include <QDebug>
#include <QLocale>
#include <QString>

ConfigurableBoard::ConfigurableBoard(const QString &name, const BTranslation &title,
                                     const BTranslation &defaultUserName) :
    DefaultUserName(defaultUserName), Name(name), Title(title)
{
    mmarkupElements = AbstractBoard::markupElements();
    mshowWhois = AbstractBoard::showWhois();
}

QString ConfigurableBoard::defaultUserName(const QLocale &l) const
{
    if (!DefaultUserName.isValid())
        return AbstractBoard::defaultUserName(l);
    const BTranslation &un = DefaultUserName;
    return TranslatorQt(l).translate(un.context().toUtf8().constData(), un.sourceText().toUtf8().constData(),
                                     un.disambiguation().toUtf8().constData(), un.n());
}

AbstractBoard::MarkupElements ConfigurableBoard::markupElements() const
{
    return mmarkupElements;
}

QString ConfigurableBoard::name() const
{
    return Name;
}

void ConfigurableBoard::setMarkupElements(MarkupElements elements)
{
    mmarkupElements = elements;
}

void ConfigurableBoard::setShowWhois(bool show)
{
    mshowWhois = show;
}

bool ConfigurableBoard::showWhois() const
{
    return mshowWhois;
}

QString ConfigurableBoard::title(const QLocale &l) const
{
    return TranslatorQt(l).translate(Title.context().toUtf8().constData(), Title.sourceText().toUtf8().constData(),
                                     Title.disambiguation().toUtf8().constData(), Title.n());
}
