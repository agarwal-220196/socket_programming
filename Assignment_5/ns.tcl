if { $argc != 2 } {
	puts "Please enter the correct format as follows"
	puts "ns ns2.tcl <TCP_Flavour> <case_num>"
	exit
   }

set user_entered_flavor [lindex $argv 0]
set flavor_case_no [lindex $argv 1]

if { $flavor_case_no > 3 || $flavor_case_no < 1 } {
	puts "$case number entered is not valid "
	exit
}

global actual_tcp_flvr
global time
set time 0

#define the time requirement as mentioned in the PDF. 
switch $flavor_case_no {
	global time
	1 {set time "12.5ms"}
	2 {set time "20ms"}
	3 {set time "27.5ms"}
} 

#First set the TCP flavor as per the entry from the command line. 
if {$user_entered_flavor == "SACK"} {
	set actual_tcp_flvr "Sack1"
} elseif {$user_entered_flavor == "VEGAS"} {
	set actual_tcp_flvr "Vegas"
} else {
	puts "Invalid TCP Flavor $flavor"
	exit
}	


#create the ns simulator
set ns [new Simulator]
set file_name "out_$user_entered_flavor$flavor_case_no"

#two files for comparison
set fptr1 [open output1.tr w]
set fptr2 [open output2.tr w]

#trace
set file_trace [open out_trace.tr w]
$ns trace-all $file_trace

#NAM 
set file_nam [open out_nam.nam w]
$ns namtrace-all $file_nam

#node creation 
set source1 [$ns node]
set source2 [$ns node]
set router1 [$ns node]
set router2 [$ns node]
set receive1 [$ns node]
set receive2 [$ns node]

#color
$ns color 1 Red
$ns color 2 Green

#default values of output
set throughput1 0
set throughput2 0
set count 0

#TCP connection 
set tcp1 [new Agent/TCP/$actual_tcp_flvr]
set tcp2 [new Agent/TCP/$actual_tcp_flvr]
$ns attach-agent $source1 $tcp1
$ns attach-agent $source2 $tcp2

$tcp1 set class_ 1
$tcp2 set class_ 2

# Link
$ns duplex-link $source1 $router1 10.0Mb 5ms DropTail  
$ns duplex-link $receive1 $router2 10.0Mb 5ms DropTail
$ns duplex-link $router1 $router2 1.0Mb 5ms DropTail






































