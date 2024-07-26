#include "Component/dataqueue.h"
#include <QEventLoop>

// DataQueue::DataQueue(QObject* parent):
//     QThread(parent)
// {

// }

// DataQueue::~DataQueue()
// {
//     m_mutex.lock();
//     while(!m_queue.empty()){
//         m_queue.dequeue();
//     }
//     m_mutex.unlock();
// }

// void DataQueue::run()
// {
//     using Ms = std::chrono::milliseconds;
//     while(true){
//         if(m_mutex.try_lock_for(Ms(10))){
//             bool ok = !m_queue.empty();
//             if(ok){
//                 Node dt = m_queue.dequeue();
//                 m_mutex.unlock();
//                 QSharedPointer<AVFrame> pFrame = dt.frame;
//                 double time = dt.time;
//                 emit dataSend(pFrame);
//                 if(dt.type== 0)
//                     usleep(time*1000);
//                 // else if(dt.type == 1)
//                 //     usleep(time*1000);
//             }else
//                 m_mutex.unlock();
//         }
//     }
// }

// void DataQueue::pushData(QSharedPointer<AVFrame> pFrame, double time,int type)
// {
//     m_mutex.lock();
//     Node node{
//         pFrame,
//         time,
//         type
//     };
//     m_queue.enqueue(node);
//     m_mutex.unlock();
// }

// int DataQueue::getDataNum()
// {
//     m_mutex.lock();
//     int len = m_queue.size();
//     m_mutex.unlock();
//     return len;
// }

