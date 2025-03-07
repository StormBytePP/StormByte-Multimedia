#include <multimedia/stream/stream.hxx>

#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Subtitle
	 * @brief The class for subtitle streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Subtitle final: public Stream {
		public:
			/**
			 * @brief Default constructor.
			 */
			Subtitle();

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Subtitle(const Subtitle& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Subtitle(Subtitle&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Subtitle& operator=(const Subtitle& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Subtitle& operator=(Subtitle&& stream) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			~Subtitle() noexcept override 						= default;
	};
}