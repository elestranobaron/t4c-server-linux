#ifndef DEFINE_GEN_REF
#define DEFINE_GEN_REF

#define PROHIBIT_ASSIGNMENT(classname) \
private: \
    classname(const classname&); \
    classname& operator=(const classname&);

#endif
