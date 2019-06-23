IF EXIST auxlab.sln (
	SET AUXLABREPODIR=".\"
) ELSE (
	SET AUXLABREPODIR="..\AUXLAB\"
)
for %%x in (%*) do if exist %AuxlabRepoDir%INCLUDE\%%x.h del %AuxlabRepoDir%INCLUDE\%%x.h