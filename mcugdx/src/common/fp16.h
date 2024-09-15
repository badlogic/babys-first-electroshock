#include <cstdint>

class mcugdx_fp16_t {
private:
	int32_t value;

public:
	static constexpr int32_t FRACTIONAL_BITS = 16;
	static constexpr int32_t ONE = 1 << FRACTIONAL_BITS;

	constexpr mcugdx_fp16_t() : value(0) {}
	constexpr explicit mcugdx_fp16_t(int32_t i) : value(i << FRACTIONAL_BITS) {}
	constexpr explicit mcugdx_fp16_t(float f) : value(static_cast<int32_t>(f * ONE)) {}

	constexpr int32_t raw() const { return value; }

	constexpr float to_float() const { return static_cast<float>(value) / ONE; }
	constexpr int32_t to_int() const { return value >> FRACTIONAL_BITS; }

	constexpr mcugdx_fp16_t operator+(mcugdx_fp16_t rhs) const { return mcugdx_fp16_t(value + rhs.value); }
	constexpr mcugdx_fp16_t operator-(mcugdx_fp16_t rhs) const { return mcugdx_fp16_t(value - rhs.value); }
	constexpr mcugdx_fp16_t operator*(mcugdx_fp16_t rhs) const {
		return mcugdx_fp16_t((int32_t)(static_cast<int64_t>(value) * rhs.value) >> FRACTIONAL_BITS);
	}
	constexpr mcugdx_fp16_t operator/(mcugdx_fp16_t rhs) const {
		return mcugdx_fp16_t((int32_t)(static_cast<int64_t>(value) << FRACTIONAL_BITS) / rhs.value);
	}

	constexpr mcugdx_fp16_t &operator+=(mcugdx_fp16_t rhs) {
		value += rhs.value;
		return *this;
	}

	constexpr mcugdx_fp16_t &operator-=(mcugdx_fp16_t rhs) {
		value -= rhs.value;
		return *this;
	}

	constexpr bool operator==(mcugdx_fp16_t rhs) const { return value == rhs.value; }
	constexpr bool operator!=(mcugdx_fp16_t rhs) const { return value != rhs.value; }
	constexpr bool operator<(mcugdx_fp16_t rhs) const { return value < rhs.value; }
	constexpr bool operator>(mcugdx_fp16_t rhs) const { return value > rhs.value; }
	constexpr bool operator<=(mcugdx_fp16_t rhs) const { return value <= rhs.value; }
	constexpr bool operator>=(mcugdx_fp16_t rhs) const { return value >= rhs.value; }

private:
	constexpr explicit mcugdx_fp16_t(int32_t raw_value) : value(raw_value) {}
};