IF EXIST auxlab.sln (
	SET AUXLABREPODIR=".\"
) ELSE (
	SET AUXLABREPODIR="..\AUXLAB\"
)
for %%x in (%*) do if not exist %AUXLABREPODIR%INCLUDE\%%x.h (
copy %AUXLABREPODIR%%%x\%%x.h %AUXLABREPODIR%INCLUDE\
)