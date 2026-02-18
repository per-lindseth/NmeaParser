#pragma once

#include <string>

enum class ErrorCode {
    E000,   // No error
    E001,   // Illegal start of sentence, excpected $ or !
    E002,   // Illegal address field
    E003,   // Illegal end of sentence, excpected checksum field
    E004,   // Checksum error
    E005,   // Unknown talker ID
    E006,   // Illegal character in address field
    E007,   // Undefined character
    E008,   // Illegal character in data field
    E009,   // Unknown sentence formatter
    E010,   // Illegal character in talker id
    E011,   // Illegal character in proprietary talker 
    E012,   // Illegal character in sentence formatter
    E013,   // Illegal length of field
    E014,   // Illegal character in alpha field
    E015,   // Illegal number of fields for sentence
    E016,   // Mandatory field can't be empty
    E017,   // Illegal character literal
    E018,   // Illegal character in numeric field
    E019,   // Illegal character in time field
    E020,   // Illegal character in time field, exceptec period
    E021,   // Illegal character in hex field
    E022,   // Illegal character in six-bit binary representation field
    E023,   // Illegal length of sentence
    E024,   // Illegal character, excpected CR
    E025,   // Illegal character, excpected LF
    E026,   // Illegal character, excpected tab block end
    E027,   // Unknown parameter code
    E028,   // Illegal character, excpected colon
    E029,   // Identification field too long
    E030,   // Illegal character in identification field, excpected alphanumeric
    E031,   // Illegal character, excpected digit
    E032,   // Illegal character in sentence group field, excpected hyphen
    E033,   // Unexcpected end on line
};

inline
std::string ToString(const ErrorCode e)
{
    switch (e)
    {
    case ErrorCode::E000: return "No error";
    case ErrorCode::E001: return "Illegal start of sentence, excpected $ or !";
    case ErrorCode::E002: return "Illegal address field";
    case ErrorCode::E003: return "Illegal end of sentence, excpected checksum field";
    case ErrorCode::E004: return "Checksum error";
    case ErrorCode::E005: return "Unknown talker ID";
    case ErrorCode::E006: return "Illegal character in address field";
    case ErrorCode::E007: return "Undefined character";
    case ErrorCode::E008: return "Illegal character in data field";
    case ErrorCode::E009: return "Unknown sentence formatter";
    case ErrorCode::E010: return "Illegal character in talker id";
    case ErrorCode::E011: return "Illegal character in proprietary talker";
    case ErrorCode::E012: return "Illegal character in sentence formatter";
    case ErrorCode::E013: return "Illegal length of field";
    case ErrorCode::E014: return "Illegal character in alpha field";
    case ErrorCode::E015: return "Illegal number of fields for sentence";
    case ErrorCode::E016: return "Mandatory field can't be empty";
    case ErrorCode::E017: return "Illegal character literal";
    case ErrorCode::E018: return "Illegal character in numeric field";
    case ErrorCode::E019: return "Illegal character in time field";
    case ErrorCode::E020: return "Illegal character in time field, excpected period";
    case ErrorCode::E021: return "Illegal character in hex field";
    case ErrorCode::E022: return "Illegal character in six-bit binary representation field";
    case ErrorCode::E023: return "Illegal length of sentence";
    case ErrorCode::E024: return "Illegal character, excpected CR";
    case ErrorCode::E025: return "Illegal character, excpected LF";
    case ErrorCode::E026: return "Illegal character, excpected tab block end";
    case ErrorCode::E027: return "Unknown parameter code";
    case ErrorCode::E028: return "Illegal character, excpected colon";
    case ErrorCode::E029: return "Identification field too long";
    case ErrorCode::E030: return "Illegal character in identification field, excpected alphanumeric";
    case ErrorCode::E031: return "Illegal character, excpected digit";
    case ErrorCode::E032: return "Illegal character in sentence group field, excpected hyphen";
    case ErrorCode::E033: return "Unexcpected end on line";
    }

    return "Unknown error code";
}