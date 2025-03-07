#include <multimedia/codec/video.hxx>
#include <multimedia/property/resolution.hxx>
#include <multimedia/stream/stream.hxx>

#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Video
	 * @brief The class for video streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Stream {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The codec of the video.
			 * @param res The resolution of the video.
			 * @param lang The language of the video.
			 */
			Video(const Codec::Video& codec, const Property::Resolution& res, const std::optional<Property::Language>& lang = std::nullopt);

			/**
			 * @brief Default constructor.
			 * @param codec The codec of the video.
			 * @param res The resolution of the video.
			 * @param lang The language of the video.
			 */
			Video(Codec::Video&& codec, Property::Resolution&& res, std::optional<Property::Language>&& lang) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Video(const Video& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Video(Video&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Video& operator=(const Video& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Video& operator=(Video&& stream) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			~Video() noexcept override 					= default;

			/**
			 * @brief Get the size of the video.
			 * @return The size of the video.
			 */
			const Property::Resolution& 						GetResolution() const noexcept;

		private:
			Property::Resolution m_res; 						///< The resolution of the video.
	};
}