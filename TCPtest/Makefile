
server: server.o
		g++ server.o -pg -o server -lboost_system -lboost_thread -lboost_system -lpthread

client: client.o
		g++ client.o -o client -lboost_system -lboost_thread -lboost_system -lpthread -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy
		
all: client server