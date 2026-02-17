This repo will contain C++ code for coding and dedcoding NMEA sentences.
The aim is to provide I high level interface for sending and subcribing sentences.
The plan is to implement the following for reading:
1. Level1: The lowest level shall provide "sentences" that starts with '\\', '!' or '$'
   and ends with '\r\n'. There may be several versions with the same interface but different
   implementations using various lower level code. The sentence is a std::string.
2. Level2: This level code will verify that the sentence is well formed with a correct checksum.
   It will decode the talker and sentence formatter and recognice paramteric ($aaccc),
   encapsulated (!aaccc), query ($aabbQ) and propriatary ($aaa) sentences including tag block
   and the source (s:aaccc) withing these. It shall provide an interfcae for subsribing to
   variaous kinds of sentences based on types (paramteric, encapsulated, query, propriatary)
   talker, sentence formatter and source. The result is chall be vied as fileds of text where
   the first is from the start up to the first comma, the last is the checksum part.
3. Level3: wil provide the a class for each recognized NMEA sentences all providing info about
   the tag block info.
4. Level4: AIS
5. Level5: ASM
