#include <multimedia/property/duration.hxx>
#include <multimedia/stream/stream.hxx>

#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Audio
	 * @brief The class for audio streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final: public Stream {
		public:
			/**
			 * @brief Default constructor.
			 * @param dur The duration of the audio.
			 * @param lang The language of the audio.
			 */
			Audio(const Property::Duration& dur, const std::optional<Property::Language>& lang = std::nullopt);

			/**
			 * @brief Default constructor.
			 * @param dur The duration of the audio.
			 * @param lang The language of the audio.
			 */
			Audio(Property::Duration&& dur, std::optional<Property::Language>&& lang) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Audio(const Audio& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Audio(Audio&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Audio& operator=(const Audio& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Audio& operator=(Audio&& stream) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			~Audio() noexcept override 					= default;

			/**
			 * @brief Get the duration of the audio.
			 * @return The duration of the audio.
			 */
			const Property::Duration& 					GetDuration() const noexcept;

		private:
			Property::Duration m_dur;					///< The duration of the audio.
	};
}