#pragma once

#include <string_view>
#include "Exception.h"

typedef unsigned char byte;

namespace NmeaFunctions
{

    /// <summary>
    ///        Converts a decimal value integer to a hex character.
    /// </summary>
    /// \param value [in] The decimal value to be convertet.
    /// \exception std::logic_error(), if <code> value > 15 </code>
    /// \return a character '0' .. '9', 'A' .. 'F' representing the hex value of the argument
    char hex2Char(size_t value);

    /// <summary>
    ///        Increments the iterator \c begin and updates the \c checksum.
    /// </summary>
    /// \param checkSum [in,out] The \c checksum is updated after the iterator has been incremented.
    /// \param begin [in,out]] The iterator to be incremented.
    /// \param end [in,out]] The iterator to one past the last legal character.
    /// \pre \code{.cpp} oldBegin == begin \endcode
    /// \pre \code{.cpp} oldCheckSum == checkSum \endcode
    /// \post \code{.cpp} begin != end \endcode
    /// \post \code{.cpp} begin == oldBegin + 1 \endcode
    /// \post \code{.cpp} isdefined(*begin) \endcode
    /// \post \code{.cpp} checkSum = oldCheckSum ^ *begin \endcode
    /// \exception Exception::E033 if <code> begin+1 == end </code>
    /// \exception Exception::E007 if <code> isundefined(*(begin+1)) </code>
    void incr(uint8_t& checkSum, const char*& begin, const char* end);

    /// <summary>
    ///        Increments the \c char iterator \c begin \c count times and updates the \c checksum accordingly.
    /// </summary>
    /// This is equivalent to calling <code>incr(checkSum, begin, end)</code> \c count times.
    /// \ref incr "See incr"
    void incr(uint8_t& checkSum, const char*& begin, const char* end, size_t count);

    /// <summary>
    ///        Takes a character and checks wether it is a valid one or not.
    /// </summary>
    /// \param ch [in] The character to be checked.
    /// \return <code> true </code> if ch is in the range '\\x20' .. '\\0x7F' and <code> !isreserved(ch) </code>, otherwise <code> false </code>
    bool isvalid(const char ch) noexcept;

    /// <summary>
    ///        Takes a character and checks whether it is a reserved one or not.
    /// </summary>
    /// \param ch [in] The character to be checked.
    /// \return true if ch is one of \<CR\> \<LF\> ! $ * , \ ^ ~ \<DEL\>, otherwise false
    bool isreserved(const char ch) noexcept;

    /// <summary>
    ///        Takes a character and checks whether it is a defined or not.
    /// </summary>
    ///    According to NMEA 0183 Version 4.00, 5.1.3 ch shall be a valid or a reserved character.
    /// \param ch [in] The character to be checked.
    /// \returns true if ch is a defined character, otherwise false
    inline
    bool isdefined(const char ch) noexcept
    {
        return ch <= 127 && (ch >= 32 || ch == '\x0A' || ch == '\x0D');
    }

    /// <summary>
    ///        Takes a character and checks whether it is a undefined or not.
    /// </summary>
    ///    According to NMEA 0183 Version 4.00, 5.1.3 ch shall be a valid or a reserved character.
    /// \param ch [in] The character to be checked.
    /// \returns true if ch is a undefined character, otherwise false
    inline
    bool isundefined(const char ch) noexcept
    {
        return !isdefined(ch);
    }

    /// <summary>
    ///        Takes a character and checks whether it is a six-bit binary representation.
    /// </summary>
    ///    NMEA 0183 Version 4.00, 6.2.4 defines the six-bit binary representation.
    /// \param ch [in] The character to be checked.
    /// \returns true if ch is a legal six-bit binary representation, otherwise false
    inline
    bool issixbit(const char ch) noexcept
    {
        return ((48 <= ch) && (ch <= 87)) || ((96 <= ch) && (ch <= 119));
    }

    /// <summary>
    ///   Check a talker id.
    /// </summary>
    /// \param ch1 [in] The first character in the talker id.
    /// \param ch2 [in] The second character in the talker id.
    /// \param errorIndication [in, out] This value is returned in the Exception.
    /// \post ch1 and ch2 are a uppercase letters or digits
    /// \exception Exception::E010 illegal character in talker id
    void checkTalkerId(char ch1, char ch2, const char* errorIndication = nullptr);

    /// <summary>
    ///   Check a proprietary talker id.
    /// </summary>
    /// \param ch1 [in] The first character in the talker id.
    /// \param ch2 [in] The second character in the talker id.
    /// \param ch3 [in] The third character in the talker id.
    /// \param errorIndication [in, out] This value is returned in the Exception.
    /// \post ch1, ch2 and ch3 are uppercase letters
    /// \exception Exception::E011 illegal character in talker id
    void checkProprietaryTalkerId(char ch1, char ch2, char ch3, const char* errorIndication = nullptr);

    /// <summary>
    ///   Check a sentence formatter.
    /// </summary>
    /// \param ch1 [in] The first character in sentence formatter.
    /// \param ch2 [in] The second character in sentence formatter.
    /// \param ch3 [in] The second character in sentence formatter.
    /// \param errorIndication [in, out] This value is returned in the Exception.
    /// \post ch1, ch2 and ch3 are a legal uppercase letters or digits
    /// \exception Exception::E012 illegal charackter in talker id
    void checkSentenceFormatter(char ch1, char ch2, char ch, const char* errorIndication = nullptr);

    /// <summary>
    ///   Checks that each character in the field is valid. 
    /// </summary>
    /// \param field [in] The characters to be checked.
    /// \pre \code{.cpp} for (ch : field) isdefined(ch) \endcode
    /// \post \code{.cpp} for (ch : field) isvalid(ch) \endcode
    /// \exception Exception::E008 Illegal character in data field
    /// \note If the precondition is false then it may return without an error
    void checkDataFieldCharacters(std::string_view field);

    /// <summary>
    ///   Checks that each character in the field is valid or a '^'. TBD
    /// </summary>
    /// \param field [in] The characters to be checked.
    /// \pre \code{.cpp} for (ch : field) isdefined(ch) \endcode
    /// \post \code{.cpp} for (ch : field) isvalid(ch) || ch=='^' \endcode
    /// \exception Exception::E008 Illegal character in data field
    void checkProprietaryDataFieldCharacters(std::string_view field);

    /// <summary>
    ///   Match a talker id.
    /// </summary>
    /// \param ch1 [in] The first character in the talker id.
    /// \param ch2 [in] The second character in the talker id.
    /// \post ch1 and ch2 are an approved talker id
    /// \param errorIndication [in, out] This value is returned in the Exception.
    /// \exception Exception::E005 Unknown talker ID
    void matchTalker(char ch1, char ch2, const char* errorIndication);

    /// <summary>
    ///   Match a talker id.
    /// </summary>
    /// \param ch1 [in] The first character in the proprietary talker id.
    /// \param ch2 [in] The second character in the proprietary talker id.
    /// \param ch3 [in] The third character in the proprietary talker id.
    /// \post ch1, ch2 and ch2 are an approved proprietary talker id
    /// \exception Exception::E009 Unknown sentence formatter
    void matchProprietaryTalker(char ch1, char ch2, char ch3);

    /// <summary>
    ///   Match a talker id.
    /// </summary>
    /// \param ch1 [in] The first character in the sentence formatter.
    /// \param ch2 [in] The second character in the sentence formatter.
    /// \param ch3 [in] The third character in the sentence formatter.
    /// \post ch1, ch2 and ch2 are an approved sentence formatter
    /// \exception Exception::E009 Unknown sentence formatter
    void matchSentenceFormatter(char ch1, char ch2, char ch3);

    void checkPositivInteger(std::string_view field);
    void checkIdentification(std::string_view field);
    void checkSentenceGrouping(std::string_view field);
};
