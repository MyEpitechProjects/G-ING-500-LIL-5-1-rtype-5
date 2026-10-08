#ifndef WAKE_SIGNAL_HPP
    #define WAKE_SIGNAL_HPP

    #include <mutex>
    #include <condition_variable>

class WakeSignal {
    public:
        WakeSignal() = default;
        ~WakeSignal() = default;
        
        void notify();

        void wait();

    private:
        std::mutex _mutex;
        std::condition_variable _condition;
        bool _pending = false;
};

#endif