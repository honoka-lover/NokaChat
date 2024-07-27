#ifndef DATAQUEUE_H
#define DATAQUEUE_H

extern "C"{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
}


#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class DataQueue {
public:
    DataQueue(size_t maxSize = 100) : maxSize(maxSize),stopped(false) {}

    bool  push(T item) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this]() { return queue.size() < maxSize || stopped; });
        // if (stopped) return false;
        if(queue.size() >= maxSize)
            return false;
        queue.push(item);
        cond.notify_all();
        return true;
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex);
        cond.wait(lock, [this]() { return !queue.empty() || stopped; });
        if (stopped && queue.empty()) return false;
        item = queue.front();
        queue.pop();
        cond.notify_all();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    void stop() {
        std::unique_lock<std::mutex> lock(mutex);
        stopped = true;
        cond.notify_all();
    }

    void resume(){
        std::unique_lock<std::mutex> lock(mutex);
        stopped = false;
        cond.notify_all();
    }
private:
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable cond;
    size_t maxSize;
    std::atomic<bool> stopped;
};

// #include<QSharedPointer>
// #include<QThread>
// #include<QQueue>
// #include<QMutex>
// #include<QTimer>
// class DataQueue:public QThread
// {
//     Q_OBJECT
// signals:
//     void dataSend(QSharedPointer<AVFrame> data);
// public:
//     explicit DataQueue(QObject *parent = nullptr);

//     DataQueue(const DataQueue& other)=delete;
//     DataQueue& operator=(const DataQueue& other)=delete;

//     ~DataQueue();

//     void run();

//     void pushData(QSharedPointer<AVFrame>,double,int type);

//     int getDataNum();
// private:
//     struct Node{
//         QSharedPointer<AVFrame> frame;
//         double time;
//         int type;
//     };
//     QQueue<Node> m_queue;
//     QMutex m_mutex;

// };

#endif // DATAQUEUE_H
