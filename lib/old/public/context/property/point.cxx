#include <StormByte/multimedia/context/property/point.hxx>

using namespace StormByte::Multimedia::Context::Property;

Point::Point(int x, int y) noexcept:
m_x(x), m_y(y) {}

int Point::X() const noexcept {
	return m_x;
}

int Point::Y() const noexcept {
	return m_y;
}

Point Point::Normalized(int numerator_x, int denominator_x, int numerator_y, int denominator_y, int denominator) noexcept {
	double rate_x = denominator > 0 ? static_cast<double>(denominator) / static_cast<double>(denominator_x) : 1;
	double rate_y = denominator > 0 ? static_cast<double>(denominator) / static_cast<double>(denominator_y) : 1;
	return {
		static_cast<int>((static_cast<double>(numerator_x) * rate_x)),
		static_cast<int>((static_cast<double>(numerator_y) * rate_y))
	};
}