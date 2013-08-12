
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/date_time.hpp>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>

using boost::asio::ip::tcp;

#define IMAGE_SIZE 360960 //=752*480


typedef struct
{
  uint32_t timestamp;
  uint32_t data_size;
  uint32_t data_id;

} DataHeader;

cv::Mat convertImage(unsigned char* dataBuffer)
{
  unsigned int width = 752;
  unsigned int height = 480;
  cv::Mat image(cv::Size(width, height), CV_8UC1, dataBuffer, cv::Mat::AUTO_STEP);
  return image;
}

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

    cv::namedWindow( "Camera 0", CV_WINDOW_AUTOSIZE);// Create a window for display.
    cv::waitKey(1);
    for (;;)
    {
      //			boost::array<char, IMAGE_SIZE+1> buf;
      unsigned char buf[IMAGE_SIZE];
      boost::system::error_code error;

      printf("waiting for image.\n");
      //      size_t len = socket.read_some(boost::asio::buffer(buf), error);


      // read header
      std::vector<uint32_t> header_payload(3,0);

      size_t len = boost::asio::read(socket, boost::asio::buffer(header_payload, sizeof(header_payload)), error);
      std::printf("len %d\n",len);

      if (error == boost::asio::error::eof)
      {
        printf("connection closed\n");
        break; // Connection closed cleanly by peer.
      }
      else if (error)
        throw boost::system::system_error(error); // Some other error.



      DataHeader header;
      header.timestamp = ntohl(header_payload[0]);
      header.data_size = ntohl(header_payload[1]);
      header.data_id = ntohl(header_payload[2]);

      printf("header: %d, %d, %d\n", header.timestamp, header.data_size, header.data_id);

      //			for(int i = 0; i<40; i+=4)
      //			{
      //			  unsigned int * word = (unsigned int *)(buf + i);
      //		    // Dereference and convert it.
      //			  printf("%x ", (*word));
      //			}
      //      printf("\n");


      switch(header.data_id)
      {
        case 100:
        {
          // read payload
          len = boost::asio::read(socket, boost::asio::buffer(buf, header.data_size), error);
          std::printf("len %d\n",len);

          boost::posix_time::ptime time_now(boost::posix_time::microsec_clock::local_time());
          std::printf("frequenz: %f delta t %f\n", (double)1000000.0/(time_now - last_time).total_microseconds(), (double)(time_now - last_time).total_microseconds());
          last_time = time_now;

          cv::Mat image = convertImage(buf);
          cv::imshow( "Camera 0", image );                   // Show our image inside it.
          cv::waitKey(1);
          break;
        }
        case 101:
          len = boost::asio::read(socket, boost::asio::buffer(buf, header.data_size), error);
          break;
        case 102:
          len = boost::asio::read(socket, boost::asio::buffer(buf, header.data_size), error);
          break;
        case 103:
          len = boost::asio::read(socket, boost::asio::buffer(buf, header.data_size), error);
          break;
        case 10:
          len = boost::asio::read(socket, boost::asio::buffer(buf, header.data_size), error);
          break;
        default:
          printf("Error: Data ID not known: %d", header.data_id);
          break;
      }
    }
    socket.close();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
