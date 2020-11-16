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

$ns duplex-link $source2 $router1 10.0Mb $time DropTail
$ns duplex-link $receive2 $router2 10.0Mb $time DropTail

#node positions && flow of link
$ns duplex-link-op $router1 $router2 orient right
$ns duplex-link-op $source1 $router1 orient right-down
$ns duplex-link-op $source2 $router1 orient right-up
$ns duplex-link-op $router2 $receive1 orient right-up
$ns duplex-link-op $router2 $receive2 orient right-down

#Definition

set ftp_1 [new Application/FTP]
set ftp_2 [new Application/FTP]

$ftp_1 attach-agent $tcp1
$ftp_2 attach-agent $tcp2

set sink_1 [new Agent/TCPSink]
set sink_2 [new Agent/TCPSink]
$ns attach-agent $receive1 $sink_1
$ns attach-agent $receive2 $sink_2

$ns connect $tcp1 $sink_1
$ns connect $tcp2 $sink_2

#PROCEDURE
proc finish {} {
	global ns nf file_trace file_nam file_name throughput1 throughput2 count
	#global fptr1
	$ns flush-trace
	#close $f1
	puts "AVERAGE THROUGHPUT FOR SOURCE 1 = [expr $throughput1/$count] MBits/sec \n"
	puts "AVERAGE THROUGHPUT FOR SOURCE 2 = [expr $throughput2/$count] MBits/sec \n"
	close $file_trace
	close $file_nam
	exec nam out_nam.nam &
	# exec xgraph output1.tr output2.tr -geometry 800x400 &
	exit 0
}

proc record {} {
	global sink_1 sink_2 fptr1 fptr2 throughput1 throughput2 count

	set ns [Simulator instance]
	#Time after which the procedure is called again
	set time_set 0.5

	#bytes received
	set byte_1 [$sink_1 set bytes_]
	set byte_2 [$sink_2 set bytes_]

	#CURRENT TIME
	set now [$ns now]

	#BW CALCULATION
	puts $fptr1 "$now [expr $byte_1/$time_set*8/1000000] "
	puts $fptr2 "$now [expr $byte_2/$time_set*8/1000000] "
	set throughput1 [expr $throughput1 + $byte_1/$time_set*8/1000000]
	set throughput2 [expr $throughput2 + $byte_2/$time_set*8/1000000]
	set count [expr $count + 1]

	#RELEASE BYTES ON TRAFFIC SINK
	$sink_1 set bytes_ 0
	$sink_2 set bytes_ 0

	$ns at [expr $now+$time_set] "record"

}

$ns at 0 "record"
$ns at 0 "$ftp_1 start"
$ns at 0 "$ftp_2 start"
$ns at 400 "$ftp_1 stop"
$ns at 400 "$ftp_2 stop"
$ns at 400 "finish"
$ns color 1 Blue
$ns color 2 Red

set trace_file_1 [open "$file_name--source1.tr" w]
$ns trace-queue $source1 $router1 $trace_file_1

set trace_file_2 [open "$file_name--source2.tr" w]
$ns trace-queue $source2 $router1 $trace_file_2

$ns run









































