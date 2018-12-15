# AUXLAB

AUXLAB is as an integrative computing environment, created for scientists, engineers, and technicians who are working with audio signals. It allows you to generate and modify signals and execute computations on them. Its apperarance mimics MATLAB and the syntax, AUX (**AU**ditory synta**X**) was derived from MATLAB; therefore, it is an easy-to-use tool for anyone familiar with MATLAB. However, because AUX dramatically simplifies coding for audio signals and signal processing, prior knowledge of MATLAB shouldn't be a pre-requisite. Any professional or student who studies, plays and deals with sounds, particularly those in the fields that are considered "less quantitative," such as music, psychology, or linguistics, can learn coding easily with AUX.

# Installation

The stand-alone package of AUXLAB is currently available only for Windows OS (Windows 7, 8 and 10). No special installation is required. Download the zip file and run the exe after unzipping it. [Download here](http://auditorypro.com/download/auxlab/auxlab.html)

You might need Microsoft Visual C++ Redistributable for Visual Studio 2015, such as [vc_redist.x86.exe or vc_redist.x64.exe](https://www.microsoft.com/en-us/download/details.aspx?id=48145), depending on your machine, if you don't already have them installed in your system.

# How to compile and link

This repository has all files necessary for you to build the package with Visual Studio 2017. The only step to do is to edit auxlab.props--go to lines 5 and 6.

```sh
5: <LocalRepoDir>_________</LocalRepoDir>
6: <BuildDir>_________</BuildDir>
```
Specify the directory of the local repository on line 5 and the directory for the build outputs on line 6. Now you are ready to build the application.
