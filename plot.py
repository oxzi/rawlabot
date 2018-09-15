#!/usr/bin/env python3

import csv
import sys
import matplotlib.pyplot as plt


FIG_NAME = 'plot-{:05}.png'


def parse_csv(filename):
    'Parses the given filename and returns a dict of run no to list of values.'
    data = {}

    with open(filename, newline='') as csv_file:
        for row in csv.DictReader(csv_file):
            run, signal = int(row['run']), float(row['signal'])
            data.setdefault(run, []).append(signal)

    return data


if __name__ == '__main__':
    if len(sys.argv) == 1:
        print('Useage: ./plot.py *.csv\n')
        exit(1)

    data = [(fn.rstrip('.csv'), parse_csv(fn)) for fn in sys.argv[1:]]

    for run in data[0][1].keys():
        for csv_set in data:
            plt.plot(csv_set[1][run], alpha=0.5, label=csv_set[0])
        plt.legend()
        plt.savefig(FIG_NAME.format(run))
        plt.gcf().clear()
