void WfirstRWLock::lock_read() {
    std::unique_lock<std::mutex> ulk(counter_mutex);
    cond_r.wait(ulk, [=]() -> bool { return write_cnt == 0; });
    ++read_cnt;
}
void WfirstRWLock::lock_write() {
    std::unique_lock<std::mutex> ulk(counter_mutex);
    ++write_cnt;
    cond_w.wait(ulk, [=]() -> bool { return read_cnt == 0 && !inwriteflag; });
    inwriteflag = true;
}
void WfirstRWLock::release_read() {
    std::unique_lock<std::mutex> ulk(counter_mutex);
    if (--read_cnt == 0 && write_cnt > 0) {
        cond_w.notify_one();
    }
}
void WfirstRWLock::release_write() {
    std::unique_lock<std::mutex> ulk(counter_mutex);
    if (--write_cnt == 0) {
        cond_r.notify_all();
    } else {
        cond_w.notify_one();
    }
    inwriteflag = false;
}