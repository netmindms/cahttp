cahttp
======
C++ Asynchronous HTTP library

Introduction
--------
Asynchronous HTTP client and server library for C++.
- Asynchronous event driven callback using c++11 lambda function.
- Single thread and multithread model( or multiinstance )
- HTTP 1.1 pipelining support


Installation
------------
  * Install prerequisites.
   - GCC 4.7 or later required.( GCC4.9 recommended.)
   - Install ednio library: 
     refer to [ednio github site](http://github.com/netmindms/ednio)
   - Install cmake: 
    ```
    sudo make install cmake
    ```
  * Build & Install
  
   ```
   $ cmake src_dir
   $ make
   $ sudo make install
   ```

Code Snippet
------------

* client API

    ```cpp
    #include <ednio/EdNio.h>
    #include <cahttp/CaHttpReq.h>
    using namespace std;
    using namespace edft;
    using namespace cahttp;
    int main(int argc, char* argv[]) {
      EdNioInit(); // init ednio library
      EdTask task;
      CaHttpReq request;
      // set task event message callback with lambda function.
      task.setOnListener([&](EdMsg &msg) {
        if(msg.msgid == EDM_INIT) { // task init message
          request.request_get("http://www.google.com", [&](CaHttpReq &req, int event, int status) {
            if(event == req.HTTP_REQ_END) {
              if(!status) {
                int status = req.getRespCode(); // get http response status code
                string resp_data = req.getRespData(); // get http body data
                printf("status code=%d\n", status);
                printf("body\n%s", resp_data.data());
                task.postExit(); // let task terminate.
              }
            }
          });
        } else if(msg.msgid == EDM_CLOSE) { // task close message.
          request.close();
        }
        return 0;
      });
      task.runMain(); // start event loop of task.
      return 0;
    }
    ```

* Server API

    ```cpp
    #include <cahttp/CaHttpServer.h>
    using namespace std;
    using namespace edft;
    using namespace cahttp;

    // /index url path controller
    class IndexUrlCtrl: public CaHttpUrlCtrl {
      void OnHttpReqMsg() override {
        response(200, "hello, cahttp\n");
      }
    };

    int main(int argc, char* argv[]) {
      EdNioInit();
      EdTask task;
      CaHttpServer server;
      task.setOnListener([&](EdMsg &msg) {
        if(msg.msgid == EDM_INIT) {
          // configure http server
          server.config("port", "9000");
          server.config("ip", "0.0.0.0");
          // set url controller
          server.setUrl<IndexUrlCtrl>(HTTP_GET, "/index");

          // start server
          server.start(0); // start event loop in current thread(that is, EdTask)

        } else if(msg.msgid == EDM_CLOSE) {
          server.close();
        }
        return 0;
      });
      task.runMain(); // start event loop
      return 0;
    }

    ```

Release Note
------------

TODO
----
  - HTTPS support
