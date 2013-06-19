#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

#define IMAGE_SIZE 360960 //=752*480

using boost::asio::ip::tcp;

class tcp_writer
{
public:
	tcp_writer(boost::asio::io_service& io_service, char *data)
: io_service_(io_service),
  socket_(io_service),
  acceptor_(io_service, tcp::endpoint(tcp::v4(), 13777))
{
		data_ = data;
        std::cout << "max_iov_len=" << boost::asio::detail::max_iov_len << std::endl;
        acceptor_.set_option(tcp::no_delay(true));
		accept_connection();
}

private:

	void accept_connection()
	{
		std::cout << "accepting connection" << std::endl;
		// Verify socket is in a closed state.
		socket_.close();
		// On success or failure, acceptor will open() socket_.
		acceptor_.async_accept(socket_,
				boost::bind(&tcp_writer::handle_accept, this,
						boost::asio::placeholders::error));
	}

	void handle_accept(const boost::system::error_code& error)
	{
		// On error, return early.
		if (error)
		{
			std::cout << "handle_accept: " << error.message() << std::endl;
			return;
		}

		// Start writing to socket.
		write();
	}

	void write()
	{
	    async_write(socket_, boost::asio::buffer(data_, IMAGE_SIZE),
	      boost::asio::transfer_at_least(IMAGE_SIZE),
	      boost::bind(&tcp_writer::handle_writer, this,
	                  boost::asio::placeholders::error,
	                  boost::asio::placeholders::bytes_transferred));
//		for (int i = 0; i <100; i++)
//		{
//			// increment the counter
//			*((unsigned int *) data_+0) += 1;
//			boost::system::error_code ignored_error;
//			boost::asio::write(socket_, boost::asio::buffer(data_, IMAGE_SIZE), ignored_error);
//		}
	}

	void handle_writer(const boost::system::error_code& error,
			std::size_t bytes_transferred)
	{
		// On error, go back to listening for a new connection.
		if (error)
		{
			std::cout << "handle_write: " << error.message() << std::endl;
			accept_connection();
			return;
		}

		// Output read data.
//		std::cout.write(&buffer_[0], bytes_transferred);

		// Read data, so read some more.
		write();
	}

private:
	boost::asio::io_service& io_service_;
	tcp::socket              socket_;
	tcp::acceptor            acceptor_;
	char *data_;

};


int main()
{

	char data[IMAGE_SIZE]; // as one b/w image
	bzero(data,IMAGE_SIZE);

	*((unsigned int *) data+0) = 0; //image counter at the beginning of the data

	//fill the data with numbers
	for(int i = 0; i<IMAGE_SIZE; i+=4)
	{
		char number [5];
		sprintf(((char *) data+i), "%d", i/4);
	}


	boost::asio::io_service io_service;
	tcp_writer writer(io_service, data);
	io_service.run();

	return 0;
}
