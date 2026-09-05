#pragma once

#include <StormByte/multimedia/media/codec.hxx>
#include <StormByte/multimedia/media/typedefs.hxx>

#include <functional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Media
 * @brief Public media types: codecs, registry and stream kinds.
 */
namespace StormByte::Multimedia::Media {
	namespace Tables::Codec {
		struct CodecDef;
	}

	/**
	 * @class Registry
	 * @brief Process-wide catalog of codecs.
	 *
	 * First Instance() loads the private tables and probes FFmpeg encoders.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Registry {
		public:
			/**
			 * @brief Copy is disabled.
			 */
			Registry(const Registry&) = delete;

			/**
			 * @brief Move is disabled.
			 */
			Registry(Registry&&) = delete;

			/**
			 * @brief Destructor.
			 */
			~Registry() noexcept = default;

			/**
			 * @brief Copy assignment is disabled.
			 * @return *this.
			 */
			Registry& operator=(const Registry&) = delete;

			/**
			 * @brief Move assignment is disabled.
			 * @return *this.
			 */
			Registry& operator=(Registry&&) = delete;

			/**
			 * @brief Process-wide instance.
			 * @return The singleton.
			 */
			static Registry& Instance() noexcept;

			/**
			 * @brief Codecs of one Type.
			 * @param type Kind to list.
			 * @return References into the registry storage.
			 */
			CodecRefs CodecList(Type type) const noexcept;

			/**
			 * @brief Looks up by StormByte name or FFmpeg id.
			 * @param name Key (`H.265` or `hevc`).
			 * @return Codec or CodecNotFoundException.
			 */
			ExpectedCodec FindCodec(std::string_view name) const noexcept;

		private:
			/**
			 * @struct NameHash
			 * @brief Transparent hasher for string_view map keys.
			 */
			struct NameHash {
				using is_transparent = void;	///< Enables heterogeneous lookup

				/**
				 * @brief Hashes a view.
				 * @param view Key.
				 * @return Hash.
				 */
				std::size_t operator()(std::string_view view) const noexcept {
					return std::hash<std::string_view>{}(view);
				}
			};

			/**
			 * @brief Loads tables and probes encoders.
			 */
			Registry() noexcept;

			/**
			 * @brief Reserves storage and loads every table.
			 */
			void Initialize() noexcept;

			/**
			 * @brief Inserts every row of @p table as @p type.
			 * @param type Media kind of the table.
			 * @param table Rows to load.
			 */
			void Load(Type type, std::span<const Tables::Codec::CodecDef> table) noexcept;

			/**
			 * @brief Inserts one codec and its FFmpeg ids.
			 * @param type Media kind.
			 * @param def Table row.
			 */
			void Add(Type type, const Tables::Codec::CodecDef& def) noexcept;

			std::vector<Codec> m_codecs;	///< Owned codec instances
			std::unordered_map<std::string_view, std::size_t, NameHash, std::equal_to<>> m_by_name;	///< Name / FFmpeg id → index
			std::unordered_map<Type, std::vector<std::size_t>> m_by_type;	///< Type → indices
	};
}
