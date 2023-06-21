#include "byte_stream.hh"
#include <algorithm>
#include <stdexcept>
#include <string>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}
void Writer::push( string data )
{
  if ( capacity_ == 0 )
    return;
  uint64_t num_pushed_ = min( data.size(), capacity_ );

  string temp = data.substr( 0, num_pushed_ );
  buffer_ += temp;
  capacity_ = capacity_ - num_pushed_;
  num_pushed += num_pushed_;
}

void Writer::close()
{
  closed_ = true;
}

void Writer::set_error()
{
  error_ = true;
}

bool Writer::is_closed() const
{
  // Your code here.
  if ( closed_ )
    return true;
  return false;
}

uint64_t Writer::available_capacity() const
{
  return { capacity_ };
}

uint64_t Writer::bytes_pushed() const
{
  return { num_pushed };
}

string_view Reader::peek() const
{
  return buffer_;
}

bool Reader::is_finished() const
{
  if ( closed_ && buffer_.size() == 0 )
    return true;
  return false;
}

bool Reader::has_error() const
{
  if ( !error_ )
    return false;
  return true;
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  uint64_t maxx_pop = min( len, uint64_t( buffer_.size() ) );
  buffer_ = buffer_.substr( maxx_pop, buffer_.size() );
  capacity_ += maxx_pop;
  num_poped += maxx_pop;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return { num_poped };
}
