import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import hashlib
import colorsys

fname = "jan4_round1_rgb_cluster_formatted_header.csv"

def record_samples():
    """
    Interactive prompt to record labeled RGB samples.
    Returns a DataFrame with columns: label, R, G, B
    """
    samples = []
    print("Enter samples for each color. Type 'done' as label to finish.")
    while True:
        label = input("Enter color label (or 'done' to finish): ").strip()
        if label.lower() == 'done':
            break
        n = int(input(f"How many samples for '{label}'? "))
        for i in range(n):
            rgb_str = input(f"Sample {i+1}/{n} for '{label}' (format: R,G,B): ")
            try:
                r, g, b = [float(x) for x in rgb_str.strip().split(',')]
                samples.append({'label': label, 'R': r, 'G': g, 'B': b})
            except Exception:
                print("Invalid input, try again.")
    return pd.DataFrame(samples)

def label_to_color(label):
    """
    Map label names to fixed colors.
    Known color names are literal; others get stable auto colors.
    """
    named_colors = {
        'LIGHTBLUE': (0.4, 0.7, 1.0),
        'DARKBLUE':  (0.0, 0.0, 0.6),
        'GREEN':      (0.0, 1.0, 0.0),
        'YELLOW':     (1.0, 1.0, 0.0),
        'ORANGE':     (1.0, 0.5, 0.0),
        'RED':        (1.0, 0.0, 0.0),
        'PINK':       (1.0, 0.4, 0.7),
        'PURPLE':     (0.6, 0.0, 0.8),
        'WHITE':      (0.7, 0.7, 0.7),
        'NOBALL': (0.0, 0.0, 0.0),
    }

    key = label.strip().upper()
    if key in named_colors:
        return named_colors[key]

    # fallback: stable distinct color
    h = int(hashlib.md5(label.encode()).hexdigest(), 16)
    hue = (h % 360) / 360.0
    return colorsys.hsv_to_rgb(hue, 0.85, 0.9)

def plot_clusters(df):
    """
    Plots labeled RGB samples in 3D with legend colors matching labels.
    """
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    for label in df['label'].unique():
        subset = df[df['label'] == label]
        color = label_to_color(label)

        # ONE scatter per label -> legend color matches points
        ax.scatter(
            subset['R'], subset['G'], subset['B'],
            color=color,
            label=label,
            s=40
        )

    ax.set_xlabel('Red')
    ax.set_ylabel('Green')
    ax.set_zlabel('Blue')
    ax.set_title('RGB Color Clusters')
    ax.legend()
    plt.show()

def main():
    #print("Choose mode:")
    #print("1. Record new samples")
    #print("2. Load samples from CSV")
    #mode = input("Enter 1 or 2: ").strip()
    '''if mode == '1':
        df = record_samples()
        save = input("Save to CSV? (y/n): ").strip().lower()
        if save == 'y':
            fname = input("Enter filename (e.g. samples.csv): ").strip()
            df.to_csv(fname, index=False)
            print(f"Saved to {fname}")
    else:'''
    #fname = input("Enter CSV filename: ").strip()
    df = pd.read_csv(fname)

    plot_clusters(df)

if __name__ == '__main__':
    main()
