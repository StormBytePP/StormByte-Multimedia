#pragma once

#include <StormByte/multimedia/media/property/flags.hxx>
#include <StormByte/multimedia/media/type.hxx>

#include <memory>
#include <string>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
    /**
     * @class Codec
     * @brief The template class for all multimedia codecs.
     */
    class STORMBYTE_MULTIMEDIA_PUBLIC Codec {
		public:
			/**
			 * @brief Constructor.
			 * @param name The name of the codec.
			 * @param description The description of the codec.
			 * @param flags The flags of the codec.
			 */
			Codec(const std::string& name, const std::string& description, const Media::Property::Flags& flags) noexcept;

			/**
			 * @brief Constructor.
			 * @param name The name of the codec.
			 * @param description The description of the codec.
			 * @param flags The flags of the codec.
			 */
			Codec(std::string&& name, std::string&& description, Media::Property::Flags&& flags) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The codec to copy.
			 */
			Codec(const Codec& codec) noexcept						= default;

			/**
			 * @brief Move constructor.
			 * @param codec The codec to move.
			 */
			Codec(Codec&& codec) noexcept							= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The codec to copy.
			 * @return Reference to the assigned codec.
			 */
			Codec& operator=(const Codec& codec) noexcept			= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The codec to move.
			 * @return Reference to the assigned codec.
			 */
			Codec& operator=(Codec&& codec) noexcept				= default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Codec() noexcept 								= default;

			/**
			 * @brief Gets the name of the codec.
			 * @return The name of the codec.
			 */
			const std::string& 										Name() const noexcept;

			/**
			 * @brief Gets the description of the codec.
			 * @return The description of the codec.
			 */
			const std::string& 										Description() const noexcept;

			/**
			 * @brief Gets the name of the codec.
			 * @return The name of the codec.
			 */
			const std::shared_ptr<const Media::Property::Flags>& 	Flags() const noexcept;

			/**
			 * @brief Gets the type of the codec.
			 * @return The type of the codec.
			 */
			virtual Media::Type 									Type() const noexcept = 0;

		protected:
			std::string m_name;										///< The name of the codec.
			std::string m_description;								///< The description of the codec.
			std::shared_ptr<const Media::Property::Flags> m_flags;	///< The flags of the codec.
	};
}