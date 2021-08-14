rem FLAGS : /r - Compile shaders and resources only
@echo off
where cl
IF %errorlevel% NEQ 0 call "c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall" x64

mkdir ..\build
mkdir ..\build\res
del /Q /S ..\build\*
cls
pushd ..\build

FOR %%A IN (%*) DO (
  IF "%%A"=="/r" (
    echo [1] Compiling Resources:
    python3 ..\misc\ply.py ..\res\trunk.ply res\trunk.ply
    python3 ..\misc\bmp.py ..\res\bark.bmp res\bark.bmp
    python3 ..\misc\ply.py ..\res\cube.ply res\cube.ply
    popd
    exit /B
  )
)

echo [1] Compiling Shaders:
fxc -nologo -EVertexMain -T vs_5_0 ..\res\shader.hlsl -Fo res\shaderv.cso
fxc -nologo -EPixelMain -T ps_5_0 ..\res\shader.hlsl -Fo res\shaderp.cso
rem fxc -nologo -EVertexMain -T vs_5_0 ..\res\gui.hlsl -Fo res\guiv.cso
rem fxc -nologo -EPixelMain -T ps_5_0 ..\res\gui.hlsl -Fo res\guip.cso

echo.
echo [2] Compiling Code:
#taskkill /IM "win64_app.exe" 2> nul
cl -nologo -Wall -wd4820 -wd5045 -DSLINAPP_DEBUG=1 -GS- -Z7 ..\src\win64_app.c /link /MAP /SUBSYSTEM:WINDOWS /ENTRY:MainEntry /OPT:REF KERNEL32.LIB USER32.LIB D3D11.LIB WINMM.LIB LIBUCRT.LIB
popd
