#include "Nmea.h"

#include <exception>
#include <iostream>
#include <string>

#include "ErrorCodes.h"
#include "NmeaFunctions.h"
#include "HardCodedMessages.h"
#include "Exception.h"

using namespace std;
Nmea::Nmea() :
    m_Line(),
    m_Error(ErrorCode::E000),
    m_Indication(nullptr)
{
}

Nmea::~Nmea()
{
}

void Nmea::parse(std::string_view line)
{
    try
    {
        parseMainStructure(line);
        parseGeneralContents();
        parseSpecificContents();
    }
    catch (const ErrorCode& e)
    {
        m_Error = e;
    }
    catch (const Exception& e)
    {
        m_Error = e.errorCode;
        m_Indication = e.indication;
    }
    catch (...)
    {
    }
}

ErrorCode Nmea::errorCode() const
{
    return m_Error;
}

void Nmea::parseMainStructure(std::string_view line)
{
    // Check the arguments for empty line
    if (line.length() == 0)
        throw logic_error("Empty line");

    auto begin = &line[0];
    auto end = begin + line.size();

    // Start with no m_Error
    m_Error = ErrorCode::E000;

    // Find the first character in the sentence
    for (; true; ++begin)
    {
        if (begin == end)
            throw Exception(ErrorCode::E033);

        if ((*begin == '$') || (*begin == '!') || (*begin == '\\'))
            break;
    }

    // Parse Tag Blocks
    while (*begin == '\\')
    {
        m_Line.emplace_back(parseMainStructureTagBlockOrSentence(begin, end));

        if (*begin++ != '\\')
            throw Exception(ErrorCode::E026);
    }

    // Parse Sentence
    if ((*begin == '$') || (*begin == '!'))
        m_Line.emplace_back(parseMainStructureTagBlockOrSentence(begin, end));

    // Check for CR LF
    if (begin >= end || *begin != '\r')
        throw Exception(ErrorCode::E024, begin);
    if (++begin >= end || *begin != '\n')
        throw Exception(ErrorCode::E025, begin);
}

Nmea::TagBlockOrSentence Nmea::parseMainStructureTagBlockOrSentence(const char*& begin, const char* end)
{
    TagBlockOrSentence tagBlockOrSentence;

    // Initialize some variables
    auto start = begin;
    uint8_t checkSum = 0;

    // Process the address field
    if (*begin == '\\')
    {
        tagBlockOrSentence.m_LineElementType = LineElementType::tag_block;
    }
    else if (*begin == '!')
    {
        tagBlockOrSentence.m_LineElementType = LineElementType::sentence;
        tagBlockOrSentence.m_SentenceType = SentenceType::encapsulated;

        // Skip address field
        NmeaFunctions::incr(checkSum, begin, end, 5);
    }
    else if (*begin == '$')
    {
        tagBlockOrSentence.m_LineElementType = LineElementType::sentence;
        NmeaFunctions::incr(checkSum, begin, end, 1);

        if (*begin == 'P')
        {
            tagBlockOrSentence.m_SentenceType = SentenceType::proprietary;
            // Skip address field
            NmeaFunctions::incr(checkSum, begin, end, 3);
        }
        else
        {
            // Skip address field
            NmeaFunctions::incr(checkSum, begin, end, 5);

            if (*(begin - 1) == 'Q')
            {
                tagBlockOrSentence.m_SentenceType = SentenceType::query;
            }
            else
            {
                tagBlockOrSentence.m_SentenceType = SentenceType::parametric;
            }
        }
    }
    else
        throw Exception(ErrorCode::E001, &(begin[0]));


    // Process the data fields
#pragma warning( disable : 4127)
    while (true) {
        if (end <= begin)
            throw Exception(ErrorCode::E003, &(begin[0]));
        else if (*begin == ',')
        {
            appendSpan(tagBlockOrSentence.m_Splitter, start, begin);
            start = begin;
        }
        else if (*begin == '*')
        {
            checkSum ^= '*'; // Undo the XOR with *
            appendSpan(tagBlockOrSentence.m_Splitter, start, begin);
            start = begin;
            break;
        }
        NmeaFunctions::incr(checkSum, begin, end, 1);
    }

    // Skip checksum field
    begin += 3;
    if (end < begin)
        throw Exception(ErrorCode::E003, &(begin[0]));

    appendSpan(tagBlockOrSentence.m_Splitter, start, begin);

    // Check total length
    // Ref. NMEA 0183 V.4.00 5.2.4
    auto headerField{ tagBlockOrSentence.m_Splitter[0] };
    auto checksumField{ tagBlockOrSentence.m_Splitter[tagBlockOrSentence.m_Splitter.size() - 1] };
    if ((&(checksumField[checksumField.size() - 1]) - &(headerField[0]))  > 79)
        throw Exception(ErrorCode::E023, &(headerField[0]));

    // Check Checksum
    if ((checksumField[1] != NmeaFunctions::hex2Char(checkSum >> 4)) &&
        (checksumField[2] != NmeaFunctions::hex2Char(checkSum & 0x0F)))
    {
        throw Exception(ErrorCode::E004, &(checksumField[0]));
    }

    return tagBlockOrSentence;
}

void Nmea::parseGeneralContents()
{
    for (auto& tagBlockOrSentence : m_Line)
    {
        const std::vector<std::string_view> splitter{ tagBlockOrSentence.m_Splitter };

        if (tagBlockOrSentence.m_LineElementType == LineElementType::tag_block)
        {
            // Check data field characters
            for (size_t i = 0; i < splitter.size() - 1; ++i)
            {
                auto& tagField = splitter[i];
                if (tagField[1] != ':')
                    throw Exception(ErrorCode::E028, &(tagField[1]));

                switch (tagField[0])
                {
                case 'c':
                {
                    std::string_view field{ &tagField[2], tagField.size() - 2 };
                    NmeaFunctions::checkPositivInteger(field);
                    break;
                }
                case 'd':
                {
                    std::string_view field{ &tagField[2], tagField.size() - 2 };
                    NmeaFunctions::checkIdentification(field);
                    break;
                }
                case 'g':
                {
                    std::string_view field{ &tagField[2], tagField.size() - 2 };
                    NmeaFunctions::checkSentenceGrouping(field);
                    break;
                }
                case 'n':
                {
                    std::string_view field{ &tagField[2], tagField.size() - 2 };
                    NmeaFunctions::checkPositivInteger(field);
                    break;
                }
                case 'r':
                {
                    std::string_view field{ &tagField[2], tagField.size() - 2 };
                    NmeaFunctions::checkPositivInteger(field);
                    break;
                }
                case 's':
                {
                    std::string_view field{ &tagField[2], tagField.size() - 2 };
                    NmeaFunctions::checkIdentification(field);
                    break;
                }
                case 't':
                    // No check necessary. The field is already checked that it contains only valid characters
                    break;
                default:
                    throw Exception(ErrorCode::E027, &(tagField[0]));
                }
            }
        }
        else
        {
            auto sentenceType = tagBlockOrSentence.m_SentenceType;

            // Check data field characters
            switch (sentenceType)
            {
            case SentenceType::parametric:
            case SentenceType::encapsulated:
            {
                // Check address field
                auto headerField{ splitter[0] };
                NmeaFunctions::checkTalkerId(headerField[1], headerField[2], &(headerField[1]));
                NmeaFunctions::checkSentenceFormatter(headerField[3], headerField[4], headerField[5], &(headerField[3]));

                // Check data field characters
                for (size_t i = 1; i < splitter.size() - 1; ++i)
                    NmeaFunctions::checkDataFieldCharacters(splitter[i]);

                break;
            }
            case SentenceType::query:
            {
                // Check address field
                auto headerField{ splitter[0] };
                NmeaFunctions::checkTalkerId(headerField[1], headerField[2], &(headerField[1]));
                NmeaFunctions::checkTalkerId(headerField[3], headerField[4], &(headerField[3]));

                // Check m_Splitter.size() == 3

                // Check data field
                auto field{ splitter[1] };
                NmeaFunctions::checkSentenceFormatter(field[0], field[1], field[2], &(field[0]));
                break;
            }
            case SentenceType::proprietary:
            {
                // Check address field
                auto headerField{ splitter[0] };
                NmeaFunctions::checkProprietaryTalkerId(headerField[2], headerField[3], headerField[4], &(headerField[2]));

                // Check data fields
                for (size_t i = 1; i < splitter.size() - 1; ++i)
                    NmeaFunctions::checkProprietaryDataFieldCharacters(splitter[i]);
                break;
            }
            }
        }
    }
}

void Nmea::parseSpecificContents()
{
    HardCodedMessages hardCodedMessages;

    for (auto& tagBlockOrSentence : m_Line)
    {
        const std::vector<std::string_view> splitter{ tagBlockOrSentence.m_Splitter };

        if (tagBlockOrSentence.m_LineElementType == LineElementType::tag_block)
        {

        }
        else
        {
            auto sentenceType = tagBlockOrSentence.m_SentenceType;

            // Match Address field
            switch (sentenceType)
            {
            case SentenceType::parametric:
            case SentenceType::encapsulated:
            {
                auto& headerField{ splitter[0] };

                // Check talker id
                NmeaFunctions::matchTalker(headerField[1], headerField[2], &(headerField[1])); // Skip the $ or !

                                                                                               // Check talker sentence formatter
                const char formatter[4]{ headerField[3] , headerField[4] , headerField[5] , '\0' };

                //NmeaFunctions::matchSentenceFormatter(headerField[3], headerField[4], headerField[5]);

                const auto& sentence(hardCodedMessages.m_Sentences.find(formatter));

                if (sentence != hardCodedMessages.m_Sentences.end())
                    sentence->second->parse(splitter);
                else
                    throw Exception(ErrorCode::E009, &(headerField[0]));
            }
            break;
            case SentenceType::query:
            {
                auto& headerField{ splitter[0] };

                // Check talker id1
                NmeaFunctions::matchTalker(headerField[1], headerField[2], &(headerField[1]));

                // Check talker id2
                NmeaFunctions::matchTalker(headerField[3], headerField[4], &(headerField[3]));

                // Check talker sentence formatter
                auto& field{ splitter[1] };
                NmeaFunctions::matchSentenceFormatter(field[0], field[1], field[2]);
            }
            break;
            case SentenceType::proprietary:
                break;
            }
        }
    }
}

void Nmea::appendSpan(std::vector<std::string_view>& splitter, const char* begin, const char* end)
{
    // Is it a datafield?
    if (*begin == ',' || *begin == '\\')
    {
        // Yes, is it only containing a ','?
        if ((end - begin) < 2)
        {
            // Yes, insert an empty field
            splitter.emplace_back(nullptr, 0);
        }
        else
        {
            splitter.emplace_back(&(*(begin + 1)), end - (begin + 1)); // Skip ','
        }
    }
    else
    {
        //No, address field or checksum field
        splitter.emplace_back(&(*begin), end - begin);
    }
}
