/* Copyright 2022-2025, Mansour Moufid <mansourmoufid@gmail.com> */

/*
 * This file is part of Aluminium Library.
 *
 * Aluminium Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Aluminium Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Aluminium Library. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <float.h> // FLT_MANT_DIG
#include <stddef.h>

#define FLT_MAX_ZERO_EXP (((uint32_t) 1 << FLT_MANT_DIG) - 1)

#define SIZE_MAX_SQRT (((size_t) 1) << (8 * sizeof (size_t) / 2))

static inline
size_t
_al_calc_next_multiple(size_t x, size_t n)
{
    return (x / n + 1) * n;
}
