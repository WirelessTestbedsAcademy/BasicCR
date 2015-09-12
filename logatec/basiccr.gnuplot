set pm3d map
set xlabel "frequency [MHz]"
set ylabel "time [s]"
set cblabel "power [dBm]"
unset key

set term wxt 0
set title "Recorded by node 51"
splot "data/node_51.csv" using ($2/1e6):1:3

set term wxt 1
set title "Recorded by node 58"
splot "data/node_58.csv" using ($2/1e6):1:3
