#pragma once

#include <map>
#include "Sentence.h"

class HardCodedMessages
{
public:
    HardCodedMessages();
    ~HardCodedMessages();

    std::map<std::string, Sentence*> m_Sentences;
};

