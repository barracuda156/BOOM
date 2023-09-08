#ifndef BOOM_MATH_FFT_HPP_
#define BOOM_MATH_FFT_HPP_
/*
  Copyright (C) 2005-2023 Steven L. Scott

  This library is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation; either version 2.1 of the License, or (at your option)
  any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License along
  with this library; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include "LinAlg/Vector.hpp"
#include <vector>
#include <complex>

namespace FFT {
  class RealConfig;
}  // namespace FFT

namespace BOOM {

  class FastFourierTransform {
   public:
    std::vector<std::complex<double>> transform(
        const Vector &time_domain) const;
    Vector inverse_transform(
        const std::vector<std::complex<double>> &frequency_domain) const;

    std::string print_config(int data_size, bool inverse) const;

   protected:
    // Reflect the sequence of complex numbers in the first half of the vector
    // to the second half.  The real component values match.  The imaginary
    // component matches after multiplying by -1.
    void reflect(std::vector<std::complex<double>> &freq) const;
  };

}  // namespace BOOM

#endif  //  BOOM_MATH_FFT_HPP_
