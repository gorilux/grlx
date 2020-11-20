#ifndef GRLX_UTILITY_LITERAL_H
#define GRLX_UTILITY_LITERAL_H

#include "../details/config.h"

#ifdef CLX_USE_WCHAR
namespace grlx {
    /* --------------------------------------------------------------------- */
    //  basic_literals
    /* --------------------------------------------------------------------- */
    template <class CharT, class WideT>
    class basic_literals {
    public:
        typedef CharT char_type;
        typedef WideT wide_type;

        basic_literals(CharT c, WideT wc) :
            c_(c), wc_(wc) {}

        operator char_type() const { return c_; }
        operator wide_type() const { return wc_; }

    private:
        char_type c_;
        wide_type wc_;
    };

    typedef basic_literals<const char, const wchar_t> char_literals;
    typedef basic_literals<const char*, const wchar_t*> string_literals;

    /* --------------------------------------------------------------------- */
    //  generate_char_literals
    /* --------------------------------------------------------------------- */
    inline char_literals generate_literals(const char c, const wchar_t wc) {
        return char_literals(c, wc);
    }

    inline string_literals generate_literals(const char* c, const wchar_t* wc) {
        return string_literals(c, wc);
    }
}
#define LITERAL(X) clx::generate_literals(X, L##X)
#else
#define LITERAL(X) X
#endif // CLX_USE_WCHAR


#endif // GRLX_UTILITY_LITERAL_H

