set terminal svg
set output "sirius-s.svg"
set title '[0,0,1] -> [0,0,6] s1/s2 (starting point) 10/100 (steps)'
set xlabel 'l'
set ylabel 'kphi'
plot \
     's1-11.dat' u 3:7 w lp t 's1-10',\
     's2-11.dat' u 3:7 w lp t 's2-10',\
     's1-101.dat' u 3:7 w lp t 's1-100',\
     's2-101.dat' u 3:7 w lp t 's2-100'
