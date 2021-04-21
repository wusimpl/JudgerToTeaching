CXXFLAGS := g++ -g # c++ 编译参数
LinkLibs := -lpthread -lseccomp # 引用线程库与seccomp库
ConfigDir := $(CURDIR)/config.ini # 程序配置文件目录
EtsJudgeDir := /$(shell whoami)/.ets/judge/ # 程序工作目录

# 指定源文件所在目录
vpath %.h compiler controller checker utils
vpath %.o compiler controller checker
vpath %.cpp compiler controller checker

main:main.o SubProcess.o CCompiler.o CXXCompiler.o ProgramController.o
	$(CXXFLAGS) -o main $^ $(LinkLibs)
	mkdir -p $(EtsJudgeDir)
	cp $(ConfigDir) $(EtsJudgeDir)
	make cleanobj


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
	-rm $(EtsJudgeDir)/JudgeExeFilePath/* $(EtsJudgeDir)/JudgeOutputFilePath/*

