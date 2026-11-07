#pragma once
#include "ChessEngineLogger.h"
#include <array>
#include <type_traits>
#include <utility>

namespace ChessEngine {
#define VECTOR_TAG "Vector"

template <typename T, size_t N>
class Vector {
    typedef typename std::array<T, N>::iterator iterator;
    typedef typename std::array<T, N>::const_iterator const_iterator;

 public:
    constexpr Vector() :
        array(),
        count() {}

    template <typename... OtherT>
    Vector(OtherT&&... args) :
        array(),
        count() {
        static_assert(sizeof...(OtherT) <= N);
        pushAll(std::forward<OtherT>(args)...);
    }

    constexpr size_t capacity() const { return N; }
    constexpr size_t available() const { return N - count; }
    constexpr size_t size() const { return count; }
    constexpr bool empty() const { return !count; }
    constexpr bool full() const { return count >= N; }

    iterator begin() { return array.begin(); }
    const_iterator begin() const { return array.begin(); }
    const_iterator cbegin() const { return array.begin(); }
    iterator end() { return array.begin() + count; }
    const_iterator end() const { return array.begin() + count; }
    const_iterator cend() const { return array.begin() + count; }

    T& at(size_t pos) { return array.at(pos); }
    const T& at(size_t pos) const { return array.at(pos); }
    T& operator[](size_t pos) { return array[pos]; }
    const T& operator[](size_t pos) const { return array[pos]; }
    T& front() { return array.front(); }
    const T& front() const { return array.front(); }
    T& back() { return array[count - 1]; }
    const T& back() const { return array[count - 1]; }

    void clear() {
        if (std::is_trivially_destructible<T>::value) {
            clearFast();
        } else {
            clearSafe();
        }
    }
    void resize(size_t size) { count = size < N ? size : N; }
    void clearFast() { count = 0; }
    void clearSafe() {
        for (size_t pos = 0; pos < count; pos++) {
            array[pos] = T();
        }
        count = 0;
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        if (full()) {
            CHESS_ENGINE_LOG(VECTOR_TAG, "Emplace dropped");
            return;
        }
        array[count++] = T(std::forward<Args>(args)...);
    }
    void push(const T& value) {
        if (full()) {
            CHESS_ENGINE_LOG(VECTOR_TAG, "Push dropped");
            return;
        }
        array[count++] = value;
    }
    void push(T&& value) {
        if (full()) {
            CHESS_ENGINE_LOG(VECTOR_TAG, "Push dropped");
            return;
        }
        array[count++] = std::move(value);
    }
    template <size_t OtherN>
    void push(const Vector<T, OtherN>& other) {
        push(other.array.data(), other.size());
    }
    template <size_t OtherN>
    void push(const std::array<T, OtherN>& arr) {
        push(arr.data(), arr.size());
    }
    void push(const T* ptr, size_t size) {
        if (count + size > N) {
            CHESS_ENGINE_LOG(VECTOR_TAG, "Push dropped");
            return;
        }
        for (size_t pos = 0; pos < size; pos++) {
            array[count + pos] = ptr[pos];
        }
        count += size;
    }
    Vector& operator=(const Vector& other) {
        for (size_t pos = 0; pos < other.count; pos++) {
            array[pos] = other.array[pos];
        }
        count = other.count;
        return *this;
    }
    bool operator==(const Vector& other) const { return count == other.count && array == other.array; }

    template <typename OtherT, size_t OtherN>
    friend class Vector;

 private:
    void pushAll() {}
    template <typename U, typename... Rest>
    void pushAll(U&& value, Rest&&... rest) {
        array[count++] = T(std::forward<U>(value));
        pushAll(std::forward<Rest>(rest)...);
    }

    std::array<T, N> array;
    size_t count;
};
}
