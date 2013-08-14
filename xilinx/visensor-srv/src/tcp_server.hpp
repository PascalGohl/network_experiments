
#ifndef TCP_SERVER_HPP_
#define TCP_SERVER_HPP_

#include <boost/asio.hpp>

typedef struct
{
	uint32_t timestamp;
	uint32_t data_size;
	uint32_t data_id;

	void print()
	{
	  printf("timestamp: %d\n",timestamp);
    printf("data_size: %d\n",data_size);
    printf("data_id: %d\n",data_id);
	}
} DataHeader;

class TcpServer {
public:
	TcpServer(boost::asio::io_service& io_service, unsigned short int port);
	virtual ~TcpServer();

	void send_data(char* data, DataHeader& header);
	bool has_connection(){
		return has_connection_;
	}
	const boost::asio::ip::tcp::endpoint getEndpoint();

private:
	void read();
	void handle_read(const boost::system::error_code& error,
			std::size_t bytes_transferred);
	void accept_connection();
	void handle_accept(const boost::system::error_code& error);
	//  void handle_writer(const boost::system::error_code& error,
	//                     std::size_t bytes_transferred, SharedMemory& data);

private:
	boost::asio::io_service& io_service_;
	boost::asio::ip::tcp::socket socket_;
	boost::asio::ip::tcp::acceptor acceptor_;
	bool has_connection_;
	boost::array<char, 1024> buffer_;
};

#endif /* TCP_SERVER_HPP_ */

