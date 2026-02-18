#include "NmeaFunctions.h"

#include <stdexcept>

#include "Messages.h"
#include "SentenceType.h"

using namespace std;

namespace
{
    static const bool reserved[]{
        false, false, false, false, false, false, false, false, false, false, //   0 -   9
        true, false, false,  true, false, false, false, false, false, false,  //  10 -  19    LF( 10),  CR( 13)
        false, false, false, false, false, false, false, false, false, false, //  20 -  29             
        false, false, false,  true, false, false,  true, false, false, false, //  30 -  39     !( 36),   $( 36)
        false, false,  true, false,  true, false, false, false, false, false, //  40 -  49     *( 42),   ,( 44)
        false, false, false, false, false, false, false, false, false, false, //  50 -  59             
        false, false, false, false, false, false, false, false, false, false, //  60 -  69             
        false, false, false, false, false, false, false, false, false, false, //  70 -  79             
        false, false, false, false, false, false, false, false, false, false, //  80 -  89             
        false, false,  true, false,  true, false, false, false, false, false, //  90 -  99     \( 92),   ^( 94)
        false, false, false, false, false, false, false, false, false, false, // 100 - 109             
        false, false, false, false, false, false, false, false, false, false, // 110 - 119             
        false, false, false, false, false, false,  true,  true                // 120 - 127     ~(126), DEL(127)
    };
}

namespace NmeaFunctions
{

    // value --> result
    //    15 --> 'F'
    //       ...
    //    10 --> 'A'
    //     9 --> '9'
    //      ...
    //     0 --> '0'
    // 
    // others --> exception::logic_error
    char hex2Char(size_t value)
    {
        if (value > 9)
        {
            if (value > 15)
                throw std::logic_error("Illegal value, excpected a number in the range 0 .. 15");
            else
                return static_cast<char>('A' + (value - 10));
        }

        return static_cast<char>('0' + value);
    }

    // Increment begin and update checksum
    // Exception ErrorCode::E033 end of input
    // Exception ErrorCode::E007 undefined character in input
    // 
    void incr(uint8_t& checkSum, const char*& begin, const char* end)
    {
        ++begin;

        if (end <= begin)
            throw Exception(ErrorCode::E033);

        if (isundefined(*begin))
            // Ref.: NMEA 0183 Version 4.00, 5.
            throw Exception(ErrorCode::E007, &(begin[0]));

        checkSum ^= static_cast<uint8_t>(*begin);
    }

    // Increment begin count number of times
    void incr(uint8_t& checkSum, const char*& begin, const char* end, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
            incr(checkSum, begin, end);
    }
    
    // Return true if ch is a valid character
    bool isvalid(const char ch) noexcept
    {
        return 32 <= ch && ch <= 127 && !reserved[ch];
    }

    // Return true if ch is a resvered character
    bool isreserved(const char ch) noexcept
    {
        return (0 <= ch && ch <= 127) && reserved[ch];
    }

    void checkTalkerId(char ch1, char ch2, const char* errorIndication)
    {
        // Ref.: NMEA 0183 Version 4.00, 5.2.1
        if ((isdigit(ch1) || isupper(ch1)) &&
            (isdigit(ch2) || isupper(ch2)))
            return;

        throw Exception(ErrorCode::E010, errorIndication);
    }

    void checkProprietaryTalkerId(char ch1, char ch2, char ch3, const char* errorIndication)
    {
        if (isupper(ch1) && isupper(ch2) && isupper(ch3))
            return;

        throw Exception(ErrorCode::E011, errorIndication);
    }

    void checkSentenceFormatter(char ch1, char ch2, char ch3, const char* errorIndication)
    {
        // Ref.: NMEA 0183 Version 4.00, 5.2.1
        if ((isdigit(ch1) || isupper(ch1)) &&
            (isdigit(ch2) || isupper(ch2)) &&
            (isdigit(ch3) || isupper(ch3)))
            return;

        throw Exception(ErrorCode::E012, errorIndication);
    }

    void checkDataFieldCharacters(std::string_view field)
    {
        // Ref.: NMEA 0183 Version 4.00, 5.2.2 and 5.3.3 (3.)
        for (auto ch: field)
        {
            // isdefined(ch) && isvalid(ch) implies !reserved[ch]
            if (reserved[ch])
                throw Exception(ErrorCode::E008, &(field[0]));
        }
    }

    void checkProprietaryDataFieldCharacters(std::string_view field)
    {
        // Ref.: NMEA 0183 Version 4.00, 5.2.2
        for (auto ch : field)
        {
            // isdefined(ch) && isvalid(ch) implies !reserved[ch]
            if (reserved[ch] && ch != '^')
                throw Exception(ErrorCode::E008, &(field[0]));
        }
    }

    void matchTalker(char ch1, char ch2, const char* errorIndication)
    {
        static const char* talkers[]{
            "AB", // Independent AIS Base Station
            "AD", // Dependent AIS Base Station
            "AG", // Autopilot - General 
            "AP", // Autopilot - Magnetic 
            "CD", // Communications - Digital Selective Calling (DSC) 
            "CR", // Communications - Receiver / Beacon Receiver 
            "CS", // Communications - Sattelite 
            "CT", // Communications - Radio-Telephone (MF/HF) 
            "CV", // Communications - Radio-Telephone (VHF) 
            "CX", // Communications - Scanning Receiver 
            "DF", // Direction Finder 
            "EC", // Electronic Chart Display & Information System (ECDIS) 
            "EP", // Emergency Position Indicating Beacon (EPIRB) 
            "ER", // Engine Room Monitoring Systems 
            "GP", // Global Positioning System (GPS) 
            "HC", // Heading - Magnetic Compass 
            "HE", // Heading - North Seeking Gyro 
            "HN", // Heading - Non North Seeking Gyro 
            "II", // Integrated instrumentation 
            "IN", // Integrated Navigation 
            "LC", // Loran C 
            "RA", // RADAR and/or ARPA 
            "SD", // Sounder, Depth 
            "SN", // Electronic Positioning System, other/general 
            "SS", // Souder, Scanning 
            "TI", // Turn Rate Indicator 
            "VD", // Velocity Sensor, Doppler, other/general 
            "DM", // Velocity Sensor, Speed Log, Water, Magnetic 
            "VW", // Velocity Sensor, Speed Log, Water, Mechanical 
            "WI", // Weather Instruments 
            "YX", // Transduser 
            "ZA", // Timekeeper - Atomic Clock 
            "ZC", // Timekeeper - Chronometer 
            "ZQ", // Timekeeper - Quartz 
            "ZV"  // Radio Update, WWV or WWVH
        };

        char buff[4];
        buff[0] = ch1;
        buff[1] = ch2;
        buff[2] = '\0';

        for (const char* elem : talkers)
        {
            const int cmp = strcmp(elem, buff);
            if (cmp == 0)
                return;
            else if (cmp > 0)
                throw Exception(ErrorCode::E005, errorIndication);
        }

        throw Exception(ErrorCode::E005, errorIndication);
    }

    void matchProprietaryTalker(char ch1, char ch2, char ch3)
    {
        return;
        throw  ErrorCode::E005;
    }

    void  matchSentenceFormatter(char ch1, char ch2, char ch3)
    {
        // Sorted array of sentence formatters
        static const char* formatters[]{
            "AAM",
            "ALM",
            "APA",
            "APB",
            "ASD",
            "BEC",
            "BOD",
            "BWC",
            "BWR",
            "BWW",
            "DBK",
            "DBS",
            "DBT",
            "DCN",
            "DPT",
            "DSC",
            "DSE",
            "DSI",
            "DSR",
            "DTM",
            "FSI",
            "GBS",
            "GGA",
            "GLC",
            "GLL",
            "GRS",
            "GSA",
            "GST",
            "GSV",
            "GTD",
            "GXA",
            "HDG",
            "HDM",
            "HDT",
            "HSC",
            "LCD",
            "MSK",
            "MSS",
            "MTW",
            "MWD",
            "MWV",
            "OLN",
            "OSD",
            "RMA",
            "RMB",
            "RMC",
            "ROO",
            "ROT",
            "RPM",
            "RSA",
            "RSD",
            "RTE",
            "SFI",
            "STN",
            "TLL",
            "TRF",
            "TTM",
            "TXT",
            "VBW",
            "VDR",
            "VHW",
            "VLW",
            "VPW",
            "VTG",
            "VWR",
            "WCV",
            "WDC",
            "WDR",
            "WNC",
            "WPL",
            "XDR",
            "XTE",
            "XTR",
            "ZDA",
            "ZDL",
            "ZFO",
            "ZTG"
        };

        char buff[4];
        buff[0] = ch1;
        buff[1] = ch2;
        buff[2] = ch3;
        buff[3] = '\0';

        for (const char* elem : formatters)
        {
            const int cmp = strcmp(elem, buff);
            if (cmp == 0)
                return;
            else if (cmp > 0)
                throw  ErrorCode::E009;
        }

        throw  ErrorCode::E009;
    }

    void checkPositivInteger(std::string_view field)
    {
        for (size_t i = 0; i < field.size(); ++i)
            if (!isdigit(field[i]))
                throw Exception(ErrorCode::E031, &(field[i]));
    }

    void checkIdentification(std::string_view field)
    {
        if (field.size()>15)
            throw Exception(ErrorCode::E029, &(field[0]));

        for (size_t i = 0; i < field.size(); ++i)
            if (!isdigit(field[i]) && !isupper(field[i]) && !islower(field[i]))
                throw Exception(ErrorCode::E030, &(field[i]));
    }

    void checkSentenceGrouping(std::string_view field)
    {
        int i{ 0 };
        int last{ 0 };
        const size_t size{ field.size() };

        if (size <= i)
            throw Exception(ErrorCode::E032);

        for (size_t count = 0; count < 2; ++count)
        {
            last = i;

            // Number
            while (true)
            {
                if (!isdigit(field[i])) break;
                if (size <= ++i)
                    throw Exception(ErrorCode::E032);
            }

            if (i == last)
                throw Exception(ErrorCode::E031, &(field[i]));

            if (field[i] != '-')
                throw Exception(ErrorCode::E032, &(field[i]));

            // Total number of sentences
            if (size <= ++i)
                throw Exception(ErrorCode::E032);
        }

        last = i;

        // Number
        while (true)
        {
            if (!isdigit(field[i])) break;
            if (size <= ++i) break;
        }

        if (i == last)
            throw Exception(ErrorCode::E031, &(field[i]));
    }
}