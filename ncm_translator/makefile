cc		:= g++
libs	:= boost_unit_test_framework
opts	:= -Wall

all:
	$(cc) -o ncm_translator_test translator.c ncm_translator_test.cpp $(opts) -l$(libs)
	
clean:
	rm -rf *.o ncm_translator_test