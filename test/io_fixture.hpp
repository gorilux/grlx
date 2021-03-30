#pragma once


#include <boost/asio/io_context.hpp>

#include <string>
#include <thread>


namespace grlx::test {

namespace asio = boost::asio;

struct io_fixture {
  asio::io_context io_context;
  asio::executor   executor;

  using work_guard = asio::executor_work_guard<asio::executor>;

  io_fixture()
    : executor(io_context.get_executor()),
      work(asio::make_work_guard(executor)),
      worker([this]() {         
        io_context.run(); })
  {
    
  }
  virtual ~io_fixture()
  {
    work.reset();
    worker.join();
  }

private:
  work_guard work;
  std::thread worker;

};

} // namespace grlx::test