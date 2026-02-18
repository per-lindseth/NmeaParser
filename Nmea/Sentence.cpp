#include "Sentence.h"

#include "Exception.h"

Sentence::Sentence(std::string sentenceFormatter):
    m_SentenceFormatter(sentenceFormatter),
    m_Fields()
{
}

Sentence::~Sentence()
{
    for (auto ptr : m_Fields)
        delete ptr;
}

void Sentence::parse(const std::vector<std::string_view> splitter)
{
    if (splitter.size() < 2)
        throw  Exception(ErrorCode::E015, &(splitter[0][0]));

    size_t j = 1; // Skip header field
    for (size_t i = 0; i < m_Fields.size(); ++i)
        j = m_Fields[i]->parse(splitter, j);
}
