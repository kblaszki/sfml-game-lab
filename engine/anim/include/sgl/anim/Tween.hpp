#pragma once

#include <sgl/anim/Easing.hpp>
#include <sgl/core/Time.hpp>

namespace sgl
{

template<class T>
class Tween
{
public:
    void start(T from, T to, Seconds duration, EaseFn ease)
    {
        from_ = from;
        to_ = to;
        duration_ = duration;
        ease_ = ease != nullptr ? ease : linear;
        elapsed_ = Seconds{0.f};

        if(duration_.count() <= 0.f)
        {
            value_ = to_;
            done_ = true;
            running_ = false;
            return;
        }

        value_ = from_;
        done_ = false;
        running_ = true;
    }

    void update(Seconds dt)
    {
        if(done_)
        {
            return;
        }

        elapsed_ += dt;
        if(elapsed_ >= duration_)
        {
            value_ = to_;
            done_ = true;
            running_ = false;
            return;
        }

        const float t = elapsed_.count() / duration_.count();
        const float e = ease_(t);
        value_ = from_ + (to_ - from_) * e;
    }

    [[nodiscard]] T value() const
    {
        return value_;
    }

    [[nodiscard]] bool done() const noexcept
    {
        return done_;
    }

    [[nodiscard]] bool running() const noexcept
    {
        return running_;
    }

private:
    T from_{};
    T to_{};
    T value_{};
    Seconds duration_{};
    Seconds elapsed_{};
    EaseFn ease_{linear};
    bool done_{true};
    bool running_{false};
};

} // namespace sgl
