
#ifndef WOKER_THREAD_HPP
#define WOKER_THREAD_HPP


//#include <QMutex>
#include <QThread>

#include "../command.hpp"


#include "../queue.hpp"

#include <memory>
#include <atomic>
#include <string>


class interThreadData {
    public:

        QString str;

    interThreadData(QString _str): str(_str) {}

    interThreadData() = default;
    interThreadData(const interThreadData&) = default;
    interThreadData& operator=(const interThreadData&) = default;
    ~interThreadData() = default;
};

Q_DECLARE_METATYPE(interThreadData);


//! [WokerThread class definition]
class WokerThread : public QThread
{
    Q_OBJECT

    static constexpr size_t QueueSize = 512;

public:
    WokerThread(const std::string& path, QObject *parent = nullptr);
    ~WokerThread();

    void runCommand(std::unique_ptr<voltio::command>&& cmd);

signals:
    void sendResponse(const interThreadData&);
    void sendFatalError(const interThreadData&);

public slots:
    void stopProcess();

protected:
    void run();

private:
    
    std::string path;
    std::atomic<bool> isDone;
    bounded_queue <std::unique_ptr<voltio::command>> queue;

    //QMutex mutex;
};
//! [WokerThread class definition]

#endif