#pragma once

#include <StormByte/type_traits.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	/**
	 * @class AVPointer
	 * @brief Wrapper class for FFmpeg pointers.
	 *
	 * This class provides a simple wrapper around FFmpeg pointers to ensure type safety
	 * and provide basic functionality.
	 *
	 * @tparam Ptr The type of the FFmpeg pointer.
	 */
	template<StormByte::Type::Pointer Ptr>
	class STORMBYTE_MULTIMEDIA_ADVANCED AVPointer {
		public:
			/**
			 * @brief Default constructor (delete).
			 */
			constexpr AVPointer() noexcept 									= delete;

			/**
			 * @brief Copy constructor (delete).
			 * @param other The other AVPointer to copy from.
			 */
			constexpr AVPointer(const AVPointer&) noexcept 					= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other AVPointer to move from.
			 */
			constexpr AVPointer(AVPointer&& other) noexcept
			:m_ptr(other.m_ptr) {
				other.m_ptr = nullptr;
			}

			/**
			 * @brief Destructor.
			 * @note Does not free the underlying FFmpeg pointer, that is responsibility of the derived class.
			 */
			virtual ~AVPointer() noexcept 									= default;

			/**
			 * @brief Copy assignment operator (delete).
			 * @param other The other AVPointer to copy from.
			 * @return Reference to this AVPointer.
			 */
			constexpr AVPointer& operator=(const AVPointer&) noexcept 		= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other AVPointer to move from.
			 * @return Reference to this AVPointer.
			 */
			constexpr AVPointer& operator=(AVPointer&& other) noexcept {
				if (this != &other) {
					Free();
					m_ptr = other.m_ptr;
					other.m_ptr = nullptr;
				}
				return *this;
			}

			/**
			 * @brief Get the underlying FFmpeg pointer.
			 * @return Ptr The underlying FFmpeg pointer as a const pointer.
			 */
			constexpr const Ptr 											Get() const noexcept {
				return m_ptr;
			}

		protected:
			Ptr m_ptr = nullptr;											///< The underlying FFmpeg pointer.

			/**
			 * @brief Constructor with pointer.
			 * @param ptr The FFmpeg pointer to wrap.
			 */
			explicit constexpr AVPointer(Ptr ptr) noexcept
			:m_ptr(ptr) {}

			/**
			 * @brief Pure virtual function to free the underlying FFmpeg pointer.
			 *
			 * This function must be implemented by derived classes to properly free
			 * the FFmpeg pointer when needed.
			 */
			virtual void 													Free() noexcept = 0;
	};
}