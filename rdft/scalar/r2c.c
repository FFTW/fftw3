/*
 * Copyright (c) 2003, 2007-11 Matteo Frigo
 * Copyright (c) 2003, 2007-11 Massachusetts Institute of Technology
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "codelet-rdft.h"

#include "r2cf.h"
const kr2c_genus GENUS = { R2HC, 1 };
#undef GENUS

#include "r2cfII.h"
const kr2c_genus GENUS = { R2HCII, 1 };
#undef GENUS

#include "r2cb.h"
const kr2c_genus GENUS = { HC2R, 1 };
#undef GENUS

#include "r2cbIII.h"
const kr2c_genus GENUS = { HC2RIII, 1 };
#undef GENUS
