# os-sudoku-sim

## How to build

See `Makefile` for the build scripts.

To build the project, simply run `make` in the root directory of the project.

This will generate two executables called whatever `monitor` and `server`. The `server` executable must be run first and passed a configuration file as an argument, i.e.

```
./build/server server.conf
```

The server can then accept incoming connections from monitors and the simulation will start when a minimum amount of monitors have connected. Like the server, the `monitor` executable must be passed a configuration file as an argument:

```
./build/monitor monitor.conf
```

Log files and executables can be cleared with `make clean`.

## Configuration flags
### Server

| Flag             	| Type  	| Description                                                                                                                                                                                                                    	|
|------------------	|-------	|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------	|
| `socket_backlog` 	| `int` 	| Limit the number of incoming connect requests the server will retain in a queue assuming it can serve the current request and the small amount of queued pending requests in a reasonable amount of time while under high load 	|
| `min_monitors`   	| `int` 	| Minimum amount of monitors needed to connect before the simulation starts                                                                                                                                                      	|
| `dispatch_batch` 	| `int` 	| Number of requests processed each time the `dispatch` thread is called                                                                                                                                                         	|

### Monitor

| Flag              	| Type     	| Description                                                                     	|
|-------------------	|----------	|---------------------------------------------------------------------------------	|
| `arrival_time_ms` 	| `int`    	| The average amount of time, in milliseconds, between each message from a thread 	|
| `threads`         	| `int`    	| Number of playing threads                                                       	|
| `server_address`  	| `string` 	| The IPv4 address of the server                                                  	|