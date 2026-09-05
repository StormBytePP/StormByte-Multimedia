#pragma once

#include <StormByte/bitmask.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>

/**
 * @namespace StormByte::Multimedia::Media
 * @brief Public media types: codecs, registry and stream kinds.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @enum Type
	 * @brief Kind of media stream or codec.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type {
		Audio,			///< Audio stream or codec
		Video,			///< Video stream or codec
		Subtitle,		///< Subtitle stream or codec
		Attachment,		///< Attachment / ancillary data
		Copy,			///< Stream copy (passthrough)
		Unknown			///< Unclassified type
	};

	/**
	 * @brief Converts a Type to a string literal.
	 * @param type Value to convert.
	 * @return Null-terminated name, or `"Invalid"`.
	 */
	constexpr const char* ToString(Type type) noexcept {
		switch (type) {
			case Type::Audio:		return "Audio";		///< Audio
			case Type::Video:		return "Video";		///< Video
			case Type::Subtitle:	return "Subtitle";	///< Subtitle
			case Type::Attachment:	return "Attachment";	///< Attachment
			case Type::Copy:		return "Copy";		///< Copy
			case Type::Unknown:		return "Unknown";	///< Unknown
			default:				return "Invalid";	///< Out of range
		}
	}

	/**
	 * @enum Operation
	 * @brief Read / write capability flags for a codec.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Operation: std::uint8_t {
		None	= 0,		///< No access
		Read	= 1 << 0,	///< Decode / demux is available
		Write	= 1 << 1	///< At least one encoder exists in this FFmpeg
	};

	/**
	 * @class Access
	 * @brief Bitmask of Operation flags for a codec.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Access: public StormByte::Bitmask<Access, Operation> {
		public:
			/**
			 * @brief Empty mask.
			 */
			constexpr Access() noexcept
			: StormByte::Bitmask<Access, Operation>() {}

			/**
			 * @brief Mask from a single operation.
			 * @param op Initial flag.
			 */
			constexpr Access(Operation op) noexcept
			: StormByte::Bitmask<Access, Operation>(op) {}

			/**
			 * @brief Copy constructor.
			 * @param access Source mask.
			 */
			constexpr Access(const Access& access) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param access Source mask.
			 */
			constexpr Access(Access&& access) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			constexpr ~Access() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param access Source mask.
			 * @return *this.
			 */
			constexpr Access& operator=(const Access& access) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param access Source mask.
			 * @return *this.
			 */
			constexpr Access& operator=(Access&& access) noexcept = default;
	};
}
