#ifndef JLE_COMPILE_HELPER
#define JLE_COMPILE_HELPER

// These macros are used to reduce compile times by having the serialisation function from cereal
// only be compiled once for the archives used in jle, in one .cpp file, and not spread out across multiple.

// Note: this is not needed, it is only used to reduce compile time.

#ifdef BUILD_EDITOR

namespace cereal
{
    class JSONOutputArchive;
    class JSONInputArchive;
    class jleImGuiCerealArchive;
    class jleImGuiCerealArchiveInternal;
}; // namespace cereal

#define JLE_EXTERN_TEMPLATE_CEREAL_H(object)                                                                           \
    extern template void object::serialize<cereal::jleImGuiCerealArchive>(cereal::jleImGuiCerealArchive &);            \
    extern template void object::serialize<cereal::jleImGuiCerealArchiveInternal>(                                     \
        cereal::jleImGuiCerealArchiveInternal &);                                                                      \
    extern template void object::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);                    \
    extern template void object::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);

#define JLE_EXTERN_TEMPLATE_CEREAL_CPP(object)                                                                         \
    template void object::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);                           \
    template void object::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);                             \
    template void object::serialize<cereal::jleImGuiCerealArchive>(cereal::jleImGuiCerealArchive &);                   \
    template void object::serialize<cereal::jleImGuiCerealArchiveInternal>(cereal::jleImGuiCerealArchiveInternal &);

#else

namespace cereal
{
    class JSONOutputArchive;
    class JSONInputArchive;
}; // namespace cereal

#define JLE_EXTERN_TEMPLATE_CEREAL_H(object)                                                                           \
    extern template void object::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);                    \
    extern template void object::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);

#define JLE_EXTERN_TEMPLATE_CEREAL_CPP(object)                                                                         \
    template void object::serialize<cereal::JSONOutputArchive>(cereal::JSONOutputArchive &);                           \
    template void object::serialize<cereal::JSONInputArchive>(cereal::JSONInputArchive &);

#endif

#endif // JLE_COMPILE_HELPER