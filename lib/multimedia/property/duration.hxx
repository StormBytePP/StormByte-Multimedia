#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @struct Duration
	 * @brief The struct for duration properties.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Duration {
		unsigned int	s_secs;	///< The seconds of the duration.
		/**
		 * @brief Gets the name of the duration.
		 * @return The name of the duration. (Example 1:30:15)
		 */
		std::string		GetName() const noexcept;
	};
}