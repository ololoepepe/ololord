TEMPLATE = subdirs

SUBDIRS = lib app

app.depends = lib

TRANSLATIONS += \
    ../translations/ololord_ru.ts
