
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>

using boost::asio::ip::tcp;

#define IMAGE_SIZE 360960 //=752*480

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: client <host>" << std::endl;
			return 1;
		}

		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(argv[1], "13777");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		tcp::socket socket(io_service);
		boost::asio::connect(socket, endpoint_iterator);

		boost::posix_time::ptime last_time(boost::posix_time::microsec_clock::local_time());

		for (;;)
		{
			boost::array<char, IMAGE_SIZE> buf;
			boost::system::error_code error;

			//      size_t len = socket.read_some(boost::asio::buffer(buf), error);
			size_t len = boost::asio::read(socket, boost::asio::buffer(buf), error);

			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.


			boost::posix_time::ptime time_now(boost::posix_time::microsec_clock::local_time());
			std::printf("frequency %f\n", (double)(time_now - last_time).total_microseconds());
			last_time =boost::posix_time::microsec_clock::local_time();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
