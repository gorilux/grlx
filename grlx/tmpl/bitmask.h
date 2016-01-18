#ifndef GRLX_TMPL_BITMASK_H
#define GRLX_TMPL_BITMASK_H

#include <cstddef>
#include <climits>

namespace grlx {
    namespace tmpl {
        /* ----------------------------------------------------------------- */
        //  lower_mask
        /* ----------------------------------------------------------------- */
        template <std::size_t Bits, typename T = std::size_t>
        struct lower_mask {
            typedef T type;
            static const type value = (type(1) << Bits) - 1;
        };

        /* ----------------------------------------------------------------- */
        //  upper_mask
        /* ----------------------------------------------------------------- */
        template <std::size_t Bits, typename T = std::size_t>
        struct upper_mask {
            typedef T type;
            static const type value = ~lower_mask<Bits, T>::value;
        };
    } // namespace mpl
} // namespace clx



#endif // BITMASK_H

