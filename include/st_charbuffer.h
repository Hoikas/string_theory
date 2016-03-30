/*  This file is part of string_theory.

    string_theory is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    string_theory is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with string_theory.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef _ST_CHARBUFFER_H
#define _ST_CHARBUFFER_H

#include "st_config.h"

#include <cstddef>
#ifdef ST_HAVE_RVALUE_MOVE
#  include <utility>    // For std::move
#endif

namespace _ST_PRIVATE
{
    // Avoid including <cstring> in this header...
    ST_EXPORT void _zero_buffer(void *buffer, size_t size);
    ST_EXPORT void _fill_buffer(void *buffer, int ch, size_t count);
    ST_EXPORT void _copy_buffer(void *dest, const void *src, size_t size);
}

namespace ST
{
    template<typename char_T>
    class ST_EXPORT buffer
    {
    private:
        union
        {
            char_T *m_ref;
            char_T m_data[ST_SHORT_STRING_LEN];
        };
        size_t m_size;

        inline bool is_reffed() const ST_NOEXCEPT
        {
            return m_size >= ST_SHORT_STRING_LEN;
        }

        struct _scope_deleter
        {
            // Useful for deleting the buffer at the *end* of a function,
            // even though we must capture it at the beginning, in case
            // the user does something silly like assign a buffer to itself.
            char_T *m_buffer;

            _scope_deleter(buffer<char_T> *self)
            {
                m_buffer = self->is_reffed() ? self->m_ref : nullptr;
            }

            ~_scope_deleter()
            {
                delete[] m_buffer;
            }
        };

    public:
        buffer() ST_NOEXCEPT
            : m_size()
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
        }

        buffer(const buffer<char_T> &copy)
            : m_size(copy.m_size)
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
            if (is_reffed()) {
                m_ref = new char_T[m_size + 1];
                _ST_PRIVATE::_copy_buffer(m_ref, copy.m_ref, m_size * sizeof(char_T));
                m_ref[m_size] = 0;
            } else {
                _ST_PRIVATE::_copy_buffer(m_data, copy.m_data, sizeof(m_data));
            }
        }

#ifdef ST_HAVE_RVALUE_MOVE
        buffer(buffer<char_T> &&move) ST_NOEXCEPT
            : m_size(std::move(move.m_size))
        {
            _ST_PRIVATE::_copy_buffer(m_data, move.m_data, sizeof(m_data));
            move.m_size = 0;
        }
#endif

        buffer(const char_T *data, size_t size)
            : m_size(size)
        {
            _ST_PRIVATE::_zero_buffer(m_data, sizeof(m_data));
            char_T *buffer = is_reffed() ? (m_ref = new char_T[m_size + 1]) : m_data;
            _ST_PRIVATE::_copy_buffer(buffer, data, m_size * sizeof(char_T));
            buffer[m_size] = 0;
        }

        ~buffer<char_T>() ST_NOEXCEPT
        {
            if (is_reffed())
                delete[] m_ref;
        }

        buffer<char_T> &operator=(const buffer<char_T> &copy) ST_NOEXCEPT
        {
            _scope_deleter unref(this);
            m_size = copy.m_size;
            if (is_reffed()) {
                m_ref = new char_T[m_size + 1];
                _ST_PRIVATE::_copy_buffer(m_ref, copy.m_ref, m_size * sizeof(char_T));
                m_ref[m_size] = 0;
            } else {
                _ST_PRIVATE::_copy_buffer(m_data, copy.m_data, sizeof(m_data));
            }
            return *this;
        }

#ifdef ST_HAVE_RVALUE_MOVE
        buffer<char_T> &operator=(buffer<char_T> &&move) ST_NOEXCEPT
        {
            _scope_deleter unref(this);
            m_size = std::move(move.m_size);
            _ST_PRIVATE::_copy_buffer(m_data, move.m_data, sizeof(m_data));
            move.m_size = 0;
            return *this;
        }
#endif

        const char_T *data() const ST_NOEXCEPT
        {
            return is_reffed() ? m_ref : m_data;
        }

        size_t size() const ST_NOEXCEPT { return m_size; }

        operator const char_T *() const ST_NOEXCEPT { return data(); }

        char_T *create_writable_buffer(size_t size)
        {
            if (is_reffed())
                delete[] m_ref;

            m_size = size;
            if (is_reffed())
                return m_ref = new char_T[m_size + 1];
            else
                return m_data;
        }

        static inline size_t strlen(const char_T *buffer)
        {
            if (!buffer)
                return 0;

            size_t length = 0;
            for ( ; *buffer++; ++length)
                ;
            return length;
        }
    };

    typedef buffer<char>        char_buffer;
    typedef buffer<wchar_t>     wchar_buffer;
    typedef buffer<char16_t>    utf16_buffer;
    typedef buffer<char32_t>    utf32_buffer;
} // namespace ST

#endif // _ST_CHARBUFFER_H
