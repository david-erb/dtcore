// helpers for the demo driver main programs

typedef struct dtcore_demos_demo_t
{
    void (*func)(void);
    const char* name;
} dtcore_demos_demo_t;

#define DTCORE_DEMOS_ADD(demo)                                                                                                 \
    extern void demo(void);                                                                                                    \
    demos[ndemos].func = (void*)demo;                                                                                          \
    demos[ndemos].name = #demo;                                                                                                \
    ndemos++;
