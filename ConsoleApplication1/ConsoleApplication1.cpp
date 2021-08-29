// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "AUXLib.h"


int main()
{
    std::cout << "Hello World!\n";

    int handle = AUXNew(44100, "");
    double* buf;
    int len;
    int res = AUXEval(handle, "tone([1000 2000],10000).ramp(50)@-20", &buf, &len);
    AUXWavwrite(handle, "output.wav");
    AUXDelete(handle);

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
