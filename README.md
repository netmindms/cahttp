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
   - Build & Install
   ```
   $ cmake src_dir
   $ make
   $ sudo make install
   ```

Code Snippet
------------
  - client API
  ```
  ...
  CaHttpReq req;
  ...
  req.request_get("http://www.google.com", [](CaHttpReq &req, int event, int status) {
    if(event == HTTP_REQ_END) {
      int status_code = req.getRespCode(); // http response status code
      std::string resp_data = req.getRespData(); // response body data
      ...
    }
	});
  ...
  ```
  - Server API
  ```
  class IndexUrlCtrl : public CaHttpUrlCtrl {
    void OnHttpReqMsg() override {
      response(200, "Hello,...");
    }
  }
  ...
  CaHttpSvr server;
  ...
  server.config("port", 9090);
  server.setUrl<IndexUrlCtrl>(HTTP_GET, "/index");
  server.start(0);
  ...
  ```

Release Note
------------

TODO
----
  - HTTPS support
