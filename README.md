***XCP Test Suite***
===========================
This is a collection of TCL scripts designed to emulate the different XCP performance in ns2 and ns3 (experimental support). This repository is built on ns 2.35. But the script itself is self-contained.

## Starting ##
The following assumes you are running Ubuntu or a debian variant (this was tested in Mint).

Run the follwoing command

    sudo apt-get install ns2  nam xgraph

This should install xgraph which we will be using to create the graphs.
Once that is done run the following in the terminal and it should bring up the graphs

    ns chagolchana.tcl
    
The topology we try to emulate here is (you can change it of course)

--------- Creating the TOPOLOGY --------------------
       n0                             
       : \   Bottleneck            
       :  R0-----------------------R1
         /                            
       ni                         


### Done as part of COMP 529 ###
