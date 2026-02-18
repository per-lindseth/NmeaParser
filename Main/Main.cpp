// Main.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <iomanip>

#include <Nmea/Nmea.h>
#include <Nmea/Messages.h>
#include <future>

using namespace std;

int main()
{
	size_t i = 1;
	for (auto& sentence : GetMessages())
	{
		Nmea nmea;
		nmea.parse(sentence);
		cout << setw(3) << i++ << " " << ": " << sentence.data();
		if (nmea.errorCode() != ErrorCode::E000)
		{
			auto count = nmea.indication() - &sentence[0] + 1 + 5;
			for (int i = 0; i < count; ++i)
				cout << '-';

			cout << '^' << endl;

			cout << " ErrorCode: " << ToString(nmea.errorCode()).c_str() << endl;
		}
	}

	return 0;
}

