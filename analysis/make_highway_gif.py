"""
make_highway_gif.py  -  animated view of ONE simulation run (config "TimeSeries", H = 5 s, replica 1)

Top panel   : the highway, the 5 base stations and the car.
              The dotted line goes from the car to its serving BS:
              green = it is the closest BS, red = it is NOT (old handover information).
Bottom panel: end-to-end delay of the application packets up to the current time.

Output: figures/10_highway_animation.gif
Usage:  python make_highway_gif.py
"""
import os
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import imageio.v2 as imageio

DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "figures")

# the same values as in omnetpp.ini (config TimeSeries)
L = 2000.0            # highway length [m]
N = 5                 # number of base stations
ISD = L / (N - 1)     # 500 m
SPEED = 30.0          # [m/s]
X0 = 0.0              # initial position [m]
H_SHOWN = 5.0         # which run we animate: H = 5 s ...
REPLICA_SHOWN = 0     # ... replica 0

T_END = 140           # seconds of simulation shown in the GIF (one frame per second)

try:
    import kaleido
    kaleido.start_sync_server(silence_warnings=True)
except Exception:
    pass


# ---- 1. read the two vectors we need from the CSV ----------------------------------
raw = pd.read_csv(os.path.join(DATA, "TimeSeries.csv"), low_memory=False)
H_of_run = raw[(raw["type"] == "itervar") & (raw["attrname"] == "H")].set_index("run")["attrvalue"].astype(float)
rep_of_run = raw[(raw["type"] == "runattr") & (raw["attrname"] == "repetition")].set_index("run")["attrvalue"].astype(int)
run = [r for r in H_of_run.index if H_of_run[r] == H_SHOWN and rep_of_run[r] == REPLICA_SHOWN][0]


def read_vector(name):
    """Times and values of one recorded vector of the chosen run."""
    row = raw[(raw["run"] == run) & (raw["type"] == "vector") & (raw["name"] == name)].iloc[0]
    return np.array(row["vectime"].split(), dtype=float), np.array(row["vecvalue"].split(), dtype=float)


bs_time, bs_value = read_vector("servingBs:vector")           # serving BS, recorded at every change
delay_time, delay_value = read_vector("endToEndDelay:vector")  # delay of every application packet


# ---- 2. helpers --------------------------------------------------------------------
def position(t):
    """Vehicle position: same formula as Vehicle::getPosition() in Vehicle.cc."""
    s = (X0 + SPEED * t) % (2 * L)
    if s <= L:
        return s
    return 2 * L - s


def serving_bs(t):
    """Serving BS at time t = last value recorded before t."""
    last = np.searchsorted(bs_time, t, side="right") - 1
    return int(bs_value[max(last, 0)])


def draw_frame(t):
    x = position(t)
    bs = serving_bs(t)
    closest = min(N - 1, max(0, round(x / ISD)))
    link_colour = "#1baf7a" if bs == closest else "#e34948"     # green = correct BS, red = wrong BS

    fig = make_subplots(rows=2, cols=1, row_heights=[0.4, 0.6], vertical_spacing=0.2,
                        subplot_titles=["Highway (L = 2 km, 5 base stations)", "End-to-end delay of the application packets"])

    # top panel: road, base stations, link, car
    fig.add_trace(go.Scatter(x=[0, L], y=[0, 0], mode="lines", line=dict(color="#b5b4ad", width=10)), row=1, col=1)
    bs_x = [i * ISD for i in range(N)]
    bs_colour = ["#eb6834" if i == bs else "#52514e" for i in range(N)]
    fig.add_trace(go.Scatter(x=bs_x, y=[60] * N, mode="markers+text", text=[f"BS{i}" for i in range(N)],
                             textposition="top center", marker=dict(symbol="triangle-up", size=22, color=bs_colour)),
                  row=1, col=1)
    fig.add_trace(go.Scatter(x=[x, bs_x[bs]], y=[0, 60], mode="lines",
                             line=dict(color=link_colour, width=3, dash="dot")), row=1, col=1)
    fig.add_trace(go.Scatter(x=[x], y=[0], mode="markers", marker=dict(size=22, color="#2a78d6", symbol="square")),
                  row=1, col=1)

    # bottom panel: delays of the packets received until time t
    shown = delay_time <= t
    fig.add_trace(go.Scatter(x=delay_time[shown], y=delay_value[shown] * 1000, mode="lines",
                             line=dict(color="#2a78d6", width=1.5)), row=2, col=1)

    fig.update_xaxes(range=[-100, L + 100], title="position [m]", row=1, col=1)
    fig.update_yaxes(range=[-40, 110], visible=False, row=1, col=1)
    fig.update_xaxes(range=[0, T_END], title="simulation time [s]", row=2, col=1)
    fig.update_yaxes(type="log", dtick=1, range=[0, 4], title="delay [ms] (log scale)", row=2, col=1)
    fig.update_layout(title=f"t = {t:3.0f} s   |   x = {x:4.0f} m   |   serving BS{bs}   (V = 30 m/s, H = 5 s, E[T] = 10 ms)",
                      template="simple_white", showlegend=False, width=1000, height=620,
                      font=dict(family="Segoe UI, Arial", size=14), paper_bgcolor="#fcfcfb", plot_bgcolor="#fcfcfb")
    return fig


# ---- 3. one PNG per second of simulation, glued into a GIF ---------------------------
frames = []
for t in range(1, T_END + 1):
    frames.append(imageio.imread(draw_frame(t).to_image(format="png")))
imageio.mimsave(os.path.join(OUT, "10_highway_animation.gif"), frames, duration=0.12, loop=0)
print("saved 10_highway_animation.gif (", len(frames), "frames )")
