//# Vector_tmpl.cc: Explicit Vector template instantiations
//# Copyright (C) 2015
//# Associated Universities, Inc. Washington DC, USA,
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: Vector.h 21545 2015-01-22 19:36:35Z gervandiepen $

//# Includes
#include <casacore/casa/Arrays/Vector.h>

//# Instantiate extern templates for often used types.
#ifdef AIPS_CXX11
namespace casacore {
  template class Vector<Bool>;
  template class Vector<Char>;
  template class Vector<Short>;
  template class Vector<uShort>;
  template class Vector<Int>;
  template class Vector<uInt>;
  template class Vector<Int64>;
  template class Vector<Float>;
  template class Vector<Double>;
  template class Vector<Complex>;
  template class Vector<DComplex>;
  template class Vector<String>;
}
#endif
