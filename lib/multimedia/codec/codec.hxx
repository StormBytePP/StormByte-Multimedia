#pragma once

#include <util/templates/clonable.hxx>
#include <multimedia/type.hxx>

#include <string>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	class STORMBYTE_MULTIMEDIA_PUBLIC Codec:public Util::Templates::Clonable<Codec> {
		public:
			/**
			 * @brief Default constructor.
			 * @param name The name of the Codec.
			 * @param type The type of the Codec.
			 */
			Codec(const std::string& name, const Type& type);

			/**
			 * @brief Default constructor.
			 * @param name The name of the Codec.
			 * @param type The type of the Codec.
			 */
			Codec(std::string&& name, const Type& type) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param codec The Codec to copy.
			 */
			Codec(const Codec& codec) 					= default;

			/**
			 * @brief Move constructor.
			 * @param codec The Codec to move.
			 */
			Codec(Codec&& codec) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param codec The Codec to copy.
			 * @return The copied Codec.
			 */
			Codec& operator=(const Codec& codec) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param codec The Codec to move.
			 * @return The moved Codec.
			 */
			Codec& operator=(Codec&& codec) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Codec() noexcept					= 0;

			/**
			 * @brief Get the name of the Codec.
			 * @return The name of the Codec.
			 */
			const std::string&							GetName() const noexcept;

			/**
			 * @brief Get the type of the Codec.
			 * @return The type of the Codec.
			 */
			const Type&									GetType() const noexcept;

		protected:
			std::string m_name;							///< The name of the Codec.
			Type m_type;								///< The type of the Codec.
	};
}