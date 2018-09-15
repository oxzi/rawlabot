#!/usr/bin/env python3

import csv
import random
import matplotlib.pyplot as plt


CSV_LOG = 'log.csv'
PLOTS = 6
FIG_NAME = 'plot.png'


def parse_csv(filename):
    'Parses the CSV file and creates a dict of TX-RX antenna to list of values.'
    data = {}

    with open(filename, newline='') as csv_file:
        for row in csv.reader(csv_file):
            key, val = row[0], float(row[1])
            data.setdefault(key, []).append(val)

    return data


if __name__ == '__main__':
    data = parse_csv(CSV_LOG)
    keys = random.sample(data.keys(), PLOTS)

    for key in keys:
        plt.plot(data[key], alpha=0.5, label=key)
    plt.legend()
    plt.savefig(FIG_NAME)
