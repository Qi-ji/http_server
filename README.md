# lws
### General production
`LWS` - lite web server
A lite http protocol parser and generator used on linux platform.    
Main functions:
* support local port web accessing
* support http protocol parsing
* support web serice definition
* support only linux system

### Build
To build executable file by command-line utility:
> make clean && make

### Usage
```
Usage: lws_tool [options...]
Options:
    -s  start local service
    -p port  select local port, default is 8000
    -l level  set syslog level, 0-all,1-sys,2-error,3-warning,4-info
              default log level is 3-warning
    -h  print usage information
```
