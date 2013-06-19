
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

#define IMAGE_SIZE 360960 //=752*480
#define TCP_PORT 13777
#define UDP_PORT 13778

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

		tcp::socket tcp_socket(io_service);
		boost::asio::connect(tcp_socket, endpoint_iterator);

		udp::socket udp_socket(io_service, udp::endpoint(udp::v4(), UDP_PORT));
    udp::endpoint udp_endpoint(tcp_socket.remote_endpoint().address(), UDP_PORT);

    printf("listening to %s on port %d\n", udp_endpoint.address().to_string().c_str(), udp_endpoint.port());

		boost::posix_time::ptime last_time(boost::posix_time::microsec_clock::local_time());

		char image[2][IMAGE_SIZE];
    int image_package_counter[2] = {0};

		for (;;)
		{
			char buf[1600];
			boost::system::error_code error;

	    udp::endpoint sender_endpoint;
			size_t len = udp_socket.receive_from( boost::asio::buffer(buf), udp_endpoint, 0, error);

      boost::array<int, 2> header = *((boost::array<int, 2> *)buf);
      int cam_number = header[0];
      int package_number = header[1];

			// copy received data into image buffers
			memcpy( image[cam_number], buf + 8, len - 8);

			// check if
			if(package_number >= 240)
			{
			  printf("complete image %d received in %d packages\n", cam_number, package_number);
			  if(image_package_counter[cam_number] != package_number)
			    printf("One or more packages lost or not in order %d != %d\n",image_package_counter[cam_number], package_number );
			  image_package_counter[cam_number] = 0;
			  if(cam_number == 0)
			  {
			    boost::posix_time::ptime time_now(boost::posix_time::microsec_clock::local_time());
			    std::printf("package nr %d with %d byte,delta t %5d\n", *((int *)buf), len, (int)(time_now - last_time).total_microseconds());
			    last_time =boost::posix_time::microsec_clock::local_time();
			  }
			}
			else
	      image_package_counter[cam_number]++;

			if (error == boost::asio::error::eof)
				break; // Connection closed cleanly by peer.
			else if (error)
				throw boost::system::system_error(error); // Some other error.

		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
