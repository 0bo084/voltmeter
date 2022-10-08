/*
	voltmeter
*/

/*!
	Exception class for all exceptions thrown by test example.
*/

#pragma once

#include <string>
#include <stdexcept>
#include <string_view>


namespace voltio
{

//
// exception_t
//

//! Exception class for all exceptions thrown by Voltmeter
class exception_t
	:	public std::runtime_error
{
	using base_type_t = std::runtime_error;
	public:
		exception_t( const char * err )
			:	base_type_t{ err }
		{}

		exception_t( const std::string & err )
			:	base_type_t{ err }
		{}

		exception_t( std::string_view err )
			:	base_type_t{ std::string{ err.data(), err.size() } }
		{}
};

} /* namespace voltio */
