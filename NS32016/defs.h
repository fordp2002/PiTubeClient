#define BYTE_SWAP

#if defined(WIN32) && !defined(BEEBEM)
#define SWAP16 _byteswap_ushort
#define SWAP32 _byteswap_ulong
#else
#define SWAP16 __builtin_bswap16
#define SWAP32 __builtin_bswap32
#endif
