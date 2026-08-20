#pragma once

#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/features.hxx>

#include <string>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Encoder
	 * @brief Concrete encoder implementation (name + feature set).
	 *
	 * Constructed only via Codec::Encoders().
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Encoder final {
		friend class Codec;
	public:
		/**
		 * Copy constructor.
		 */
		Encoder(const Encoder& other) = default;

		/**
		 * Move constructor.
		 */
		Encoder(Encoder&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~Encoder() noexcept = default;

		/**
		 * Copy assignment.
		 */
		Encoder& operator=(const Encoder& other) = default;

		/**
		 * Move assignment.
		 */
		Encoder& operator=(Encoder&& other) noexcept = default;

		/**
		 * @return Underlying codec id.
		 */
		int CodecID() const noexcept;

		/**
		 * @return Detected capability flags.
		 */
		const StormByte::Multimedia::Features& Features() const noexcept;

		/**
		 * @return Implementation name (e.g. "libx265").
		 */
		const std::string& Name() const noexcept;

	private:
		int m_id;										///< Codec id
		std::string m_name;								///< Implementation name
		StormByte::Multimedia::Features m_features;		///< Capabilities

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Encoder(int id, const std::string& name) noexcept;

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Encoder(int id, std::string&& name) noexcept;

		/**
		 * Merges registry + FFmpeg capability bits for @p name.
		 * @param name Implementation name.
		 * @return Feature set.
		 */
		static StormByte::Multimedia::Features DetectFeatures(const std::string_view& name) noexcept;
	};
}
