#include "libs/postgres/include/pg_pool.h"
//#include "impl/pg_pool.h"

namespace vm_scheduler {

//PgPool::PgPool(const size_t poolSize)
//{
//    for (auto i = 0; i < poolSize_; i++) {
//        connections.emplace(std::make_shared<pqxx::lazyconnection>());
//    }
//}
//
//std::shared_ptr<pqxx::lazyconnection> PGPool::connection()
//{
//    std::unique_lock<std::mutex> lock_(m_mutex);
//
//    // if pool is empty, then wait until it notifies back
//    while (m_pool.empty()) {
//        m_condition.wait(lock_);
//    }
//
//    // get new connection in queue
//    auto conn_ = m_pool.front();
//    // immediately pop as we will use it now
//    m_pool.pop();
//
//    return conn_;
//}
//
//void PGPool::freeConnection(std::shared_ptr<pqxx::lazyconnection> conn_)
//{
//    std::unique_lock<std::mutex> lock_(m_mutex);
//
//    // push a new connection into a pool
//    m_pool.push(conn_);
//
//    // unlock mutex
//    lock_.unlock();
//
//    // notify one of thread that is waiting
//    m_condition.notify_one();
//}

} // namespace vm_scheduler
