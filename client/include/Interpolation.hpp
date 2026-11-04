#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>

namespace rtype::interp {

    struct Point {
        float x = 0.0f;
        float y = 0.0f;
    };

    class SnapshotBuffer {
        public:
            void push(double time, float x, float y) {
                if (!_samples.empty() && time <= _samples.back().time)
                    return;
                _samples.push_back({time, x, y});
                while (_samples.size() > MAX_SAMPLES)
                    _samples.pop_front();
            }

            bool empty() const { return _samples.empty(); }
            std::size_t size() const { return _samples.size(); }
            Point latest() const { return {_samples.back().x, _samples.back().y}; }
            bool still() const {
                if (_samples.size() < 2)
                    return false;
                const Sample &a = _samples[_samples.size() - 2];
                const Sample &b = _samples.back();
                return a.x == b.x && a.y == b.y;
            }
            Point sample(double time, double maxExtrapolation) const {
                const Sample &first = _samples.front();
                const Sample &last = _samples.back();

                if (_samples.size() == 1 || time <= first.time)
                    return {first.x, first.y};
                if (time >= last.time) {
                    const Sample &prev = _samples[_samples.size() - 2];
                    const double span = last.time - prev.time;
                    const double ahead = std::min(time - last.time, maxExtrapolation);
                    if (span <= 0.0)
                        return {last.x, last.y};
                    return {
                        last.x + static_cast<float>((last.x - prev.x) / span * ahead),
                        last.y + static_cast<float>((last.y - prev.y) / span * ahead)
                    };
                }
                const auto upper = std::upper_bound(_samples.begin(), _samples.end(), time,
                    [](double t, const Sample &s) { return t < s.time; });
                const Sample &hi = *upper;
                const Sample &lo = *(upper - 1);
                const float k = static_cast<float>((time - lo.time) / (hi.time - lo.time));
                return {lo.x + (hi.x - lo.x) * k, lo.y + (hi.y - lo.y) * k};
            }

        private:
            struct Sample {
                double time;
                float x;
                float y;
            };
            static constexpr std::size_t MAX_SAMPLES = 32;
            std::deque<Sample> _samples;
    };

    class ServerClock {
        public:
            explicit ServerClock(double tickRate) : _tickRate(tickRate) {}
            void onSnapshot(std::uint32_t tick, double localArrival) {
                const double candidate = localArrival - tick / _tickRate;
                if (!_valid) {
                    _offset = candidate;
                    _valid = true;
                } else if (candidate < _offset) {
                    _offset = candidate;
                } else {
                    _offset += (candidate - _offset) * 0.01;
                }
            }

            void reset() { _valid = false; _offset = 0.0; }
            bool valid() const { return _valid; }
            double tickTime(std::uint32_t tick) const { return tick / _tickRate; }
            double serverTimeAt(double localNow) const { return localNow - _offset; }

        private:
            double _tickRate;
            double _offset = 0.0;
            bool _valid = false;
    };
}
