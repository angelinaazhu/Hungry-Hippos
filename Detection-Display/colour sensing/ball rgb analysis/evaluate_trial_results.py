import numpy as np
import pandas as pd
from itertools import combinations
from numpy.linalg import norm

# -----------------------------
# Load dataset
# -----------------------------
df = pd.read_csv("jan4_round1_rgb_cluster_formatted_header.csv")

# -----------------------------
# Compute centroids (same as Arduino calibration)
# -----------------------------
centroids = df.groupby("label")[["R", "G", "B"]].mean()

# -----------------------------
# Compute all pairwise centroid distances
# -----------------------------
centroid_distances = []

for a, b in combinations(centroids.index, 2):
    dist = norm(centroids.loc[a] - centroids.loc[b])
    centroid_distances.append({
        "Centroid A": a,
        "Centroid B": b,
        "Distance": dist
    })

centroid_distances_df = pd.DataFrame(centroid_distances)
centroid_distances_df = centroid_distances_df.sort_values("Distance").reset_index(drop=True)

# -----------------------------
# Measure cluster tightness
# -----------------------------
def cluster_stats(df, centroids):
    stats = []
    for label, group in df.groupby("label"):
        center = centroids.loc[label].values
        diffs = group[["R","G","B"]].values - center
        dists = np.linalg.norm(diffs, axis=1)
        stats.append({
            "label": label,
            "mean_dist": dists.mean(),
            "max_dist": dists.max()
        })
    return pd.DataFrame(stats)

stats = cluster_stats(df, centroids)

# -----------------------------
# Measure separation between colours
# -----------------------------
pairs = []
for a, b in combinations(centroids.index, 2):
    d = norm(centroids.loc[a] - centroids.loc[b])
    ra = stats[stats.label == a]["max_dist"].values[0]
    rb = stats[stats.label == b]["max_dist"].values[0]
    pairs.append({
        "A": a,
        "B": b,
        "centroid_dist": d,
        "min_safe_dist": ra + rb,
        "safe": d > (ra + rb)
    })

pairs_df = pd.DataFrame(pairs)

# -----------------------------
# Arduino-style classification
# -----------------------------
def classify(sample, centroids):
    sample = np.array(sample, dtype=float).reshape(1, -1)
    diffs = centroids.values - sample
    dists = np.linalg.norm(diffs, axis=1)
    return centroids.index[np.argmin(dists)]

df["pred"] = df.apply(
    lambda r: classify(r[["R","G","B"]].values, centroids),
    axis=1
)

# -----------------------------
# Accuracy
# -----------------------------
accuracy = (df["label"] == df["pred"]).mean()
accuracy_df = pd.DataFrame({"Overall Accuracy": [accuracy]})

# -----------------------------
# Confusion matrix
# -----------------------------
confusion = pd.crosstab(
    df["label"],
    df["pred"],
    normalize="index"
)

# -----------------------------
# Write to Excel
# -----------------------------
output = "jan4_round1_color_analysis.xlsx"

with pd.ExcelWriter(output) as writer:
    centroids.to_excel(writer, sheet_name="Centroids")
    centroid_distances_df.to_excel(writer, sheet_name="Centroid_Distances", index=False)
    stats.sort_values("max_dist").to_excel(writer, sheet_name="Cluster_Tightness", index=False)
    pairs_df.to_excel(writer, sheet_name="Colour_Separation", index=False)
    accuracy_df.to_excel(writer, sheet_name="Overall_Accuracy", index=False)
    confusion.to_excel(writer, sheet_name="Confusion_Matrix")

print(f"Excel file {output} created successfully.")
