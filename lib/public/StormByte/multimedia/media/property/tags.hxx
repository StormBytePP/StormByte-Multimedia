#pragma once

#include <StormByte/alias.hxx>
#include <StormByte/multimedia/exception.hxx>

#include <string>
#include <unordered_map>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Media::Property {
	class STORMBYTE_MULTIMEDIA_PUBLIC Tags final {
		public:
			using iterator 			= typename std::unordered_map<std::string, std::string>::iterator;			///< Iterator type.
			using const_iterator	= typename std::unordered_map<std::string, std::string>::const_iterator;	///< Const iterator type.

			/**
			 * @brief Default constructor.
			 */
			Tags() noexcept 										= default;

			/**
			 * @brief Copy constructor.
			 * @param tag The tag to copy.
			 */
			Tags(const Tags& tag) noexcept 							= default;

			/**
			 * @brief Move constructor.
			 * @param tag The tag to move.
			 */
			Tags(Tags&& tag) noexcept 								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param tag The tag to copy.
			 * @return Reference to the assigned tag.
			 */
			Tags& operator=(const Tags& tag) noexcept 				= default;

			/**
			 * @brief Move assignment operator.
			 * @param tag The tag to move.
			 * @return Reference to the assigned tag.
			 */
			Tags& operator=(Tags&& tag) noexcept 					= default;

			/**
			 * @brief Destructor.
			 */
			~Tags() noexcept 										= default;

			/**
			 * @brief Gets the value of a tag.
			 * @param key The key of the tag.
			 * @return The value of the tag.
			 */
			std::string& 											operator[](const std::string& key) noexcept;

			/**
			 * @brief Gets the value of a tag.
			 * @param key The key of the tag.
			 * @return The value of the tag.
			 */
			StormByte::Expected<const std::string&, Exception> 		operator[](const std::string& key) const noexcept;

			/**
			 * @brief Removes the value of a tag.
			 * @param key The key of the tag.
			 */
			void 													Remove(const std::string& key) noexcept;

			/**
			 * @brief Gets tags size
			 * @return The size of the tags.
			 */
			std::size_t 											Size() const noexcept;

			/**
			 * @brief Gets iterator to the first tag.
			 * @return The iterator to the first tag.
			 */
			iterator 												begin() noexcept;

			/**
			 * @brief Gets const iterator to the first tag.
			 * @return The iterator to the first tag.
			 */
			const_iterator 											begin() const noexcept;

			/**
			 * @brief Gets iterator past the last tag.
			 * @return The iterator past the last tag.
			 */
			iterator 												end() noexcept;

			/**
			 * @brief Gets const iterator past the last tag.
			 * @return The iterator past the last tag.
			 */
			const_iterator 											end() const noexcept;

		private:
			std::unordered_map<std::string, std::string> m_tags;	///< The tags.
	};
}