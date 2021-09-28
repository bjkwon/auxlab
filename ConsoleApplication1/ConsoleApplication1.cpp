// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

using namespace std;

#include <iostream>
#include "AUXLib.h"

int main()
{
    std::cout << "Hello World!\n";

    int handle = AUXNew(1000, "");
    double *buf1, *buf2;
    int len;
    auto res = AUXEval(handle, "x=[noise(20); tone(100,30)];", &buf1, &len);
    if (res < 0) cout << AUXGetErrMsg() << endl;
    res = AUXEval2(handle, "x", &buf1, &buf2, &len);
    if (res < 0) cout << AUXGetErrMsg() << endl;
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
