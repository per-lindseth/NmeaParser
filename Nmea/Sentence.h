#pragma once

#include <string>
#include <memory>

#include "ISentenceParser.h"
#include "IField.h"

class Sentence
{
public:
    Sentence(std::string sentenceFormatter);
    ~Sentence();

    void parse(const std::vector<std::string_view> splitter);

    void addField(std::unique_ptr<IField> field) {
        m_Fields.push_back(field.release());
    }

private:
    std::string          m_SentenceFormatter;
    std::vector<IField*> m_Fields;
};

