# logic-engine
A utility to add logic to a knx system using scheme scripts.  
* Execute scheme procedures that can send commands and read values on a knx bus.  
* Handle scheme callbacks for events on a knx bus.  

### Prerequisites
* A running instance of knxd is needed to access the knx bus
* Required libraries:
    * The libeibclient library (supplied with knxd)
    * Chibi Scheme version >= 0.8

##### Getting the libeibclient library:
* Clone the knxd repository from [https://github.com/knxd/knxd](https://github.com/knxd/knxd)
* Compile knxd using `./configure && make`
* Copy libeibclient library found in: `knxd/src/client/c/.libs/libeibclient.a` to your libs directory e.g. /usr/lib

### Scheme Script:
All configuration and logic is defined in a Chibi Scheme script.  

##### KNXD Url
Bind the knxd url to the`knx-url` symbol e.g.`(define knx-url "ip:192.168.1.15:1234")`

##### knx:send
Send a telegram using: `(knx:send group state)`  
* `group` is a group address string e.g. `"1/2/3"`
* `state` is a boolean value for off or on e.g. on =`#t`

##### knx:read
Read a knx group value: `(knx:read group)`
* `group` is a group address string e.g. `"1/2/3"`
* returns the boolean value of the group e.g. on =`#t`

##### knx:watch
Listen to the knx bus and run callback functions on events: `(knx:watch rule-list)`
* `rule-list` is an associated list of:
  * a group address to trigger on
  * a callback function, takes 1 argument for the new group value

## Usage
Compile the binary using `make`, then run the binary with a scheme script as an argument: 
`logic-engine example.scm`

### Notes
* Currently only supports sending/receiving from group addresses with binary datapoint types