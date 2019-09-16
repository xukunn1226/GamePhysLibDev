@echo off
rd /s/q "../GamePhysLib/Inc"
rd /s/q "../GamePhysLib/Src"

mkdir "../GamePhysLib/Inc"
mkdir "../GamePhysLib/Src"

xcopy /s/y "./Inc" "../GamePhysLib/Inc"
xcopy /s/y "./Src" "../GamePhysLib/Src"

cd ../GamePhysLib

For /F "tokens=1-2 delims= " %%a In ('svn st') Do (
    IF %%a == ? (
       svn add %%b
    )
    IF %%a == ^! (
       svn delete %%b
    )
)
