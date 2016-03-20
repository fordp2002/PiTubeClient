//#ifdef PROFILING
#if 1

extern void ProfileInit(void);
extern void ProfileAdd(DecodeData* This);
extern void ProfileDump(void);

#else

#define ProfileInit()
#define ProfileAdd(a)
#define ProfileDump()

#endif
