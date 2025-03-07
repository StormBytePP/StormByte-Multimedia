#include <multimedia/type.hxx>
#include <multimedia/property/language.hxx>

#include <optional>
#include <string>

/**
 * @namespace Stream
 * @brief The namespace for all streams.
 */
namespace StormByte::Multimedia::Stream {
	/**
	 * @class Stream
	 * @brief The base class for streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @brief Default constructor.
			 * @param type The type of the Stream.
			 * @param lang The language of the Stream.
			 */
			Stream(const Type& type, const std::optional<Property::Language>& lang = std::nullopt);

			/**
			 * @brief Default constructor.
			 * @param type The type of the Stream.
			 * @param lang The language of the Stream.
			 */
			Stream(const Type& type, std::optional<Property::Language>&& lang) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param stream The stream to copy.
			 */
			Stream(const Stream& stream) 					= default;

			/**
			 * @brief Move constructor.
			 * @param stream The stream to move.
			 */
			Stream(Stream&& stream) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The stream to copy.
			 * @return The copied stream.
			 */
			Stream& operator=(const Stream& stream) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The stream to move.
			 * @return The moved stream.
			 */
			Stream& operator=(Stream&& stream) noexcept		= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Stream() noexcept						= 0;

			/**
			 * @brief Get the type of the Codec.
			 * @return The type of the Codec.
			 */
			const Type& GetType() const noexcept;

			/**
			 * @brief Get the language of the Stream.
			 * @return The language of the Stream.
			 */
			const std::optional<Property::Language>& 		GetLanguage() const noexcept;

			/**
			 * @brief Set the language of the Stream.
			 * @param lang The language of the Stream.
			 */
			void 											SetLanguage(const Property::Language& lang) noexcept;

			/**
			 * @brief Set the language of the Stream.
			 * @param lang The language of the Stream.
			 */
			void 											SetLanguage(Property::Language&& lang) noexcept;

		protected:
			Type m_type; 									///< The type of the Codec.
			std::optional<Property::Language> m_lang;		///< The language of the Stream.
	};
}