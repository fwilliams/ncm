cc		:= g++
test_libs	:= boost_unit_test_framework
test_opts	:= -Wall -O0

all:
	$(cc) -o ncm_translator translator.c translator_main.cpp
	
test:
	$(cc) -o ncm_translator_test translator.c ncm_translator_test.cpp $(test_opts) -l$(test_libs)
	
clean:
	rm -rf *.o ncm_translator_test ncm_translator