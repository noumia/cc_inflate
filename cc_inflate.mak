# cc_inflate.mak

Debug/cc_ungzip.exe: debug

Release/cc_ungzip.exe: release

debug: cc_inflate.sln
	msbuild.exe cc_inflate.sln /p:Configuration=Debug

release: cc_inflate.sln
	msbuild.exe cc_inflate.sln /p:Configuration=Release

cc_inflate.sln: cc_inflate.gyp
	cc_inflate.cmd

all: debug release

clean:
	rd /S /Q Debug
	rd /S /Q Release

test: release
	cd test
	python.exe run.py .
	cd ..

