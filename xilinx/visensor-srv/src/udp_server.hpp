/*
 * udp_server.hpp
 *
 *  Created on: Jun 18, 2013
 *      Author: pascal
 */

#ifndef UDP_SERVER_HPP_
#define UDP_SERVER_HPP_

#include <boost/asio.hpp>

class UdpServer {
 public:
  UdpServer(boost::asio::io_service& io_service);
  virtual ~UdpServer();

  void send_data(const int cam, char* data, const boost::asio::ip::udp::endpoint& remote_endpoint);

 private:
  void handle_send(char* data);

 private:
  boost::asio::ip::udp::socket socket_;
};

#endif /* UDP_SERVER_HPP_ */
