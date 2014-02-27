/* (c) 2013-2014 Jasper Spaans <j@jasper.es>
 *
 * Just use it and be happy. No warranty from my side.
 */

#ifndef __THREADPOOL11_HH__
#define __THREADPOOL11_HH__

#include <atomic>
#include <cstdint>

/** simple multi-reader / multi-writer fifo; attempts to be lock-free
 *
 *  it has a ringbuffer and several pointers inside that buffer:
 *  - reader_pos (what is the next position to read?)
 *  - read_pos (what is the last position that was read?)
 *  - writer_pos (what is the next position to write?)
 *  - written_pos (what is the last succesfully written position?)
 *
 *  if the buffer is empty, reader_pos == writer_pos == written_pos
 *  if the buffer is full, add_wrap(writer_pos) == reader_pos
 */

typedef uint32_t size_type;
template<typename T, const size_type p_size=512>
class fifo
{
private:
    std::atomic<size_type> pos_read, pos_reader;
    std::atomic<size_type> pos_written, pos_writer;

    typedef std::atomic<T> aT;
    aT *fifo_buffer;

    inline size_type add_wrap(const size_type p) {
        return (p+1)%p_size;
    }

public:
    fifo():
        pos_read(0), pos_reader(0),
        pos_written(0), pos_writer(0)
    {
        fifo_buffer = new aT[p_size];
    }

    ~fifo()
    {
        delete[] fifo_buffer;
    }

    size_type size() const { return p_size; }

    bool push(const T &element)
    {
        for (;;) {
            size_type to_write = pos_writer;
            if (add_wrap(to_write) == pos_read) {
                return false;
            }

            // get our own pointer nobody may touch!
            if (!pos_writer.compare_exchange_weak(to_write,
                                                  add_wrap(to_write))) {
                // someone else is writing... retry
                continue;
            }
            fifo_buffer[to_write] = element;

            // update the "written" pointer by incrementing it;
            // this unfortunately cannot be done using a normal
            // increment because we use a ringbuffer.
            for (size_type written = pos_written;
                 !pos_written.compare_exchange_weak(written, add_wrap(written));
                 written = pos_written);
            return true;
        }
    }

    bool pop(T &element)
    {
        for(;;) {
            size_type to_read = pos_reader;
            if (to_read == pos_written) {
                return false;
            }

            // get our own pointer nobody may touch!
            if (!pos_reader.compare_exchange_weak(to_read,
                                                  add_wrap(to_read))) {
                // someone else is also reading... back to start
                continue;
            }
            element = fifo_buffer[to_read];

            // update the "read" pointer by incrementing it;
            // this unfortunately cannot be done using a normal
            // increment because we use a ringbuffer
            for (size_type read = pos_read;
                 !pos_read.compare_exchange_weak(read, add_wrap(read));
                 read = pos_read);
            return true;
        }
    }
};

#endif // __THREADPOOL11_HH__
