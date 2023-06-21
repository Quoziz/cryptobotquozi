#ifndef STEGOFILE_H
#define STEGOFILE_H

#include <boost/asio/streambuf.hpp>
#include <iostream>

/**
 * @brief Base StegoContainer class
 */
class StegoContainer {
protected:
    int8_t* buf;
    size_t pos;
    size_t size;
public:
    StegoContainer(int8_t* data, size_t length) : buf(data), pos(0), size(length) {}
};

/**
 * @brief StegoContainer for writing
 */
class WriteonlyStegoContainer : public StegoContainer {
public:
    WriteonlyStegoContainer(int8_t* data, size_t length) : StegoContainer(data, length) {}

    /**
     * @brief Writes @ref T object into container; @ref T is only integer type
     * @param what object for writing
     * @throws std::out_of_range on container overflow
     */
    template<typename T>
    void write(T what) {
        static_assert (std::numeric_limits<T>::is_integer, "only integer types are allowed");
        if (this->pos + sizeof (T)*4 >= this->size) throw std::out_of_range("stego container overflow");

        for (uint16_t i = this->pos, reps = 0; reps < sizeof (T); i+=4, ++reps) {
            int8_t ch = static_cast<int8_t>(what);
            this->buf[i]   = (this->buf[i]   & 0b11111100) | (ch & 0b11);
            this->buf[i+1] = (this->buf[i+1] & 0b11111100) | ((ch >> 2) & 0b11);
            this->buf[i+2] = (this->buf[i+2] & 0b11111100) | ((ch >> 4) & 0b11);
            this->buf[i+3] = (this->buf[i+3] & 0b11111100) | ((ch >> 6) & 0b11);
            what >>= 8;
        }

        this->pos += sizeof (T)*4;
    }

    /**
     * @brief Writes array of @ref T object into container; @ref T is only integer type
     * @param what array of objects for writing
     * @param length @ref what array size
     * @throws std::out_of_range on container overflow
     */
    template<typename T>
    void write(T* what, size_t length) {
        for(size_t i = 0; i < length; ++i)
            this->write(what[i]);
    }
};

/**
 * @brief StegoContainer for reading
 */
class ReadonlyStegoContainer : public StegoContainer {
public:
    ReadonlyStegoContainer(int8_t* data, size_t length) : StegoContainer(data, length) {}

    /**
     * @brief Reads @ref T object from container; @ref T is only integer type
     * @return object from container
     * @throws std::out_of_range on container overflow
     */
    template<typename T>
    T read() {
        static_assert (std::numeric_limits<T>::is_integer, "only integer types are allowed");
        if (this->pos + sizeof (T)*4 >= this->size) throw std::out_of_range("stego container overflow");

        T result{0};
        for (uint16_t i = this->pos, reps = 0; reps < sizeof (T); i+=4, ++reps) {
            result |= ((buf[i]   & 0b11) << 0);
            result |= ((buf[i+1] & 0b11) << 2);
            result |= ((buf[i+2] & 0b11) << 4);
            result |= ((buf[i+3] & 0b11) << 6);
        }

        this->pos += sizeof (T)*4;

        return result;
    }

    /**
     * @brief Reads array of @ref T objects from container; @ref T is only integer type
     * @param where array-reviever
     * @param length array-reciver size
     * @throws std::out_of_range on container overflow
     */
    template<typename T>
    void read(T* where, size_t length) {
        for(size_t i = 0; i < length; ++i)
            where[i] = this->read<T>();
    }
};



#endif // STEGOFILE_H
