all:
	g++ Server.cpp -o s.out
	g++ Client.cpp -o c.out

clean:
	rm *.out