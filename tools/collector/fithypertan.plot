reset
f(x) = A + B * x
A = 90
B = 0
set yr [0.8:1.0]
fit f(x) "mfm-city-CANAL_ROUTING-2.dat" using ($1):($4) via A, B
plot "mfm-city-CANAL_ROUTING-2.dat" using ($1):($4) with linespoints, f(x)