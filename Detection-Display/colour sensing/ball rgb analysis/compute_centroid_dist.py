# using centroid.csv contents:
'''
label,R,G,B
DARK BLUE,4.933333333,28.23333333,64.83333333
GREEN,41.30555556,143.1111111,50.88888889
LIGHT BLUE,7.8,79.66666667,124.1
NO BALL,0,0.366666667,0.066666667
ORANGE,168.6333333,89.16666667,40.93333333
PINK,124.3,112.9,120.6666667
PURPLE,26.96666667,31.56666667,61.63333333
RED,75.53333333,17.53333333,15.96666667
WHITE,105.5333333,169.7,190.6
YELLOW,142.9666667,168.9,58.8
'''
# compute and print all distances between centroids in order of smallest distance to largest distance

import pandas as pd
from itertools import combinations
from numpy.linalg import norm
# Load centroids
centroids = pd.read_csv("centroid.csv", index_col="label")
# Compute distances between centroids
pairs = []
for a, b in combinations(centroids.index, 2):
    d = norm(centroids.loc[a] - centroids.loc[b])
    pairs.append({
        "A": a,
        "B": b,
        "centroid_dist": d
    })
pairs_df = pd.DataFrame(pairs)
# Sort by distance
pairs_df = pairs_df.sort_values(by="centroid_dist")
print(pairs_df)
