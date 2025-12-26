#ifndef BFACE_BFACE_EXPORT_H
#define BFACE_BFACE_EXPORT_H
/// based on https://gcc.gnu.org/wiki/Visibility
/// BFACE_API & BFACE_LOCAL & BFACE_DEPRECATED usage:
/// if current api is used only in the library, mark it as BFACE_LOCAL
/// if current api is exported to used outside the library, mark it as BFACE_API
/// if current api is outdated and not recommanded, mark it as BFACE_DEPRECATED
#if __GNUC__ >= 4
/// Describe those functions or classes can be called outside the module.
#define BFACE_API __attribute__((visibility ("default")))
/// Describe those functions or classes only used inside the module
#define BFACE_LOCAL __attribute__((visibility ("hidden")))
/// Describe those functions or classes are expected to be removed in future.
#define BFACE_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
/// Dummy definition
#define BFACE_API
/// Dummy definition
#define BFACE_LOCAL
/// Dummy definition
#define BFACE_DEPRECATED(msg)
#endif  /// __GNUC__ >= 4

#endif /// BFACE_BFACE_EXPORT_H
