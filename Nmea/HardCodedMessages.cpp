#include "HardCodedMessages.h"

#include <cassert>
#include <functional>
#include <memory>

#include "IField.h"
#include "Exception.h"
#include "NmeaFunctions.h"


namespace
{
    struct FieldBase: IField
    {
        virtual ~FieldBase() {}

        virtual size_t doParse(const std::vector<std::string_view> splitter, size_t index) = 0;

        virtual size_t parse(const std::vector<std::string_view> splitter, size_t index)
        {
            const size_t checksumIndex = splitter.size() - 1;
            if (index >= checksumIndex)
                throw  Exception(ErrorCode::E015, &(splitter[checksumIndex][0]));

            return doParse(splitter, index);
        }
    };

    // Special Format Fields *****************************************************
    //
    // a        Sentence Status Flag

    // A        Status
    struct Status : FieldBase
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            if (field.empty())
                throw Exception(ErrorCode::E016, &(field[0]));

            if (field.size() != 1)
                throw Exception(ErrorCode::E013, &(field[0]));

            if (field[0] == 'A' || field[0] == 'V')
                return index + 1;

            throw Exception(ErrorCode::E017, &(field[0]));
        }
    };

    struct LatLong : FieldBase
    {
    protected:
        size_t doParse(const std::vector<std::string_view> splitter, size_t index, size_t num)
        {
            const auto& field = splitter[index];
            const size_t size = field.size();

            if (size < num)
                throw Exception(ErrorCode::E013, &(field[0]));

            size_t i = 0;

            for (; i < num; ++i)
                if (!isdigit(field[i]))
                    break;

            if (size == num)
                return index + 1;

            if (field[i++] != '.')
                throw Exception(ErrorCode::E018, &(field[0]));

            for (; i < size; ++i)
                if (!isdigit(field[i]))
                    throw Exception(ErrorCode::E018, &(field[0]));

            return index + 1;
        }
    };

    // llll.ll        Latitude
    struct Latitude : LatLong
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            return LatLong::doParse(splitter, index, 4);
        }
    };

    //  yyyyy.yy    Longitude
    struct Longitude : LatLong
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            return LatLong::doParse(splitter, index, 5);
        }
    };

    // hhmmss.ss    Time
    struct Time : FieldBase
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            const auto ss1 = field.substr(0, 6);
            for (auto ch : ss1)
                if (!isdigit(ch))
                    throw Exception(ErrorCode::E019, &(field[0]));

            if (field.size() > 6)
                if (field[6] != '.')
                    throw Exception(ErrorCode::E020, &(field[0]));

            if (field.size() > 7)
            {
                const std::string_view ss2 = field.substr(7, field.size() - 7);
                for (auto ch : ss2)
                    if (!isdigit(ch))
                        throw Exception(ErrorCode::E019, &(field[0]));
            }

            return index + 1;
        }
    };

    //            Defined field
    struct CharLiterals : FieldBase
    {
        enum Precence { optional, mandatory };

        Precence          m_Precence;
        std::vector<char> m_Literal;

        CharLiterals(Precence precence = mandatory) : m_Precence(), m_Literal() {}

        CharLiterals* add(char ch) { m_Literal.push_back(ch); return this; }
        CharLiterals* add(std::string_view str) { for (auto ch : str) m_Literal.push_back(ch); return this; }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            if (field.empty())
                if (m_Precence == mandatory)
                    throw Exception(ErrorCode::E016, &(field[0]));
                else
                    return index + 1;

            for (auto ch : m_Literal)
                if (ch == field[0])
                    return index + 1;

            throw Exception(ErrorCode::E017, &(field[0]));
        }
    };

    // Numeric Value Fields ******************************************************

    // x.x        Variable numbers
    struct VariableNumbers : FieldBase
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            int i = 0;

            if (field.size() > 0 && field[0] == '-')
            {
                ++i;
            }

            for (; i < field.size(); ++i)
                if (!isdigit(field[i]))
                    break;

            if (i == field.size())
                return index + 1;

            if (field[i++] != '.')
                throw Exception(ErrorCode::E018, &(field[0]));

            for (; i < field.size(); ++i)
                if (!isdigit(field[i]))
                    throw Exception(ErrorCode::E018, &(field[0]));

            return index + 1;
        }
    };

    // xx__        Fixed number field
    struct FixedNumberField : FieldBase
    {
        size_t m_Length;

        FixedNumberField(size_t length) : m_Length(length) { assert(length > 0); }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            auto field = splitter[index];

            // Nonstrict mode
            if (field.size() == 0)
                return index + 1;

            if (field[0] == '-')
            {
                if (field.size() != (m_Length + 1))
                    throw Exception(ErrorCode::E013, &(field[0]));

                field = std::string_view(&(field[1]), field.size() - 1);
            }
            else
            {
                if (field.size() != m_Length)
                    throw Exception(ErrorCode::E013, &(field[0]));
            }

            for (auto ch : field)
                if (!isdigit(ch))
                    throw Exception(ErrorCode::E018, &(field[0]));

            return index + 1;
        }
    };

    // hh___    Fixed HEX field
    struct FixedHexField : FieldBase
    {
        size_t m_Length;

        FixedHexField(size_t length) : m_Length(length) { assert(length > 0); }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            auto field = splitter[index];

            // Nonstrict mode
            if (field.size() == 0)
                return index + 1;

            if (field[0] == '-')
            {
                if (field.size() != (m_Length + 1))
                    throw Exception(ErrorCode::E013, &(field[0]));

                field = std::string_view(&(field[1]), field.size() - 1);
            }
            else
            {
                if (field.size() != m_Length)
                    throw Exception(ErrorCode::E013, &(field[0]));
            }

            for (auto ch : field)
                if (!isxdigit(ch))
                    throw Exception(ErrorCode::E021, &(field[0]));

            return index + 1;
        }
    };

    // h--h        Variable HEX field 
    struct VariableHexField : FieldBase
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            int i = 0;

            if (field.size() > 0 && field[0] == '-')
            {
                ++i;
            }

            for (; i < field.size(); ++i)
                if (!isxdigit(field[i]))
                    throw Exception(ErrorCode::E021, &(field[0]));

            return index + 1;
        }
    };

    // Information Field *********************************************************

    // aa___    Fixed alpha field
    struct FixedAlphaField : FieldBase
    {
        size_t m_Length;

        FixedAlphaField(size_t length) : m_Length(length) { assert(length > 0); }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            if (field.size() != m_Length)
                throw Exception(ErrorCode::E013, &(field[0]));

            for (auto ch : field)
                if (!isupper(ch) && !islower(ch))
                    throw Exception(ErrorCode::E014, &(field[0]));

            return index + 1;
        }
    };

    // c--c        Variable Text
    struct VariableText : FieldBase
    {
        size_t m_Length;

        VariableText(size_t length=82) : m_Length(length) { assert(length > 0); }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            if (field.size() > static_cast<size_t>(m_Length))
                throw Exception(ErrorCode::E013, &(field[0]));

            /* Anything goes */

            return index + 1;
        }
    };

    // cc___    Fixed Text Field
    struct FixedTextField : FieldBase
    {
        size_t m_Length;

        FixedTextField(size_t length) : m_Length(length) { assert(length > 0); }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            if (field.size() != m_Length)
                throw Exception(ErrorCode::E013, &(field[0]));

            /* Anything goes */

            return index + 1;
        }
    };

    // ss___    Fixed Six bit field
    struct FixedSixBitField : FieldBase
    {
        size_t m_Length;

        FixedSixBitField(size_t length) : m_Length(length) { assert(length > 0); }

        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            if (field.size() != m_Length)
                throw Exception(ErrorCode::E013, &(field[0]));

            for (auto ch : field)
                if (!NmeaFunctions::issixbit(ch))
                    throw Exception(ErrorCode::E022, &(field[0]));

            return index + 1;
        }
    };

    // s--s        Variable Six bit field
    struct VariableSixBitField : FieldBase
    {
        size_t doParse(const std::vector<std::string_view> splitter, size_t index)
        {
            const auto& field = splitter[index];

            for (auto ch : field)
                if (!NmeaFunctions::issixbit(ch))
                    throw Exception(ErrorCode::E022, &(field[0]));

            return index + 1;
        }
    };

    // Repeatably group
    struct RepeatableGroup : FieldBase
    {
        enum Iteration { zero_or_one, zero_or_more, one_or_more, fixed };

        Iteration            m_Iteration;
        size_t         m_MinIterations;
        size_t         m_FiexedLength;
        std::vector<IField*> m_Fields;

        RepeatableGroup(Iteration iteration, size_t fixedLength = 0) : 
            m_Iteration(iteration), 
            m_MinIterations(0),
            m_FiexedLength(fixedLength),
            m_Fields()
        {
            switch (m_Iteration)
            {
            case zero_or_one:  m_MinIterations = 0; break;
            case zero_or_more: m_MinIterations = 0; break;
            case one_or_more:  m_MinIterations = 1; break;
            case fixed:        m_MinIterations = fixedLength; break;
            }
        }

        ~RepeatableGroup()
        {
            for (auto ptr : m_Fields)
                delete ptr;
        }

        void addField(std::unique_ptr<IField> field) {
            m_Fields.push_back(field.release());
        }

        size_t doParse(const std::vector<std::string_view> , size_t )
        {
            // Not in use! We are overridung parse() instead.
            return 0;
        }

        size_t parse(const std::vector<std::string_view> splitter, size_t index)
        {
            const size_t checksumIndex = splitter.size() - 1;
            const size_t numberOfDatafields = m_Fields.size();

            switch (m_Iteration)
            {
            case zero_or_one:
                if (index < checksumIndex)
                    if (!splitter[index].empty())
                        index = m_Fields[0]->parse(splitter, index);
                break;
            case one_or_more:
                if (index >= checksumIndex)
                    throw  Exception(ErrorCode::E015, &(splitter[checksumIndex][0]));
                // No break!
            case zero_or_more:
                while (index < checksumIndex)
                    for (size_t i = 0; i < numberOfDatafields; ++i)
                        index = m_Fields[i]->parse(splitter, index);
                break;
            case fixed:
                for (size_t count = 0; count < m_FiexedLength; ++count)
                    for (size_t i = 0; i < numberOfDatafields; ++i)
                        index = m_Fields[i]->parse(splitter, index);
                break;
            }

            return index;
        }
    };

    std::unique_ptr<Sentence> AAM()
    {
        auto s = std::make_unique<Sentence>("AAM");

        s->addField(std::unique_ptr<IField>(new Status()));
        s->addField(std::unique_ptr<IField>(new Status()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add('N')));
        s->addField(std::unique_ptr<IField>(new VariableText()));

        return s;
    }

    std::unique_ptr<Sentence> ACK()
    {
        auto s = std::make_unique<Sentence>("ACK");

        s->addField(std::unique_ptr<IField>(new FixedTextField(3)));

        return s;
    }

    std::unique_ptr<Sentence> ACN()
    {
        auto s = std::make_unique<Sentence>("ACN");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new FixedTextField(3)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("AQOS")));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add('N')));

        return s;
    }

    std::unique_ptr<Sentence> ALC()
    {
        auto s = std::make_unique<Sentence>("ALC");

        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));

        std::unique_ptr<RepeatableGroup> group(new RepeatableGroup(RepeatableGroup::one_or_more));
        group->addField(std::unique_ptr<IField>(new FixedTextField(3)));
        group->addField(std::unique_ptr<IField>(new VariableNumbers()));
        group->addField(std::unique_ptr<IField>(new VariableNumbers()));
        group->addField(std::unique_ptr<IField>(new VariableNumbers()));

        s->addField(std::unique_ptr<IField>(group.release()));

        return s;
    }

    std::unique_ptr<Sentence> ALF()
    {
        auto s = std::make_unique<Sentence>("ALF");

        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("ABC")));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("EAWC")));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("ASROUD")));
        s->addField(std::unique_ptr<IField>(new FixedTextField(3)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new VariableText(16)));

        return s;
    }

    std::unique_ptr<Sentence> ALR()
    {
        auto s = std::make_unique<Sentence>("ALR");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(3)));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("AV")));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("AV")));
        s->addField(std::unique_ptr<IField>(new VariableText()));

        return s;
    }

    std::unique_ptr<Sentence> ARC()
    {
        auto s = std::make_unique<Sentence>("ARC");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(3)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("AQOS")));

        return s;
    }

    std::unique_ptr<Sentence> EVE()
    {
        auto s = std::make_unique<Sentence>("EVE");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new VariableText()));
        s->addField(std::unique_ptr<IField>(new VariableText()));

        return s;
    }

    std::unique_ptr<Sentence> GGA()
    {
        auto s = std::make_unique<Sentence>("GGA");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new Latitude()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("NS")));
        s->addField(std::unique_ptr<IField>(new Longitude()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("EW")));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add('M')));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add('M')));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(4)));

        return s;
    }

    std::unique_ptr<Sentence> GLL()
    {
        auto s = std::make_unique<Sentence>("GLL");

        s->addField(std::unique_ptr<IField>(new Latitude()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("NS")));
        s->addField(std::unique_ptr<IField>(new Longitude()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("EW")));
        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new Status()));
        
        std::unique_ptr<RepeatableGroup> group(new RepeatableGroup(RepeatableGroup::zero_or_one));
        group->addField(std::unique_ptr<IField>((new CharLiterals())->add("ADEMSN")));

        s->addField(std::unique_ptr<IField>(group.release()));

        return s;
    }

    std::unique_ptr<Sentence> GSA()
    {
        auto s = std::make_unique<Sentence>("GSA");

        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("MA")));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));

        return s;
    }

    std::unique_ptr<Sentence> GSV()
    {
        auto s = std::make_unique<Sentence>("GSV");

        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        
        std::unique_ptr<RepeatableGroup> group(new RepeatableGroup(RepeatableGroup::one_or_more));
        group->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        group->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        group->addField(std::unique_ptr<IField>(new FixedNumberField(3)));
        group->addField(std::unique_ptr<IField>(new FixedNumberField(2)));

        s->addField(std::unique_ptr<IField>(group.release()));

        return s;
    }

    std::unique_ptr<Sentence> RMC()
    {
        auto s = std::make_unique<Sentence>("RMC");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new Status()));
        s->addField(std::unique_ptr<IField>(new Latitude()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("NS")));
        s->addField(std::unique_ptr<IField>(new Longitude()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("EW")));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(6)));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("EW")));

        std::unique_ptr<RepeatableGroup> group(new RepeatableGroup(RepeatableGroup::zero_or_one));
        group->addField(std::unique_ptr<IField>((new CharLiterals())->add("ADEMSN")));

        s->addField(std::unique_ptr<IField>(group.release()));

        return s;
    }

    std::unique_ptr<Sentence> VDM()
    {
        auto s = std::make_unique<Sentence>("VDM");

        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>((new CharLiterals())->add("AB")));
        s->addField(std::unique_ptr<IField>(new VariableSixBitField()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));

        return s;
    }

    std::unique_ptr<Sentence> VSI()
    {
        auto s = std::make_unique<Sentence>("VSI");

        s->addField(std::unique_ptr<IField>(new VariableText(15)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(1)));
        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));
        s->addField(std::unique_ptr<IField>(new VariableNumbers()));

        return s;
    }

    std::unique_ptr<Sentence> ZDA()
    {
        auto s = std::make_unique<Sentence>("ZDA");

        s->addField(std::unique_ptr<IField>(new Time()));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(4)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));
        s->addField(std::unique_ptr<IField>(new FixedNumberField(2)));

        return s;
    }
}


HardCodedMessages::HardCodedMessages()
{
    m_Sentences["AAM"] = AAM().release();
    m_Sentences["ACK"] = ACK().release();
    m_Sentences["ACN"] = ACN().release();
    m_Sentences["ALC"] = ALC().release();
    m_Sentences["ALF"] = ALF().release();
    m_Sentences["ALR"] = ALR().release();
    m_Sentences["ARC"] = ARC().release();
    m_Sentences["EVE"] = EVE().release();
    m_Sentences["GGA"] = GGA().release();
    m_Sentences["GLL"] = GLL().release();
    m_Sentences["GSA"] = GSA().release();
    m_Sentences["GSV"] = GSV().release();
    //m_Sentences["HBT"] = HBT().release();
    m_Sentences["RMC"] = RMC().release();
    m_Sentences["VDM"] = VDM().release();
    m_Sentences["VSI"] = VSI().release();
    m_Sentences["ZDA"] = ZDA().release();
}


HardCodedMessages::~HardCodedMessages()
{
    for (auto elem : m_Sentences)
        delete elem.second;
}
