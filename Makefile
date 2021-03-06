project: main.o DataParser.o AQ_Inductor.o Star.o
	g++ -std=c++11 -g -Wall main.o  DataParser.o AQ_Inductor.o Star.o -o project

main.o: main.cpp
	g++ -std=c++11 -g -Wall -c main.cpp

DataParser.o: DataParser.hpp DataParser.cpp
	g++ -std=c++11 -g -Wall -c DataParser.cpp

AQ_Inductor.o: AQ_Inductor.hpp AQ_Inductor.cpp
	g++ -std=c++11 -g -Wall -c AQ_Inductor.cpp

Star.o: Star.hpp Star.cpp
	g++ -std=c++11 -g -Wall -c Star.cpp

run: project
	./project

clean:
	rm *.o project

clean-output: #assumes all input files reside in a subfolder "data"
	rm data/*.with.negation.rul
	rm data/*.without.negation.rul
