#include <libserial/SerialPort.h>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

constexpr const char* const SERIAL_PORT = "/dev/ttyACM0" ;

int main(){
	using namespace LibSerial;
	//settings for serial port
	SerialPort mySerialPort; //create serial port;
	
	try{
		mySerialPort.Open(SERIAL_PORT); //open the hardware port
	}

	catch(const OpenFailed&){
		std::cerr << "The serial port did not open correctly" << std::endl ;
		return EXIT_FAILURE;
	}

	//set baud rate
	mySerialPort.SetBaudRate(BaudRate::BAUD_115200);

	while(!mySerialPort.IsDataAvailable())
	{
		usleep(1000);
	}
	size_t ms_timeout = 250;
	char data_byte;

	try
	{
		mySerialPort.ReadByte(data_byte,ms_timeout);
		std::cout<<data_byte<<std::flush;
	}
	catch(const ReadTimeout&)
	{
		std::cerr << "\nThe ReadByte() call has timed out"<<std::endl;
	}
	usleep(1000);
	DataBuffer read_buffer;

	try
	{
		mySerialPort.Read(read_buffer, 0, ms_timeout);
	}
	catch(const ReadTimeout&)
	{
		for(size_t i =0; i < read_buffer.size(); i++)
		{
			std::cout<<read_buffer.at(i) << std::flush;
		}
		std::cerr<<"The Read() call timed out waiting for additional data"<<std::endl;
	}


	//writing to the serial port
	//std::string myString = "Hey, Arduino";
	//std::cout << "writing" << std::endl;
	//mySerialPort.Write(myString);

	//closing
	mySerialPort.Close();

}
