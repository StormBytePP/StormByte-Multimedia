#pragma once

#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/typedefs.hxx>

#include <string>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Decoder
	 * @brief Class representing a decoder.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Decoder final {
		friend class Codec;
		public:
			/**
			 * @brief The default copy constructor.
			 * @param other The other decoder to copy from.
			 */
			Decoder(const Decoder& other) 					= default;

			/**
			 * @brief The default move constructor.
			 * @param other The other decoder to move from.
			 */
			Decoder(Decoder&& other) noexcept 				= default;

			/**
			 * @brief Default destructor.
			 */
			~Decoder() noexcept 							= default;

			/**
			 * @brief The default copy assignment operator.
			 * @param other The other decoder to copy from.
			 * @return Decoder& Reference to this decoder.
			 */
			Decoder& operator=(const Decoder& other) 		= default;

			/**
			 * @brief The default move assignment operator.
			 * @param other The other decoder to move from.
			 * @return Decoder& Reference to this decoder.
			 */
			Decoder& operator=(Decoder&& other) noexcept 	= default;

			/**
			 * @brief Gets the ID of the decoder.
			 * @return int The ID of the decoder.
			 */
			int 											CodecID() const noexcept;

			/**
			 * @brief Gets the capabilities of the decoder.
			 * @return int The capabilities of the decoder.
			 */
			const class Features&							Features() const noexcept;

			/**
			 * @brief Gets the name of the decoder.
			 * @return const std::string& The name of the decoder.
			 */
			const std::string& 								Name() const noexcept;

		private:
			int m_id;
			std::string m_name;
			class Features m_features;

			/**
			 * @brief Private constructor.
			 * @param id The ID of the decoder.
			 * @param name The name of the decoder.
			 * @param features The features of the decoder.
			 */
			Decoder(int id, const std::string& name) noexcept;

			/**
			 * @brief Private move constructor.
			 * @param id The ID of the decoder.
			 * @param name The name of the decoder.
			 * @param features The features of the decoder.
			 */
			Decoder(int id, std::string&& name) noexcept;

			/**
			 * @brief Detects features based on the decoder name.
			 * @param name The name of the decoder.
			 * @return Features The detected features.
			 */
			static class Features							DetectFeatures(const std::string_view& name) noexcept;
	};
}