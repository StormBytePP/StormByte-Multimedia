#pragma once

#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/features.hxx>

#include <string>

/**
 * @namespace Engine
 * @brief The namespace for all multimedia engine classes.
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Encoder
	 * @brief Class representing a decoder.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Encoder final {
		friend class Codec;
		public:
			/**
			 * @brief The default copy constructor.
			 * @param other The other decoder to copy from.
			 */
			Encoder(const Encoder& other) 							= default;

			/**
			 * @brief The default move constructor.
			 * @param other The other decoder to move from.
			 */
			Encoder(Encoder&& other) noexcept 						= default;

			/**
			 * @brief Default destructor.
			 */
			~Encoder() noexcept 									= default;

			/**
			 * @brief The default copy assignment operator.
			 * @param other The other decoder to copy from.
			 * @return Encoder& Reference to this decoder.
			 */
			Encoder& operator=(const Encoder& other) 				= default;

			/**
			 * @brief The default move assignment operator.
			 * @param other The other decoder to move from.
			 * @return Encoder& Reference to this decoder.
			 */
			Encoder& operator=(Encoder&& other) noexcept 			= default;

			/**
			 * @brief Gets the ID of the decoder.
			 * @return int The ID of the decoder.
			 */
			int 													CodecID() const noexcept;

			/**
			 * @brief Gets the capabilities of the decoder.
			 * @return int The capabilities of the decoder.
			 */
			const StormByte::Multimedia::Features&					Features() const noexcept;

			/**
			 * @brief Gets the name of the decoder.
			 * @return const std::string& The name of the decoder.
			 */
			const std::string& 										Name() const noexcept;

		private:
			int m_id;
			std::string m_name;
			StormByte::Multimedia::Features m_features;

			/**
			 * @brief Private constructor.
			 * @param id The ID of the decoder.
			 * @param name The name of the decoder.
			 * @param features The features of the decoder.
			 */
			Encoder(int id, const std::string& name) noexcept;

			/**
			 * @brief Private move constructor.
			 * @param id The ID of the decoder.
			 * @param name The name of the decoder.
			 * @param features The features of the decoder.
			 */
			Encoder(int id, std::string&& name) noexcept;

			/**
			 * @brief Detects features based on the decoder name.
			 * @param name The name of the decoder.
			 * @return Features The detected features.
			 */
			static StormByte::Multimedia::Features			DetectFeatures(const std::string_view& name) noexcept;
	};
}