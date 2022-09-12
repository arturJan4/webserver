# webserver
Written for Computer Networks course.

## Description
Simple HTTP server handling GET requests and responding with proper respond codes and content

- responds with correct response codes and content according to HTTP standard (at least for codes 200, 301, 403, 404, 501)
- allows file download of multiple types (octet-stream)
- it keeps the connection alive with the client (if not given connection: close)
- protects againts simple filepath abuse (`..` in path), allows access only for given directories
- doesn't use active wait
- gives errors on prepared incomplete HTTP requests

task description (in Polish) in file `p4.pdf`

## Usage
`make` -> build the project and create the executable  
`make clean` -> cleans temporary objects
`make distclean` -> cleans temporary objects and the executable
