#pragma once

#include "esphome/components/light/addressable_light.h"
#include "esphome/core/hal.h"
#include <algorithm>
#include <array>

namespace esphome {
namespace tx_ultimate_easy {

// Keep the normal pixels separate from the physical strip. Relay partitions,
// transitions and effects continue updating this buffer while the scan is shown.
class TouchScanLightOutput : public light::AddressableLight {
 public:
  TouchScanLightOutput(int32_t num_leds, int32_t scan_leds)
      : num_leds_(num_leds), scan_leds_(std::min(num_leds, scan_leds)) {}

  void set_strip(light::AddressableLight *strip) { this->strip_ = strip; }
  int32_t size() const override { return this->num_leds_; }
  light::LightTraits get_traits() override { return this->strip_->get_traits(); }
  void clear_effect_data() override { this->effect_data_.fill(0); }

  void setup() override {
    this->set_interval(100, [this]() {
      if (this->touch_active_)
        this->schedule_show();
    });
  }

  void set_touch_active(bool active) {
    if (active == this->touch_active_)
      return;
    this->touch_active_ = active;
    if (active)
      this->scan_started_ = millis();
    this->schedule_show();
  }

  void write_state(light::LightState *state) override {
    const uint32_t head = (millis() - this->scan_started_) / 100 % this->scan_leds_;
    for (int32_t i = 0; i < this->num_leds_; i++) {
      const auto &pixel = this->pixels_[i];
      auto target = (*this->strip_)[i];
      if (this->touch_active_ && i < this->scan_leds_) {
        // Three blue LEDs circulate continuously around the perimeter.
        const bool blue = (i + this->scan_leds_ - head) % this->scan_leds_ < 3;
        target.set_raw(0, 0, blue ? 255 : 0, 0);
      } else {
        // Copy hardware levels verbatim: never apply gamma/brightness twice.
        target.set_raw(pixel.r, pixel.g, pixel.b, pixel.w);
      }
    }
    this->strip_->schedule_show();
    this->mark_shown_();
  }

 protected:
  light::ESPColorView get_view_internal(int32_t index) const override {
    auto &pixel = this->pixels_[index];
    return light::ESPColorView(&pixel.r, &pixel.g, &pixel.b, &pixel.w,
                               &this->effect_data_[index], &this->correction_);
  }

  light::AddressableLight *strip_{nullptr};
  int32_t num_leds_;
  int32_t scan_leds_;
  mutable std::array<Color, 32> pixels_{};
  mutable std::array<uint8_t, 32> effect_data_{};
  bool touch_active_{false};
  uint32_t scan_started_{0};
};

}  // namespace tx_ultimate_easy
}  // namespace esphome
