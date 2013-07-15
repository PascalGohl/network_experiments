
#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

#include <boost/asio.hpp>

#include "shared_memory.hpp"

class TcpServer {
 public:
  TcpServer(boost::asio::io_service& io_service);
  virtual ~TcpServer();

  void send_data(SharedMemory& data);
  bool has_connection(){
    return has_connection_;
  }
  const boost::asio::ip::tcp::endpoint getEndpoint();

 private:
  void accept_connection();
  void handle_accept(const boost::system::error_code& error);
//  void handle_writer(const boost::system::error_code& error,
//                     std::size_t bytes_transferred, SharedMemory& data);

 private:
  boost::asio::io_service& io_service_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::acceptor acceptor_;
  bool has_connection_;
};

#endif /* TCP_SERVER_HPP_ */

