/*
	voltio
*/

/*!
	Server.
*/

#pragma once

#include "exception.hpp"
#include "epoll.hpp"
#include "values.hpp"
#include "voltmeter.hpp"
#include "command.hpp"



namespace voltio
{

	class voltmeter_server: protected epoll<voltmeter_server>
	{
		friend class epoll<voltmeter_server>;
		
		static constexpr size_t SetVoltsValueIntervalMS = 1000; // 1 sec
		voltmeter 		device;
		dataProvider	provider;



	public:
		
		// override epoll handler
		void receive_handler(int fd, char* buff, int size)
		{
			std::unique_ptr<command> cmd = cmdFactory::make( buff, size, device);

			if( nullptr == cmd.get()) {

				// serialization error
				shutdown(fd, SHUT_RDWR); // close connection
				return;
			}

			response resp = cmd->execute();

			std::string ser;
			ser.resize(epoll::SizeOfRecvBuffer);
			
			cmdFactory::make( ser, resp);
			
			send(fd, ser.c_str(), ser.length(), 0);

			cmd.release();
		}

	public:

		voltmeter_server(const std::string& path)
			: epoll<voltmeter_server>(path) // exception
		{
			provider.start( [this](){ 
					device.set(provider);
				}, 
				SetVoltsValueIntervalMS
				);
			
		}

		void stop()
		{
			provider.stop();
			epoll<voltmeter_server>::stop();
		}

		

	};

} /* namespace voltio */