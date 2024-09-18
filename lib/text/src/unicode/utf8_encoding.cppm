export module unicode.utf8_encoding;

import <string>;

#include "text/text_common.h"

namespace unicode {
export class TEXT_API UTF8Encoding: public TextEncoding
/// UTF-8 text encoding, as defined in RFC 2279.
{
 public:
    UTF8Encoding();
    ~UTF8Encoding();
    const char* canonicalName() const;
    bool isA(const std::string& encodingName) const;
    const CharacterMap& characterMap() const;
    int convert(const unsigned char* bytes) const;
    int convert(int ch, unsigned char* bytes, int length) const;
    int queryConvert(const unsigned char* bytes, int length) const;
    int sequenceLength(const unsigned char* bytes, int length) const;

    static bool isLegal(const unsigned char *bytes, int length);
    /// Utility routine to tell whether a sequence of bytes is legal UTF-8.
    /// This must be called with the length pre-determined by the first byte.
    /// The sequence is illegal right away if there aren't enough bytes
    /// available. If presented with a length > 4, this function returns false.
    /// The Unicode definition of UTF-8 goes up to 4-byte sequences.
    ///
    /// Adapted from ftp://ftp.unicode.org/Public/PROGRAMS/CVTUTF/ConvertUTF.c
    /// Copyright 2001-2004 Unicode, Inc.

 private:
    static const char* _names[];
    static const CharacterMap _charMap;
};
}// unicode

// module unicode.utf8_encoding;
