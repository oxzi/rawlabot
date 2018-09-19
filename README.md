# rawlabot

Dump a *Walabot Makers* raw signal


## rawlabot.cpp

```
rawlabot

  --help
    prints this message

  --print-antennas
    prints list of antennas (TX/RX)

  --antennas TX1:RX1,TX2:RX2,..
    dump data from antenna pairs as in --print-antennas

  --profile short-range|sensor|sensor-narrow
    use one of those three profiles (defaults to sensor)

  --arena-r START,END,RES
    set arena's radial range (defaults to 10,60,1)

  --arena-theta START,END,RES
    set arena's polar range (default to -15,15,5)

  --arena-phi START,END,RES
    set arena's azimuth range (defaults to -60,60,5)

  --runs NUMB
    query the antenna pairs NUMB times and exit
```

Stores the raw signals between TX antenna *i* and RX antenna *j* in a CSV file.
The following command will create 17-14.csv, 4-2.csv and 18-11.csv.

```bash
./rawlabot --antennas 17:14,4:2,18:11
```

Each CSV file has a `run`, `no` and `signal` column.

- `run`: the *nth* cycle
- `no`: each run creates a lot of values
- `signal`: the signal's strength

### Build

You need to have the Walabot SDK installed.

```bash
make
# a rawlabot executable should exist now
```

Currently this tool was only tested on a GNU/Linux system.


## plot.py

This Python script plots the dumped CSV files as multiple (amount of runs) PNG
files. It exists mostly to test the result of the rawlabot.

```bash
./plot.py *.csv
```
