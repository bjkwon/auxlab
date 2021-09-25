// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "AUXLib.h"


int main()
{
    std::cout << "Hello World!\n";

    int handle = AUXNew(44100, "");
    char sbuf[16];
    double* buf;
    int len;
    std::string auxcomd = "x=wave(\"c:\\temp\\";
    auxcomd = auxcomd + "nfl1.wav";
    auxcomd = auxcomd + "\"); ";
    auto res = AUXDef(handle, auxcomd.c_str());
    std::string strin = "val=0;startAt=43*1000;endAt=101*1000; y=x(startAt~endAt); y+= noise(y.dur).lpf(4000) @ y @ val;";
    //strin += " y.write(\"c:\\temp\\mix";
    //itoa(1, sbuf, 10);
    //strin += sbuf;
    //strin += ".wav\")";
    res = AUXDef(handle, strin.c_str());
    res = AUXPlay(handle, 0);
    //    AUXWavwrite(handle, "output.wav");
    strin = "val=-5;startAt=33*1000;endAt=101*1000; y=x(startAt~endAt); y+= noise(y.dur).lpf(4000) @ y @ val;";
    strin += " y.write(\"c:\\temp\\mix";
    itoa(2, sbuf, 10);
    strin += sbuf;
    strin += ".wav\")";
    res = AUXDef(handle, strin.c_str());
    for (int64_t k = 0; k > 0; k++)
    {
        if (k == k / 1000 * 1000) printf("k\n", k);
    }
 //   AUXDelete(handle);

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
