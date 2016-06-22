#ifndef DEFS_H
#define DEFS_H

#include "..\deps\include\SDL\SDL.h"

// SIGNED DATA TYPES
// 2^8 = 256 / 2 = -128 to 127
// base^bits = num / 2 = x: -x to x-1
typedef	char		i8;		// 1 byte (8 bits)
typedef short		i16;	// 2 bytes
typedef int			i32;	// 4 bytes
typedef long long	i64;	// 8 bytes

typedef float		r32;	// 4 bytes
typedef double		r64;	// 8 bytes

// UNSIGNED
// 2^8 = 256 = 0 to 255;
typedef	unsigned char		u8;		// 1 bytes
typedef unsigned short		u16;	// 2 bytes
typedef unsigned int		u32;	// 4 bytes
typedef unsigned long long	u64;	// 8 bytes

//returns size in bits
#define Kilobytes(size) ( size * 1024 )
#define Megabytes(size) ( Kilobytes(size) * 1024 )
#define Gigabytes(size) ( Megabytes(size) * 1024 )

#endif