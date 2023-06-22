Stanford CS 144 Networking Lab
==============================

These labs are open to the public under the (friendly) request that to
preserve their value as a teaching tool, solutions not be posted
publicly by anybody.

Website: https://cs144.stanford.edu

To set up the build system: `cmake -S . -B build`

To compile: `cmake --build build`

To run tests: `cmake --build build --target test`

To run speed benchmarks: `cmake --build build --target speed`

To run clang-tidy (which suggests improvements): `cmake --build build --target tidy`

To format code: `cmake --build build --target format`


https://cs144.github.io

# 虚拟环境的安装：
 [UTM virtual machine software](https://mac.getutm.app/)：在不同设备模拟不同的cpu架构，不同的操作系统 
 [ARM64 GNU/Linux virtual machine image](https://web.stanford.edu/class/cs144/vm_files/cs144-arm64-2023.utm.tar.gz)：该试验所需的Linux环境
 
# 检查点0:预热任务

[[check0.pdf]]  https://cs144.github.io/assignments/check0.pdf

### 1.**手动访问网页**：
http://cs144.keithw.org/lab0/sunetid 
1.发起一个HTTP 连接 指定目标服务器
```shell
telnet cs144.keithw.org http
```
2.发起一个POST请求，指明要访问的资源
```
GET /lab0/1
```
3.告诉服务器请求已经发送，响应后尽快释放连接
```shell
Connection: close
```
4.发送一个空消息，告诉服务器我已经处理完了HTTP请求
```shell
\Enter
```

### 2.**手动发送邮件**
1.在QQ邮件中开启SMTP服务
```shell
telnet smtp.qq.com smtp
```
2.向邮件服务器表明自己的身份
```shell
HELLO qq.com
```
3.登陆
```shell
ahth login
```
4.输入邮箱账号以及授权码的base64编码
```
xxxxxxxx
xxxxxxxx
```
5.指明发邮件的人以及发给谁
```shell
mail from:<1990977934@qq.com>
rcpt to:<652105798@qq.com>
```
6.给出数据
```shell
DATA \ENTER
354 End data with <CR><LF>.<CR><LF>.
From: 1990977934@qq.com \ENTER
To: 652105798@qq.com \ENTER
Subject: Hello! \ENTER
\ENTER
. \ENTER
```

### 3. 监听以及连接
```shell
# -v verbose模式输出更详细的信息 -l 监听模式 -p监听的端口
netcat -v -l -p 9090
```

```shell
telnet localhost 9090
```

### 4. 实现webget
现代c++风格：http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
基本的思想是：用对象的构造函数获取资源，由对象的析构函数自动进行资源回收。这种风格叫做RAII。
作业要求：
• 使用https://en.cppreference.com作为资源进行语言文档查询（不建议使用可能已过时的cplusplus.com）。
• 永远不要使用malloc()或free()。
• 永远不要使用new或delete。
• 仅在必要时使用“智能”指针（unique ptr或shared ptr），基本上不使用原始指针(*)。（在CS144中不需要使用这些。）
• 避免使用模板、线程、锁和虚函数。（在CS144中不需要使用这些。）
• 避免使用C语言风格的字符串（char *str）或字符串函数（strlen()、strcpy()），因为它们容易出错。使用std::string代替。
• 永远不要使用C风格的强制类型转换（例如，（FILE *）x）。如果必须使用，请使用C++的静态转换（在CS144中通常不需要）。
• 首选使用const引用传递函数参数（例如：const Address & address）。
• 除非需要更改变量，否则将每个变量设为const。
• 除非需要更改对象，否则将每个方法设为const。
• 避免使用全局变量，并将每个变量的作用域设为最小。
• 在提交作业之前，运行cmake --build build --target tidy获取有关如何改善与C++编程实践相关的代码的建议，并运行cmake --build build --target format以保持代码格式的一致性。

```c++
void get_URL( const string& host, const string& path )

{

  TCPSocket tcp_socket;

  string pci = "http";

  auto adr = Address( host, pci );

  tcp_socket.connect( adr );

  tcp_socket.write( "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\n" );

  tcp_socket.write( "Connection: close \r\n\r\n" );

 // tcp_socket.shutdown( SHUT_WR );

  vector<vector<string>>ss;

  while ( !tcp_socket.eof() ) {

     string s;

     tcp_socket.read(s);

     cout << s;

  }

  tcp_socket.close();

}
```

```c++
#pragma once  
  
#include <stdexcept>  
#include <string>  
#include <string_view>  
  
class Reader;  
class Writer;  
  
class ByteStream  
{  
protected:  
  uint64_t capacity_;  
  uint64_t num_pushed = 0;  
  uint64_t num_poped = 0;  
  // Please add any additional state to the ByteStream here, and not to the Writer and Reader interfaces.  
  bool error_ = false;  
  bool closed_ = false;  
  bool finshed_ = false;  
  std::string buffer_ = "";  
  
public:  
  // explicit ByteStream(uint64_t capacity) : capacity_{capacity}, buffer__{} {}  
  explicit ByteStream( uint64_t capacity );  
  
  // Helper functions (provided) to access the ByteStream's Reader and Writer interfaces  
  Reader& reader();  
  const Reader& reader() const;  
  Writer& writer();  
  const Writer& writer() const;  
};  
  
class Writer : public ByteStream  
{  
public:  
  void push( std::string data ); // Push data to stream, but only as much as available capacity allows.  
  
  void close();     // Signal that the stream has reached its ending. Nothing more will be written.  
  void set_error(); // Signal that the stream suffered an error.  
  
  bool is_closed() const;              // Has the stream been closed?  
  uint64_t available_capacity() const; // How many bytes can be pushed to the stream right now?  
  uint64_t bytes_pushed() const;       // Total number of bytes cumulatively pushed to the stream  
};  
  
class Reader : public ByteStream  
{  
public:  
  std::string_view peek() const; // Peek at the next bytes in the buffer  
  void pop( uint64_t len );      // Remove `len` bytes from the buffer  
  
  bool is_finished() const; // Is the stream finished (closed and fully popped)?  
  bool has_error() const;   // Has the stream had an error?  
  
  uint64_t bytes_buffered() const; // Number of bytes currently buffered (pushed and not popped)  
  uint64_t bytes_popped() const;   // Total number of bytes cumulatively popped from stream  
};  
  
/*  
 * read: A (provided) helper function thats peeks and pops up to `len` bytes  
 * from a ByteStream Reader into a string;  
 */  
void read( Reader& reader, uint64_t len, std::string& out );
```


```c++
#include "byte_stream.hh"  
#include <algorithm>  
#include <stdexcept>  
#include <string>  
  
using namespace std;  
  
ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}  
// ByteStream::ByteStream(uint64_t capacity) : capacity_{capacity}, buffer__{} {}  
void Writer::push( string data )  
{  
  if ( capacity_ == 0 )  
    return;  
  uint64_t num_pushed_ = min( data.size(), capacity_ );  
  //这里一次性压进去
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
  // Your code here.  
  //  return {};  
  return { capacity_ };  
}  
  
uint64_t Writer::bytes_pushed() const  
{  
  // Your code here.  
  return { num_pushed };  
}  
  
string_view Reader::peek() const  
{  
  // Your code here.  
  // buffer_pre++;  
  // return buffer_.substr(0,1);  
  // std::reverse(buffer_.begin(),buffer_.end());  
  return buffer_;  
}  
  
bool Reader::is_finished() const  
{  
  if ( buffer_.size() == 0 && closed_ )  
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
  //这里模拟一下队，直接把buffer覆盖掉，一开始用的erase最后一个点卡掉了
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
```

![](https://cdn.nlark.com/yuque/0/2023/png/22918222/1686569374130-e439ca0d-ca60-45be-82f8-e2a32c5a8223.png)

要求是超过0.1GB的速度就好，还行～
# 检查点1:将子串拼接为原本的字节流
这个点要实现TCP接收器，实现将：
- 乱序
- 不定长
- 重复
- 空
segment拼接成原本的字节流，并对速率做出了要求。写之前我已经把TCP忘的差不多了。
![](https://cdn.nlark.com/yuque/0/2023/png/22918222/1687233483749-cfec366e-0a22-482e-9e9b-34eae28cb276.png)
每一个segment都由起始索引，以及一个标志标记是否为最后一片数据。
## 1.接口定义。
```c++
#pragma once

#include "byte_stream.hh"

#include <map>
#include <set>
#include <string>
class Reassembler
{
protected:
  //用来进行segment组装的map
  std::map<uint64_t, std::string> buffer = {};
  //泪目
  std::map<uint64_t, uint64_t> buffer_p = {};
  //组装缓冲区的大小
  uint64_t buffer_size = 0;
  //永远指向已经正确接受的字节流末尾
  uint64_t p = 0;
  //泪目
  uint64_t end_p = 0;
  //标记是否接收到了最末尾的segment
  bool tag_end = false;
 
public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};
```
总体来说有四个重要的组件
- 组装字节流的组装器的缓冲区大小 **buffer_size**
- 记录是否接收过结束字节流的标志位 t**ag_end**
- 指向期待的字节流的末尾 **p**（下一个字节流就在这个位置进入）
- 用来缓存segment的hashmap **buffer**

实际上在实际完成以前我设想的组装器和最后实现的完全不同，最初的想法是直接初始化一个与发送方窗口大小一致的string，然后根据segment填就完了。如果数据无重复的话那还还行，但实际的测试样例子可能有一种很有趣~~操蛋~~的情况。
![](https://cdn.nlark.com/yuque/0/2023/png/22918222/1687239221469-3569e558-fadb-4f70-973d-110988b4765b.png)

然这属于比较夸张的比喻，如果只用string存的话，你永远不知道某个位置是不是空的（后边的暴力测试数据字符集是随机的啥都有），所以会碰到很夸张重复写问题。当然可以记录下来每个索引最大连续的长度，然后就涉及到不同segment合并了~~这样用string的意义就没有了，不如直接map~~

## 2.组装器


```c++
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
  if ( !insert_ && first_index > p && len < n && len > 0 && n > 0 ) {
    // 看一下segment的启始位置是否被占用
    auto iter = buffer.find( first_index );

    // 只要没找到就先把segment压到缓冲区
    if ( iter == buffer.end() ) {
      buffer[first_index] = data;
      buffer_size += len;
      iter = buffer.find( first_index );
    }
    // 如果这个位置被占用了且当前segmet的长度没有新来的长
    else if ( iter->second.size() < len ) {
      buffer_size += len - iter->second.size();
      buffer[first_index] = data;
    } else //这个segment没用，直接跳出
      return;
    // 进入到这里，要么是直接压进去了，要么是把原有的segment替换掉了，考虑向左和向右合并
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
          buffer_size -= it_->second.size();
          buffer.erase( it_ );
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
```

结果放出来
![](https://cdn.nlark.com/yuque/0/2023/png/22918222/1687241766217-11c0c809-b276-4187-a1b7-a014f3baa78d.png)

小小的骄傲一把吧～作业的标准是0.1Gbit/s，然后有一句话说得很好。“We have provided you with a benchmark; anything greater than 0.1 Gbit/s (100 megabits per second) is acceptable. **A top-of-the-line Reassembler** will achieve 10 Gbit/s.”

