#include "reassembler.hh"
#include <string>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{ // 碰到结尾的segment将全局标志位置为零
  if ( is_last_substring ) {
    tag_end = true;
    end_p = first_index + (uint64_t)data.size();
  }
  bool insert_ = false;
  uint64_t n = output.available_capacity();
  uint64_t len = (uint64_t)data.size();
  // 如果当前segment的索引与指向缓冲区的头指针重合，并且segment不为空则直接压入缓冲区
  if ( first_index == p && len > 0 && n > 0 ) {
    output.push( data.substr( 0, min( n, len ) ) );
    p += min( n, len );
    insert_ = true;
    // tag = true;
  }
  // 当前segment在缓冲区尾指针之前，并且可以压入缓冲区
  else if ( len > 0 && p > 0 && first_index < p && first_index + len > p && n > 0 ) {
    output.push( data.substr( p - first_index, min( n, first_index + len - p ) ) );
    p += min( n, first_index + len - p );

    // n -= first_index+len-p;
    insert_ = true;
  }
  // segment的情况只剩下在p之后，如果组装器有足够的空间就可以考虑压入组装器缓存
  if ( !insert_ && first_index > p && len+buffer_size <= n && len > 0 && n > 0 ) {
    // 看一下segment的启始位置是否被占用
    auto iter = buffer.find( first_index );
    // 只要没找到就先把segment压到缓冲区
    if ( iter == buffer.end() ) {
      if(first_index + len>p+n){
      buffer[first_index] = data.substr(0,first_index + len-p-n);
      buffer_size += first_index + len-p-n;
      iter = buffer.find( first_index );
      }
      else {
      buffer[first_index] = data;
      buffer_size += len;
      iter = buffer.find( first_index );
      }
    }
    // 如果这个位置被占用了且当前segmet的长度没有新来的长
    else if ( iter->second.size() < len ) {
      buffer_size += len - iter->second.size();
      buffer[first_index] = data;
    } else
      return;
    // 要么是直接压进去了，要么是替换掉了，考虑向左和向右合并
    auto it = buffer.lower_bound( first_index );
    // 是否有机会可以和左边的匹配
    if ( it != buffer.begin() ) {
      it--;
      // 两者有交集可以合并
      if ( iter->first <= it->first + it->second.size() ) {
        // 如果要合并的segment没有当前的块范围大
        if ( it->first + it->second.size() >= iter->first + iter->second.size() ) {
          buffer_size -= iter->second.size();
          buffer.erase( iter );
          
          return;
        } else {
          buffer_size -= it->first + it->second.size() - iter->first;
          string temp1 = iter->second.substr( it->first + it->second.size() - iter->first,
                                              iter->second.size() - it->first - it->second.size() + iter->first );
          string temp2 = it->second;
          it->second = temp2 + temp1;
          buffer.erase( iter );
          iter = it;
        }
      }
    }

    auto it_ = buffer.upper_bound( iter->first );
    // 是否有机会可以和右边的匹配
    if ( it_ != buffer.end() ) {
      // 两者有交集可以合并
      if ( iter->first + iter->second.size() >= it_->first ) {
        // 如果要合并的segment没有当前的块范围大
        if ( iter->first + iter->second.size() >= it_->first + it_->second.size() ) {
          //buffer_size -= it_->second.size();
          //buffer.erase( it_ );
 	  //处理新来的块吞并组装器右边块的情况
     	  while ( ( it_ != buffer.end() && it_->first +it_->second.size() <= iter->first + iter->second.size() ) ) { 
            //if ( erase_->first + erase_->second.size() <= p ) {
              buffer_size -= it_->second.size();
              it_ = buffer.erase( it_ ); // 删除元素，并返回指向下一个元素的迭代器
              }
           // }
        } else {
          buffer_size -= iter->first + iter->second.size() - it_->first;
          string temp1 = it_->second;
          string temp2 = iter->second.substr( 0, it_->first - iter->first );
          iter->second = temp2 + temp1;
          buffer.erase( it_ );
        }
      }
    }
  }
  // 从缓存拿数据
  auto it = buffer.find( p );
  if ( it != buffer.end() ) {
    string temp = it->second;
    output.push( temp );
    p += it->second.size();
    buffer_size -= it->second.size();
    buffer.erase( it );
  }
  // 清除不必要的block
  if ( insert_ ) {
    auto erase_ = buffer.begin();
    while ( ( erase_ != buffer.end() && erase_->first <= p - 1 ) ) { // 遍历map，删除小于a的元素
      if ( erase_->first + erase_->second.size() <= p ) {
        buffer_size -= erase_->second.size();
        erase_ = buffer.erase( erase_ ); // 删除元素，并返回指向下一个元素的迭代器
      }
      // 这里还需要考虑存在很长的一个缓
      else {
        string temp = erase_->second.substr( p - erase_->first, erase_->first + erase_->second.size() - p );
        output.push( temp );
        p += erase_->first + erase_->second.size() - p;
        buffer_size -= erase_->second.size();
        erase_ = buffer.erase( erase_ );
      }
    }
  }

  // 看下关闭信号
  if ( tag_end && p >= end_p )
    output.close();
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return { buffer_size };
}
