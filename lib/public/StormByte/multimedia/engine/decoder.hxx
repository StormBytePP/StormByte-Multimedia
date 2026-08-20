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
	 * @class Decoder
	 * @brief Concrete decoder implementation (name + feature set).
	 *
	 * Constructed only via Codec::Decoders().
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Decoder final {
		friend class Codec;
	public:
		/**
		 * Copy constructor.
		 */
		Decoder(const Decoder& other) = default;

		/**
		 * Move constructor.
		 */
		Decoder(Decoder&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~Decoder() noexcept = default;

		/**
		 * Copy assignment.
		 */
		Decoder& operator=(const Decoder& other) = default;

		/**
		 * Move assignment.
		 */
		Decoder& operator=(Decoder&& other) noexcept = default;

		/**
		 * @return Underlying codec id.
		 */
		int CodecID() const noexcept;

		/**
		 * @return Detected capability flags.
		 */
		const StormByte::Multimedia::Features& Features() const noexcept;

		/**
		 * @return Implementation name (e.g. "hevc_nvdec").
		 */
		const std::string& Name() const noexcept;

	private:
		int m_id;										///< Codec id
		std::string m_name;								///< Implementation name
		class Features m_features;						///< Capabilities

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Decoder(int id, const std::string& name) noexcept;

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Decoder(int id, std::string&& name) noexcept;

		/**
		 * Merges registry + FFmpeg capability bits for @p name.
		 * @param name Implementation name.
		 * @return Feature set.
		 */
		static StormByte::Multimedia::Features DetectFeatures(const std::string_view& name) noexcept;
	};
}
