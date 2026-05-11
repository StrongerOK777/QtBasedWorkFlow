#pragma once

#include <QString>
#include <utility>

template <typename T>
class Result {
public:
    static Result ok(const T& value) { return Result(true, value, {}); }
    static Result ok(T&& value) { return Result(true, std::move(value), {}); }
    static Result fail(const QString& message) { return Result(false, T{}, message); }

    bool isOk() const { return ok_; }
    bool isFail() const { return !ok_; }
    const QString& error() const { return error_; }
    const T& value() const { return value_; }
    T& value() { return value_; }

private:
    Result(bool ok, T value, QString error)
        : ok_(ok), value_(std::move(value)), error_(std::move(error)) {}

    bool ok_ = false;
    T value_{};
    QString error_;
};

class Status {
public:
    static Status ok() { return Status(true, {}); }
    static Status fail(const QString& message) { return Status(false, message); }

    bool isOk() const { return ok_; }
    bool isFail() const { return !ok_; }
    const QString& error() const { return error_; }

private:
    Status(bool ok, QString error) : ok_(ok), error_(std::move(error)) {}

    bool ok_ = false;
    QString error_;
};
