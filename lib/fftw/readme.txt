How Visual Studio solution was made for FFTW

1. From libfftw-3.3.vcxproj, (probably) originally from ftp://ftp.fftw.org/pub/fftw/fftw-3.3-libs-visual-studio-2010.zip
only libfftw-3.3 taken and renamed as 

2. Adjust include directory setting in libfftw.vcxproj
<AdditionalIncludeDirectories>$(SourceLoc);</AdditionalIncludeDirectories>
In github comments, "Do not set include path ("-I") in Makefile.am" 
but for Visual Studio, the base source directory, if it is not the same directory as *.vcxproj,
should be included.  $(SourceLoc) is the base source directory, declared in libfftw.props

3. Version 3.3.9 has several source files not included in the proj file from the earlier version

The files added are as follows:
avx-128-fma.c
avx2.c
avx512.c
kcvi.c
vsx.c

The follow file as removed from the project:
sse2-nonportable.c

4. config.h from the earlier version is not sufficient. To make it current with 3.3.9, I built fftw in CygWin32,
after which I hand-picked macro constants in config.h, comparing between the earlier version and new one