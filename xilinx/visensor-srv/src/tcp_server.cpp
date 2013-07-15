#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include "tcp_server.hpp"

#define TCP_PORT 13777

using boost::asio::ip::tcp;

TcpServer::TcpServer(boost::asio::io_service& io_service)
: io_service_(io_service),
  socket_(io_service),
  acceptor_(io_service, tcp::endpoint(tcp::v4(), TCP_PORT)),
  has_connection_(false) {
  std::cout << "max_iov_len=" << boost::asio::detail::max_iov_len << std::endl;
  acceptor_.set_option(tcp::no_delay(true));
  accept_connection();
}

TcpServer::~TcpServer() {}

void TcpServer::accept_connection() {
  // Verify socket is in a closed state.
  socket_.close();
  // On success or failure, acceptor will open() socket_.
  acceptor_.async_accept(
      socket_,
      boost::bind(&TcpServer::handle_accept, this,
                  boost::asio::placeholders::error));

  std::cout << "accepting connection" << std::endl;
  // waiting for new connection
  has_connection_ = false;
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
}

void TcpServer::send_data(SharedMemory& data) {

  try
  {
    write(
        socket_,
        boost::asio::buffer(data.data(), data.size()),
        boost::asio::transfer_at_least(data.size()));
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
