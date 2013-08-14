#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include "tcp_server.hpp"

using boost::asio::ip::tcp;

TcpServer::TcpServer(boost::asio::io_service& io_service, unsigned short int port)
: io_service_(io_service),
  socket_(io_service),
  acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
  has_connection_(false) {
	std::cout << "max_iov_len=" << boost::asio::detail::max_iov_len << std::endl;
	acceptor_.set_option(tcp::no_delay(true));
	accept_connection();
}

TcpServer::~TcpServer() {}

void TcpServer::accept_connection() {
	has_connection_ = false;
	// Verify socket is in a closed state.
	socket_.close();
//	// On success or failure, acceptor will open() socket_.
	acceptor_.async_accept(
			socket_,
			boost::bind(&TcpServer::handle_accept, this,
					boost::asio::placeholders::error));
	std::cout << "accepting connection" << std::endl;

	// waiting for new connection
}

void TcpServer::handle_accept(const boost::system::error_code& error) {
	// On error, return early.
	if (error) {
		std::cout << "handle_accept: " << error.message() << std::endl;
		return;
	}

	//ready to send data
	printf("client connected from %s\n", socket_.remote_endpoint().address().to_string().c_str());
	has_connection_ = true;
//    read();
}

void TcpServer::read()
{
  std::cout << "reading from socket" << std::endl;
  async_read(socket_, boost::asio::buffer(buffer_),
    boost::asio::transfer_at_least(1),
    boost::bind(&TcpServer::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
}

void TcpServer::handle_read(const boost::system::error_code& error,
                 std::size_t bytes_transferred)
{
  // On error, go back to listening for a new connection.
  if (error)
  {
    std::cout << "handle_read: " << error.message() << std::endl;
    accept_connection();
    return;
  }

  // Output read data.
  std::cout.write(&buffer_[0], bytes_transferred);

  // Read data, so read some more.
  read();
}

void TcpServer::send_data(char * data, DataHeader& header) {

	std::vector<uint32_t> header_payload(3,0);
	header_payload[0]=htonl(header.timestamp);
	header_payload[1]=htonl(header.data_size);
	header_payload[2]=htonl(header.data_id);

	std::vector<boost::asio::const_buffer> buffers;
	buffers.push_back(boost::asio::buffer(header_payload, sizeof(header_payload)));
	buffers.push_back(boost::asio::buffer(data, header.data_size));
//	printf("data pointer %p\n", data);
	try
	{
//		socket_.send(boost::asio::buffer(data.data(), data.size()));
		int bytes_sent = socket_.send(buffers);
//		printf("bytes_sent: %d\n", bytes_sent);
	}
	catch (std::exception& e){
		printf("client disconnected\n");
		accept_connection();
	}
	//  async_write(
	//      socket_,
	//      boost::asio::buffer(data.data(), data.size()),
	//      boost::asio::transfer_at_least(data.size()),
	//      boost::bind(&TcpServer::handle_writer, this,
	//                  boost::asio::placeholders::error,
	//                  boost::asio::placeholders::bytes_transferred,
	//                  data));
}

//void TcpServer::handle_writer(const boost::system::error_code& error,
//                              std::size_t bytes_transferred, SharedMemory& data) {
//  // On error, go back to listening for a new connection.
//  if (error) {
//    std::cout << "disconnected: " << error.message() << std::endl;
//    accept_connection();
//    return;
//  }
//  printf("done");
//}

const tcp::endpoint TcpServer::getEndpoint() {
	return socket_.remote_endpoint();
}
