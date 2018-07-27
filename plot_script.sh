#!/bin/sh

FILE_NAME="simulation_outputs"

TG_NAME=$(pwd)/build/
TG_PATH="$TG_NAME/examples/energy"

touch $FILE_NAME.gp
chmod +x $FILE_NAME.gp

cat > $FILE_NAME.gp <<EOF
#!/usr/bin/gnuplot

#set terminal pdf color
set grid
set xlabel 'Sample'
set ylabel 'Power Consumption (W)'
#set key top left
set key outside
set key above
set key center center

set style line 100 lt 1 lc rgb "gray" lw 2
set style line 101 lt 1 lc rgb "gray" lw 1
set grid xtics ytics ls 100
set grid mxtics mytics ls 101

#set yrange [:4]
#set xrange [0.1:2.1]

#set logscale y

#set output './$FILE_NAME.pdf'
plot \\
EOF

FIRST=1
KIND_COUNTER=1
for KIND in BIG LITTLE; do
	COUNTER=0
	for file in $(find "$TG_PATH" -name power_*$KIND* | sort); do
		if [ "$FIRST" -ne 1 ]; then
			echo ", \\" >> $FILE_NAME.gp
		else
			FIRST=0
		fi
		PLOT_NAME="$(basename $file | sed -e 's/_perf.sh_'$KIND'.txt//g' -e 's/_/\\_/g') on $KIND"
		echo -n "\"< cat $file | cut -d ':' -f2 \" u 0:1 pointtype $KIND_COUNTER lc $COUNTER lw 1 t '$PLOT_NAME'"  >> $FILE_NAME.gp
		COUNTER=$((COUNTER + 1))
	done
	KIND_COUNTER=2
done

gnuplot --persist ./$FILE_NAME.gp -
