#ifndef OLOLORDWEBAPPTHREAD_H
#define OLOLORDWEBAPPTHREAD_H

#include <cppcms/json.h>

#include <QThread>

class OlolordWebAppThread : public QThread
{
    Q_OBJECT
private:
    const cppcms::json::value Conf;
public:
    explicit OlolordWebAppThread(const cppcms::json::value &conf, QObject *parent = 0);
protected:
    void run();
};

#endif // OLOLORDWEBAPPTHREAD_H
