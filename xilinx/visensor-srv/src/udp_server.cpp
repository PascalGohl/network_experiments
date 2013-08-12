/*
 * udp_server.cpp
 *
 *  Created on: Jun 18, 2013
 *      Author: pascal
 */

#include <boost/bind.hpp>
#include "udp_server.hpp"

using boost::asio::ip::udp;

#define MAX_PAKET_SIZE 1500
#define IMAGE_SIZE 1500
#define UDP_PORT 13779

UdpServer::UdpServer(boost::asio::io_service& io_service)
: socket_(io_service, udp::endpoint(udp::v4(), UDP_PORT)) {
}

UdpServer::~UdpServer() {}

void UdpServer::send_data(const int cam, char* data, const udp::endpoint& remote_endpoint)
{
  size_t sendlen = std::min(IMAGE_SIZE, MAX_PAKET_SIZE);
  size_t remlen  = IMAGE_SIZE;
  const char *curpos = data;

  boost::array<int, 2> header = {cam, 0};

  while (remlen > 0)
  {
    // concatenate the message header and the remaining data
    std::vector<boost::asio::const_buffer> buffer_sequence;
    buffer_sequence.push_back(boost::asio::buffer(header));
    buffer_sequence.push_back(boost::asio::buffer(curpos, sendlen));

//    printf("data length: %d\n",sendlen );
    // send a datagram
    ssize_t len = socket_.send_to(buffer_sequence, remote_endpoint);
    if (len == -1)
      return;

    curpos += len - sizeof(header);
    remlen -= len - sizeof(header);
    sendlen = std::min((int)remlen, MAX_PAKET_SIZE);
    header[1]++;
  }
//  printf("message sent in %d packages\n", header[0] );
}

void UdpServer::handle_send(char* data)

{
  // mark data as sent
}
