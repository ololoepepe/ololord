#ifndef OLOLORDWEBAPPTHREAD_H
#define OLOLORDWEBAPPTHREAD_H

#include <QThread>

#include <cppcms/json.h>

class OlolordWebAppThread : public QThread
{
private:
    const cppcms::json::value Conf;
public:
    explicit OlolordWebAppThread(const cppcms::json::value &conf, QObject *parent = 0);
protected:
    void run();
};

#endif // OLOLORDWEBAPPTHREAD_H
