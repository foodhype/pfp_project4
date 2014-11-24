run:
	g++ -std=c++0x main.cpp -pthread -o main -I$(TACC_PAPI_INC) -L$(TACC_PAPI_LIB) -lpapi -lrt && ./main
