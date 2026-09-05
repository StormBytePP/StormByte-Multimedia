#pragma once

#include <StormByte/multimedia/media/type.hxx>

#include <string_view>

/**
 * @namespace StormByte::Multimedia::Media
 * @brief Public media types: codecs, registry and stream kinds.
 */
namespace StormByte::Multimedia::Media {
	class Registry;

	/**
	 * @class Codec
	 * @brief Immutable codec identity owned by Registry.
	 *
	 * Name() is the StormByte key. FFmpeg ids live only in the registry map.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Codec {
		public:
			/**
			 * @brief Copy is disabled; instances are unique in the registry.
			 */
			Codec(const Codec&) noexcept = delete;

			/**
			 * @brief Move constructor.
			 */
			Codec(Codec&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Codec() noexcept = default;

			/**
			 * @brief Copy assignment is disabled.
			 * @return *this.
			 */
			Codec& operator=(const Codec&) noexcept = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Codec& operator=(Codec&&) noexcept = default;

			/**
			 * @brief Identity equality (same registry slot).
			 * @param other Other codec.
			 * @return true if both refer to the same instance.
			 */
			bool operator==(const Codec& other) const noexcept;

			/**
			 * @brief Identity inequality.
			 * @param other Other codec.
			 * @return true if they are different instances.
			 */
			bool operator!=(const Codec& other) const noexcept;

			/**
			 * @brief Media kind of this codec.
			 * @return Type value.
			 */
			constexpr Media::Type Type() const noexcept { return m_type; }

			/**
			 * @brief StormByte codec name.
			 * @return View to a process-lifetime literal.
			 */
			constexpr std::string_view Name() const noexcept { return m_name; }

			/**
			 * @brief Human description.
			 * @return View to a process-lifetime literal.
			 */
			constexpr std::string_view Description() const noexcept { return m_description; }

			/**
			 * @brief Tests Read/Write flags.
			 * @param access Flags to test.
			 * @return true if every bit in @p access is set.
			 */
			bool HasAccess(Access access) const noexcept;

		private:
			friend class Registry;

			Media::Type m_type;					///< Stream / codec kind
			std::string_view m_name;			///< StormByte name
			std::string_view m_description;		///< Description
			Access m_access;					///< Read and optional Write

			/**
			 * @brief Registry-only constructor.
			 * @param type Media kind.
			 * @param name StormByte name (table literal).
			 * @param description Description (table literal).
			 * @param access Capability mask.
			 */
			constexpr Codec(Media::Type type, std::string_view name, std::string_view description, Access access) noexcept
			: m_type(type), m_name(name), m_description(description), m_access(access) {}
	};
}
