#pragma once

namespace My {

    /*
       Design notes:
            There are many different ways you could implement this based on what
            you're exactly optimising for
            * Here we are assuming we have a lot of readers and very few writers

        * Our goal here was to avoid the mutex and two CVs
       */

    // A Writer-Preferred Reader-Writer Lock.
    //
    // Rules:
    // 1. Multiple Readers can hold the lock simultaneously.
    // 2. Only one Writer can hold the lock.
    // 3. WRITER PRIORITY: If a Writer is waiting, no NEW Readers can acquire the lock.
    // 4. Only use an atomic flag, to avoid overhead of mutex and lock
    class RWLock {
    public:
        void acquire_read() {
            while (true) {
                uint32_t old_flag = flag_.fetch_add(READER_UNIT, std::memory_order_acquire);

                //Check if we have a writer active or waiting
                if (old_flag & (WRITER_ACTIVE | WRITER_WAITING)){
                    //Undo our previous change!
                    uint32_t updated_flag = flag_.fetch_sub(READER_UNIT, std::memory_order_release);
                    //Notify if we have a writer waiting (in the event there was a race condition w/ acquire write)
                    if (updated_flag & WRITER_WAITING) {
                        flag_.notify_all();
                    }

                    //Wait for a change
                    flag_.wait(old_flag, std::memory_order_relaxed);
                }
                else {
                    //We are free to go!
                    return;
                }
            }

        };
        void release_read() {
            //If this was the last flag, then we want to notify
            if (flag_.fetch_sub(READER_UNIT, std::memory_order_release) == (WRITER_WAITING + READER_UNIT)) {
                //This means that this was the last reader AND there is a reader waiting
                flag_.notify_all();
            }
        };

        void acquire_write() {
            //Force the writer waiting flag:
            // -- This will prevent any future readers from progressing
            flag_.fetch_or(WRITER_WAITING, std::memory_order_acquire);

            while (true) {
                uint32_t old_flag  = flag_.load(std::memory_order_acquire);
                uint32_t expected  = WRITER_WAITING;

                if (old_flag == expected) {

                    if (flag_.compare_exchange_strong(expected, WRITER_ACTIVE) ) {
                        return;
                    }
                }

                //Either waiting for reader or a writer is already active
                //NOTE: There is a subtle race condition here, but release_write()
                //          calling notify_all() regardless of condition prevents it
                flag_.wait(old_flag, std::memory_order_relaxed);

            }
        };


        void release_write() {
            uint32_t old_flag = flag_.fetch_sub(WRITER_ACTIVE, std::memory_order_release);
            //We must notify if we had a writer active or if we were just the last writer
            flag_.notify_all();
        };


    private:
        //DESIGN: We need to have a way to
        //          1. Count number of readers
        //          2. Say if a writer is waiting
        //          3. Say if a writer is active
        //  and we need to do it in ONE atomic variable
        // => Use bitmasks
        static constexpr uint32_t WRITER_ACTIVE  = 1;
        //Add 1 to the 2nd bit
        static constexpr uint32_t WRITER_WAITING = 2;
        //Add 1 to the 3rd bit
        static constexpr uint32_t READER_UNIT  = 4;
        std::atomic<uint32_t> flag_ = 0;

    };
}

