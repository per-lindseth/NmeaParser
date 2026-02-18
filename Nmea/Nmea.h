#pragma once

#include <vector>
#include <string_view>
#include "SentenceType.h"
#include "Exception.h"
#include <string_view>

/// <summary>
///        An instance of Nmea is responsible for parsing a Nmea sentences and keeping the result
/// </summary>
class Nmea
{
public:
    Nmea();
    ~Nmea();

    /// <summary>
    ///        Parse the given Nmea sentence.
    /// </summary>
    /// \param [in] sentence The Nmea sentence to parsed.
    /// \post \code{.cpp} if OK then erroCode() == ErrorCode::E000 \endcode
    /// \post \code{.cpp} else erroCode() == some other error code \endcode 
    void parse(std::string_view sentence);

    /// <summary>
    ///        The error code yielding the last call to parse.
    /// </summary>
    ErrorCode errorCode() const;

    const char* indication() const { return m_Indication; }

private:
    enum class LineElementType { tag_block, sentence};
    struct TagBlockOrSentence
    {
        std::vector<std::string_view> m_Splitter;
        LineElementType               m_LineElementType;
        SentenceType                  m_SentenceType;
    };

    std::vector<TagBlockOrSentence> m_Line;

    ErrorCode    m_Error;
    const char*  m_Indication;

public:

    /// <summary>
    /// Parses the fixed structure of the given NMEA sentence.
    /// </summary>
    /// <param name="sentence">The NMEA sentence to be parsed.</param>
    /// <post>
    ///     m_Splitter.size() >= 2
    ///
    ///     // Header field
    ///     m_Splitter[0] contains the header field.
    ///
    ///     // Data fields
    ///     m_Splitter[i] contains a data field,
    ///     where i = 1 .. m_Splitter.size() - 2.
    ///
    ///     // Checksum field
    ///     m_Splitter[m_Splitter.size() - 1] contains the checksum field.
    ///
    ///     // Data fields consist only of defined characters
    ///     for (const auto& s : m_Splitter[0] .. m_Splitter[m_Splitter.size() - 2])
    ///         for (char ch : s)
    ///             isdefined(ch) == true
    ///
    ///     // Header must start with '$' or '!'
    ///     m_Splitter[0][0] == '$' || m_Splitter[0][0] == '!'
    ///
    ///     // Data fields must not start with ','
    ///     for (size_t i = 1; i < m_Splitter.size() - 1; ++i)
    ///         m_Splitter[i][0] != ','
    ///
    ///     // Checksum field must start with '*'
    ///     m_Splitter[m_Splitter.size() - 1][0] == '*'
    ///
    ///     // Checksum must be valid
    ///     checksum_is_correct == true
    /// </post>
    /// <exception cref="Exception">
    /// Thrown if the sentence structure is invalid or the checksum is incorrect.
    /// </exception>
    void parseMainStructure(std::string_view sentence);
    
    TagBlockOrSentence parseMainStructureTagBlockOrSentence(const char*& begin, const char* end);

    /// <summary>
    /// Parse the fixed internal structure of the address field and the data fields.
    /// </summary>
    /// \pre parseMainStructure(sentence), for some NMEA sentence
    /// \post The address field and datafields contains only legal characters
    /// \exception Exception
    void parseGeneralContents();

    void parseSpecificContents();

    /// <summary>
    /// Append a field to the m_Splitter. 
    /// </summary>
    void appendSpan(std::vector <std::string_view>& splitter, const char* begin, const char* end);
};
