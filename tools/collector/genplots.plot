set term png size 1280,720
set yr [70:100]

set key below

# Canal averaging
set output "CANAL_ROUTING-average.png"

set title "Car arrival percentage using Canal Routing per epoch"
set xlabel "Elapsed AEPs"
set ylabel "Car arrival percentage"

f(x) = A
A = 98.2203
plot "CANAL_ROUTING-average.dat" using 1:($4 * 100) title 'Arrival Percentage' with linespoints, f(x) title 'Fit [10k-20k] : 98.2203'



# Sidewalk Only averaging
set output "NOT_USED-average.png"

set title "Car arrival percentage using Sidewalk-Only Routing per epoch"
set xlabel "Elapsed AEPs"
set ylabel "Car arrival percentage"

g(x) = B
B = 98.2038
plot "NOT_USED-average.dat" using 1:($4 * 100) title 'Arrival Percentage' with linespoints, g(x) title 'Fit [10k-20k] : 98.2038'


# Random averaging
set output "RANDOM_ROUTING-average.png"

set title "Car arrival percentage using Random Routing per epoch"
set xlabel "Elapsed AEPs"
set ylabel "Car arrival percentage"

h(x) = C
C = 91.5425
plot "RANDOM_ROUTING-average.dat" using 1:($4 * 100) title 'Arrival Percentage' with linespoints, h(x) title 'Fit [10k-20k] : 91.5425'


# All average routing in one graph
set output "ALL_ROUTING-average.png"

set title "Car arrival percentage per epoch"
set xlabel "Elapsed AEPs"
set ylabel "Car arrival percentage"

plot "RANDOM_ROUTING-average.dat" using 1:($4 * 100) title 'Arrival Percentage, Random routing' with linespoints, h(x) title 'Random Fit [10k-20k] : 91.5425', "NOT_USED-average.dat" using 1:($4 * 100) title 'Arrival Percentage, Sidewalk-Only' with linespoints, g(x) title 'Sidewalk-Only Fit [10k-20k] : 98.2038', "CANAL_ROUTING-average.dat" using 1:($4 * 100) title 'Arrival Percentage, Canaling' with linespoints, f(x) title 'Canaling Fit [10k-20k] : 98.2203'




set yr [0:35]
# Average travel time on random
set output "RANDOM_ROUTING-arrival_rate.png"

set title "Average car travel time of consumed cars per epoch"
set xlabel "Elapsed AEPS"
set ylabel "Car travel time"

f(x) = A + B
A = 35
B = -1
fit [2000:20000] f(x) "RANDOM_ROUTING-average.dat" using 1:3 via A, B
plot "RANDOM_ROUTING-average.dat" using 1:3 title 'Average car travel time' with linespoints, f(x) title 'Random fit [2k-20k] : 29.4626'


# Average travel time on sidewalk-only
set output "NOT_USED-arrival_rate.png"

set title "Average car travel time of consumed cars per epoch"
set xlabel "Elapsed AEPS"
set ylabel "Car travel time"

g(x) = D + E * atan(x * F)
D = 30
E = -1
F = 1
fit [2000:20000] g(x) "NOT_USED-average.dat" using 1:3 via D, E, F
plot "NOT_USED-average.dat" using 1:3 title 'Average car travel time' with linespoints, g(x) title 'Sidewalk-Only fit [2k-20k] : 20.8466'


# Average travel time on canal
set output "CANAL_ROUTING-arrival_rate.png"

set title "Average car travel time of consumed cars per epoch"
set xlabel "Elapsed AEPS"
set ylabel "Car travel time"

#h(x) = C
#C = 18.6296
#fit [10000:20000] h(x) "CANAL_ROUTING-average.dat" using 1:3 via C
h(x) = G + H * atan(x * I)
G = 30
H = -1
I = 1
fit [2000:20000] h(x) "CANAL_ROUTING-average.dat" using 1:3 via G, H, I
#plot "CANAL_ROUTING-average.dat" using 1:3 title 'Average car travel time' with linespoints, h(x) title 'Canal fit [10k-20k] : 18.6296'
plot "CANAL_ROUTING-average.dat" using 1:3 title 'Average car travel time' with linespoints, h(x) title 'Canal fit [2k-20k] : 30.0369 - 7.80858 atan(0.000593311 * x)'
# by the way, this limit approaches 17.7712


# All three travel time plots
set output "ALL_ROUTING-arrival_rate.png"

set title "Average car travel time of consumed cars per epoch"
set xlabel "Elapsed AEPS"
set ylabel "Car travel time"
plot "RANDOM_ROUTING-average.dat" using 1:3 title 'Average car travel time' with linespoints, f(x) title 'Random fit [10k-20k] : 29.4626', "NOT_USED-average.dat" using 1:3 title 'Average car travel time' with linespoints, g(x) title 'Sidewalk-Only fit [10k-20k] : 20.8466', "CANAL_ROUTING-average.dat" using 1:3 title 'Average car travel time' with linespoints, h(x) title 'Canal fit [2k-20k] : 30.0369 - 7.80858 atan(0.000593311 * x)'