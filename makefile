CXXFLAGS := g++ -ggdb -std=c++11 # c++ 编译参数
LinkLibs := -lpthread -lseccomp # 引用线程库与seccomp库
ConfigDir := $(CURDIR)/config.ini # 程序配置文件目录
EtsJudgeDir := /$(shell whoami)/.ets/judge/ # 程序工作目录

# 指定源文件所在目录
vpath %.h compiler controller checker common
vpath %.o compiler controller checker common
vpath %.cpp compiler controller checker common

main:main.o SubProcess.o CCompiler.o CXXCompiler.o ProgramController.o SubProcess.o Pipe3.o Util.o ConfigurationTool.o \
JudgeConfig.o Result.o CharsetUtil.o
	$(CXXFLAGS) -o main $^ $(LinkLibs)
	mkdir -p $(EtsJudgeDir)
	cp $(ConfigDir) $(EtsJudgeDir)
	make cleanobj
	make cleanjudge


%.o:%.cpp
	$(CXXFLAGS) -c $<

.PHONY:clean cleanobj cleanexe cleanjudge
clean:
	-rm -f main *.o
cleanobj:
	rm -f *.o
cleanexe:
	rm -f main
cleanjudge:
	cd $(EtsJudgeDir) && cd JudgeOutputFilePath && rm -rf ./*
	cd $(EtsJudgeDir) && cd JudgeExeFilePath && rm -rf ./*

