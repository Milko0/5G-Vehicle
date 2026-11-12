"""
analyze_results.py  -  Project 3 (5G connected vehicle)

What it does:
  1. reads the CSV files exported by run_all.sh (folder data/)
  2. computes, for every scenario, the mean over the 10 replications and the 95% confidence interval
  3. draws the charts with Plotly and saves them in figures/ (.html = interactive, .png = report/slides)
  4. makes one animated GIF (the delay curve while the speed grows)

How to run it (from the "analysis" folder):
    pip install plotly kaleido pandas numpy scipy imageio
    python analyze_results.py
"""
import os
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from scipy import stats
import imageio.v2 as imageio

# =====================================================================
# 0. Settings
# =====================================================================
DATA = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "figures")
os.makedirs(OUT, exist_ok=True)

# model constants (the same values as in omnetpp.ini)
ALPHA = 1.28e-7      # S = ALPHA * d^2
ISD = 500.0          # distance between two base stations [m]
OFFSET = 20.0        # distance between the road and the base stations [m]
BACKHAUL = 0.001     # BS -> server delay [s]

SPEEDS = [10, 20, 30, 40]
COLORS = ["#2a78d6", "#eb6834", "#1baf7a", "#eda100", "#e87ba4"]   # one colour per line
SYMBOLS = ["circle", "square", "triangle-up", "diamond", "x"]
GREY = "#8a8983"

TOP = 2000           # [ms] collapsed scenarios are drawn at this height (top of the delay charts)

# start the browser used by kaleido only once (makes saving PNGs much faster)
try:
    import kaleido
    kaleido.start_sync_server(silence_warnings=True)
except Exception:
    pass


# =====================================================================
# 1. Reading the data
# =====================================================================
def load_runs(config):
    """Reads data/<config>.csv and returns a table with ONE ROW PER RUN.

    The CSV (format CSV-R of opp_scavetool) has one row per value:
      type = "itervar" -> a factor of the run (V, H, T, ...)
      type = "runattr" -> run information (we need the repetition number)
      type = "scalar"  -> a result (e.g. name = "endToEndDelay:mean")
    """
    raw = pd.read_csv(os.path.join(DATA, config + ".csv"), low_memory=False)

    # results: one column per scalar name
    scalars = raw[raw["type"] == "scalar"].copy()
    scalars["value"] = scalars["value"].astype(float)
    runs = scalars.pivot_table(index="run", columns="name", values="value")

    # factors: one column per iteration variable (V, H, T, cv, Tv)
    itervars = raw[raw["type"] == "itervar"]
    for name in itervars["attrname"].unique():
        rows = itervars[itervars["attrname"] == name].set_index("run")
        runs[name] = rows["attrvalue"].astype(float)

    # a run is "collapsed" when the transmitter is (almost) never idle: the queue grows forever
    runs["collapsed"] = runs["utilization:timeavg"] > 0.98
    return runs.reset_index()


# =====================================================================
# 2. Statistics
# =====================================================================
def confidence_interval(values):
    """Mean and half-width of the 95% confidence interval (Student t, n independent replications)."""
    values = np.array(values, dtype=float)
    n = len(values)
    mean = values.mean()
    half_width = stats.t.ppf(0.975, n - 1) * values.std(ddof=1) / np.sqrt(n)
    return mean, half_width


def summarize(runs, group_by, metrics=("endToEndDelay:mean",)):
    """One row per scenario: mean and CI of each metric, plus how many replications collapsed."""
    rows = []
    for key, group in runs.groupby(group_by):
        if not isinstance(key, tuple):
            key = (key,)
        row = dict(zip(group_by, key))
        row["n_collapsed"] = int(group["collapsed"].sum())
        for m in metrics:
            row[m], row[m + "_ci"] = confidence_interval(group[m])
        rows.append(row)
    return pd.DataFrame(rows)


# =====================================================================
# 3. Small plotting helpers
# =====================================================================
def style(fig, title, height=600):
    """Same look for every chart."""
    fig.update_layout(title=title, template="simple_white", height=height,
                      font=dict(family="Segoe UI, Arial", size=14),
                      paper_bgcolor="#fcfcfb", plot_bgcolor="#fcfcfb",
                      legend=dict(orientation="h", y=-0.2, x=0.5, xanchor="center"))
    fig.update_xaxes(showgrid=True, gridcolor="#e8e7e3")
    fig.update_yaxes(showgrid=True, gridcolor="#e8e7e3")


def log_x(fig, title, row=None, col=None):
    """Logarithmic x axis, labels only at 0.01, 0.1, 1, 10 ..."""
    fig.update_xaxes(type="log", dtick=1, title=title, row=row, col=col)


def log_delay_axis(fig, row=None, col=None):
    """Logarithmic delay axis from 1 ms to ~3 s, labels only at 1, 10, 100, 1000 ms."""
    fig.update_yaxes(type="log", dtick=1, range=[0, np.log10(TOP * 1.5)],
                     title="mean E2E delay [ms] (log scale)", row=row, col=col)


def add_delay_line(fig, data, x, label, color, symbol="circle", dash="solid", row=None, col=None,
                   show_legend=True):
    """Draws the mean delay (in ms) vs. x.
       - stable scenarios: line + markers + error bars (95% CI)
       - unstable scenarios (a replication collapsed, or mean delay above the top of the chart):
         an empty marker on the top edge of the chart"""
    data = data.sort_values(x)
    mean_ms = data["endToEndDelay:mean"] * 1000
    unstable = (data["n_collapsed"] > 0) | (mean_ms > TOP)
    stable = data[~unstable]
    collapsed = data[unstable]

    ci_ms = stable["endToEndDelay:mean_ci"] * 1000
    mean_ms = stable["endToEndDelay:mean"] * 1000
    fig.add_trace(go.Scatter(
        x=stable[x], y=mean_ms,
        # on a log axis an error bar cannot go below 0: the lower part is limited to 90% of the mean
        error_y=dict(array=ci_ms, arrayminus=np.minimum(ci_ms, 0.9 * mean_ms), width=4),
        mode="lines+markers", name=label, legendgroup=label, showlegend=show_legend,
        line=dict(color=color, width=2.2, dash=dash), marker=dict(color=color, size=8, symbol=symbol)),
        row=row, col=col)

    if len(collapsed) > 0:
        fig.add_trace(go.Scatter(
            x=collapsed[x], y=[TOP] * len(collapsed), mode="markers", name=label + " (collapsed)",
            legendgroup=label, showlegend=False,
            marker=dict(color="white", size=11, symbol=symbol, line=dict(width=2, color=color))),
            row=row, col=col)


def save(fig, name, width=1100, height=600):
    fig.write_html(os.path.join(OUT, name + ".html"), include_plotlyjs="cdn")
    fig.write_image(os.path.join(OUT, name + ".png"), width=width, height=height, scale=2)
    print("saved", name)


# =====================================================================
# 4. The charts
# =====================================================================
def fig01_validation():
    """Verification against queueing theory (two cases where we know the exact answer)."""
    # (a) vehicle standing still at d = 125 m  ->  constant S  ->  M/D/1 queue
    md1 = summarize(load_runs("Validation_MD1"), ["Tv"]).sort_values("Tv", ascending=False)
    S = ALPHA * 125 ** 2                                      # 2 ms
    md1["rho"] = S / (md1["Tv"] / 1000)
    md1["theory"] = S + md1["rho"] * S / (2 * (1 - md1["rho"]))   # Pollaczek-Khinchine for M/D/1

    # (b) ideal association, no handover packets: at every position the queue is an M/D/1 with
    #     S = ALPHA * d^2; the vehicle position u is uniform in [0, ISD/2] -> average the formula over u
    ideal = summarize(load_runs("IdealHandover"), ["T", "V"])
    u = np.linspace(0, ISD / 2, 10001)
    S_u = ALPHA * (u ** 2 + OFFSET ** 2)
    theory = {}
    for T in ideal["T"].unique():
        rho_u = S_u / (T / 1000)
        theory[T] = np.mean(S_u + rho_u * S_u / (2 * (1 - rho_u))) + BACKHAUL
    ideal["theory"] = ideal["T"].map(theory)

    fig = make_subplots(rows=1, cols=2, subplot_titles=[
        "(a) vehicle standing still: M/D/1", "(b) ideal association: M/D/1 averaged over the position"])
    fig.add_trace(go.Scatter(x=md1["rho"], y=md1["theory"] * 1000, mode="lines", name="theory",
                             line=dict(color=GREY, dash="dash")), row=1, col=1)
    fig.add_trace(go.Scatter(x=md1["rho"], y=md1["endToEndDelay:mean"] * 1000, mode="markers",
                             name="OMNeT++ (mean, 95% CI)", marker=dict(size=11, color=COLORS[0]),
                             error_y=dict(array=md1["endToEndDelay:mean_ci"] * 1000)), row=1, col=1)
    th = ideal.groupby("T")["theory"].first()
    fig.add_trace(go.Scatter(x=th.index, y=th.values * 1000, mode="lines", showlegend=False,
                             line=dict(color=GREY, dash="dash")), row=1, col=2)
    for i, v in enumerate(SPEEDS):
        d = ideal[ideal["V"] == v]
        fig.add_trace(go.Scatter(x=d["T"], y=d["endToEndDelay:mean"] * 1000, mode="markers",
                                 name=f"V = {v} m/s", marker=dict(size=10, color=COLORS[i + 1], symbol=SYMBOLS[i]),
                                 error_y=dict(array=d["endToEndDelay:mean_ci"] * 1000)), row=1, col=2)
    fig.update_xaxes(title="utilisation ρ = S / E[T]", row=1, col=1)
    fig.update_xaxes(title="E[T] [ms]", row=1, col=2)
    fig.update_yaxes(title="mean E2E delay [ms]", rangemode="tozero")
    style(fig, "Verification: the simulator reproduces queueing theory", height=520)
    save(fig, "01_validation", height=520)

    md1["error_%"] = 100 * (md1["endToEndDelay:mean"] / md1["theory"] - 1)
    ideal["error_%"] = 100 * (ideal["endToEndDelay:mean"] / ideal["theory"] - 1)
    print(md1[["Tv", "rho", "endToEndDelay:mean", "theory", "error_%"]].round(5).to_string(index=False))
    print(ideal[["T", "V", "endToEndDelay:mean", "theory", "error_%"]].round(5).to_string(index=False))
    return ideal


def fig02_delay_vs_H(summary, ideal):
    """The main result: the delay is U-shaped in H."""
    data = summary[(summary["dist"] == "Exponential") & (summary["T"] == 10)]
    lower_bound = ideal[ideal["T"] == 10]["endToEndDelay:mean"].mean() * 1000

    fig = go.Figure()
    # dashed horizontal line = lower bound (drawn first, so the curves stay on top of it)
    fig.add_trace(go.Scatter(x=[0.007, 30], y=[lower_bound, lower_bound], mode="lines",
                             name=f"ideal association (lower bound) = {lower_bound:.1f} ms",
                             line=dict(color="black", dash="dash", width=1.5)))
    for i, v in enumerate(SPEEDS):
        add_delay_line(fig, data[data["V"] == v], "H", f"V = {v} m/s", COLORS[i], SYMBOLS[i])
    fig.add_annotation(text="empty markers on top = queue collapsed (unstable)", x=1, y=1.02,
                       xref="paper", yref="paper", showarrow=False, xanchor="right", font=dict(color=GREY))
    log_x(fig, "handover period H [s] (log scale)")
    log_delay_axis(fig)
    style(fig, "End-to-end delay vs. handover period (exponential T, E[T] = 10 ms)", height=620)
    save(fig, "02_delay_vs_H", height=620)


def fig03_mechanism(summary):
    """WHY the curve is U-shaped: too many handover packets on the left, wrong BS on the right."""
    data = summary[(summary["dist"] == "Exponential") & (summary["T"] == 10) & (summary["n_collapsed"] == 0)]
    panels = [("utilization:timeavg", 1, "transmitter utilisation"),
              ("misassociated:mean", 100, "app packets sent via a non-closest BS [%]"),
              ("txDistance:mean", 1, "mean distance to the serving BS [m]")]

    fig = make_subplots(rows=1, cols=3, subplot_titles=[p[2] for p in panels], horizontal_spacing=0.08)
    for i, v in enumerate(SPEEDS):
        d = data[data["V"] == v].sort_values("H")
        for c, (metric, factor, _) in enumerate(panels):
            fig.add_trace(go.Scatter(x=d["H"], y=d[metric] * factor, mode="lines+markers", name=f"V = {v} m/s",
                                     legendgroup=str(v), showlegend=(c == 0), line=dict(color=COLORS[i]),
                                     marker=dict(symbol=SYMBOLS[i], size=7)), row=1, col=c + 1)
    log_x(fig, "H [s] (log scale)")
    fig.update_yaxes(rangemode="tozero")
    style(fig, "Why the curve is U-shaped (stable scenarios only)", height=520)
    save(fig, "03_mechanism", width=1400, height=520)


def fig04_delay_vs_VH(summary):
    """V·H (metres driven between two handover packets) explains the right side of the U."""
    data = summary[(summary["dist"] == "Exponential") & (summary["T"] == 10)].copy()
    data["VH"] = data["V"] * data["H"]

    fig = make_subplots(rows=1, cols=2, horizontal_spacing=0.1, subplot_titles=[
        "misassociation depends only on V·H", "delay: V·H decides if the queue collapses"])
    for i, v in enumerate(SPEEDS):
        d = data[data["V"] == v].sort_values("VH")
        stable = d[d["n_collapsed"] == 0]
        fig.add_trace(go.Scatter(x=stable["VH"], y=stable["misassociated:mean"] * 100, mode="lines+markers",
                                 name=f"V = {v} m/s", legendgroup=str(v), line=dict(color=COLORS[i]),
                                 marker=dict(symbol=SYMBOLS[i], size=7)), row=1, col=1)
        add_delay_line(fig, d, "VH", f"V = {v} m/s", COLORS[i], SYMBOLS[i], row=1, col=2, show_legend=False)
    log_x(fig, "V·H = metres driven between two handover packets [m]")
    fig.update_yaxes(title="packets via a non-closest BS [%]", rangemode="tozero", row=1, col=1)
    log_delay_axis(fig, row=1, col=2)
    # dotted vertical line at V·H = ISD/2 = 250 m (half the distance between two BSs)
    fig.add_trace(go.Scatter(x=[ISD / 2, ISD / 2], y=[0, 17], mode="lines", showlegend=False,
                             line=dict(color=GREY, dash="dot")), row=1, col=1)
    fig.add_trace(go.Scatter(x=[ISD / 2, ISD / 2], y=[1, TOP * 1.4], mode="lines", showlegend=False,
                             line=dict(color=GREY, dash="dot")), row=1, col=2)
    style(fig, "The distance driven between two updates (V·H) is what matters  (dotted line: ISD/2 = 250 m)",
          height=560)
    save(fig, "04_delay_vs_VH", width=1300, height=560)


def fig05_heatmap(summary):
    """Summary table as a heatmap: mean delay for every (V, H), exponential vs lognormal."""
    fig = make_subplots(rows=1, cols=2, horizontal_spacing=0.08,
                        subplot_titles=["Exponential T", "Lognormal T (same mean and variance)"])
    for c, dist in enumerate(["Exponential", "Lognormal"]):
        d = summary[(summary["dist"] == dist) & (summary["T"] == 10)]
        H_values = sorted(d["H"].unique())
        colour, text = [], []
        for v in SPEEDS:
            colour_row, text_row = [], []
            for h in H_values:
                cell = d[(d["V"] == v) & (d["H"] == h)].iloc[0]
                ms = cell["endToEndDelay:mean"] * 1000
                if cell["n_collapsed"] > 0 or ms > TOP:
                    colour_row.append(np.log10(TOP))
                    text_row.append("collapsed")
                else:
                    colour_row.append(np.log10(ms))
                    text_row.append(f"{ms:.3g}")
            colour.append(colour_row); text.append(text_row)
        fig.add_trace(go.Heatmap(z=colour, x=[f"{h:g}" for h in H_values], y=[f"{v}" for v in SPEEDS],
                                 text=text, texttemplate="%{text}", textfont=dict(size=10),
                                 colorscale="Blues", zmin=0.5, zmax=np.log10(TOP), showscale=(c == 1),
                                 colorbar=dict(title="delay [ms]", tickvals=[1, 2, 3], ticktext=["10", "100", "1000"])),
                      row=1, col=c + 1)
    fig.update_xaxes(title="H [s]", type="category")
    fig.update_yaxes(title="V [m/s]", type="category")
    style(fig, "Mean E2E delay [ms] for every (V, H), E[T] = 10 ms", height=520)
    save(fig, "05_heatmap_V_H", width=1500, height=520)


def fig06_distribution():
    """Same mean of T, different variability: exponential vs lognormal with CoV 0.5 ... 3 (V = 30 m/s)."""
    exp = load_runs("Exponential")
    exp = exp[(exp["V"] == 30) & (exp["T"] == 10)].copy()
    exp["label"] = "exponential (CoV = 1)"
    logn = load_runs("Lognormal")
    logn = logn[(logn["V"] == 30) & (logn["T"] == 10)].copy()
    logn["label"] = "lognormal, CoV = 1"
    cv = load_runs("LognormalCV")
    cv["label"] = "lognormal, CoV = " + cv["cv"].map("{:g}".format)
    data = summarize(pd.concat([exp, logn, cv]), ["label", "H"])

    lines = [("lognormal, CoV = 0.5", COLORS[2], "solid"), ("exponential (CoV = 1)", COLORS[0], "solid"),
             ("lognormal, CoV = 1", COLORS[0], "dot"), ("lognormal, CoV = 2", COLORS[3], "solid"),
             ("lognormal, CoV = 3", COLORS[1], "solid")]
    fig = go.Figure()
    for i, (label, color, dash) in enumerate(lines):
        add_delay_line(fig, data[data["label"] == label], "H", label, color, SYMBOLS[i], dash=dash)
    log_x(fig, "handover period H [s] (log scale)")
    log_delay_axis(fig)
    style(fig, "Same mean E[T] = 10 ms, different variability of T (V = 30 m/s)", height=620)
    save(fig, "06_distribution", height=620)
    return data


def fig07_load(summary):
    """More traffic: the best H does not move, but the range of safe H shrinks."""
    data = summary[(summary["dist"] == "Exponential") & (summary["V"] == 30)]
    fig = go.Figure()
    for i, T in enumerate([10, 15, 20]):
        add_delay_line(fig, data[data["T"] == T], "H", f"E[T] = {T} ms", COLORS[i], SYMBOLS[i])
    log_x(fig, "handover period H [s] (log scale)")
    log_delay_axis(fig)
    style(fig, "Effect of the traffic load (exponential T, V = 30 m/s)", height=600)
    save(fig, "07_load", height=600)


def fig08_timeseries():
    """Delay over time for a stable (H = 1 s), a borderline (5 s) and a collapsing (20 s) case."""
    raw = pd.read_csv(os.path.join(DATA, "TimeSeries.csv"), low_memory=False)
    H_of_run = raw[(raw["type"] == "itervar") & (raw["attrname"] == "H")].set_index("run")["attrvalue"].astype(float)
    vectors = raw[(raw["type"] == "vector") & (raw["name"] == "endToEndDelay:vector")]

    H_list = [1.0, 5.0, 20.0]
    fig = make_subplots(rows=3, cols=1, shared_xaxes=True, vertical_spacing=0.07,
                        subplot_titles=[f"V = 30 m/s, H = {h:g} s  (V·H = {30 * h:g} m)" for h in H_list])
    for r, h in enumerate(H_list):
        runs_with_h = [run for run in vectors["run"] if H_of_run[run] == h]
        for k, run in enumerate(runs_with_h):
            row = vectors[vectors["run"] == run].iloc[0]
            t = np.array(row["vectime"].split(), dtype=float)
            delay = np.array(row["vecvalue"].split(), dtype=float)
            # average over windows of 5 s (otherwise the plot has ~60 000 points)
            window = (t // 5).astype(int)
            mean_per_window = pd.Series(delay).groupby(window).mean()
            fig.add_trace(go.Scatter(x=mean_per_window.index * 5, y=mean_per_window.values * 1000, mode="lines",
                                     name=f"replica {k + 1}", legendgroup=str(k), showlegend=(r == 0),
                                     line=dict(color=COLORS[r], dash=["solid", "dash", "dot"][k], width=1.6)),
                          row=r + 1, col=1)
        fig.update_yaxes(type="log", dtick=1, range=[0, 5.5], title="delay [ms]", row=r + 1, col=1)
    fig.update_xaxes(title="simulation time [s]", row=3, col=1)
    style(fig, "End-to-end delay over time (mean per 5 s), 3 independent replicas per H", height=800)
    save(fig, "08_timeseries", height=800)


def fig09_animated_gif(summary):
    """Animated GIF: one frame per speed, the U-curve moves when the vehicle goes faster."""
    data = summary[(summary["dist"] == "Exponential") & (summary["T"] == 10)]
    frames = []
    for i, v in enumerate(SPEEDS):
        fig = go.Figure()
        # previous speeds in light grey, current speed in colour
        for j in range(i):
            add_delay_line(fig, data[data["V"] == SPEEDS[j]], "H", f"V = {SPEEDS[j]} m/s", "#d6d5cf", SYMBOLS[j])
        add_delay_line(fig, data[data["V"] == v], "H", f"V = {v} m/s", COLORS[i], SYMBOLS[i])
        log_x(fig, "handover period H [s] (log scale)")
        log_delay_axis(fig)
        style(fig, f"How the U-curve changes with the speed  —  V = {v} m/s  (E[T] = 10 ms)", height=560)
        fig.update_xaxes(range=[-2.2, 1.5])
        frames.append(imageio.imread(fig.to_image(format="png", width=1000, height=560)))
    imageio.mimsave(os.path.join(OUT, "09_animated_speed.gif"), frames, duration=1.3, loop=0)
    print("saved 09_animated_speed.gif")


# =====================================================================
# 5. Main
# =====================================================================
if __name__ == "__main__":
    ideal = fig01_validation()

    # all runs of the two main scenarios in one table, with a column "dist"
    exp = load_runs("Exponential")
    exp["dist"] = "Exponential"
    logn = load_runs("Lognormal")
    logn["dist"] = "Lognormal"
    metrics = ["endToEndDelay:mean", "utilization:timeavg", "misassociated:mean", "txDistance:mean"]
    summary = summarize(pd.concat([exp, logn]), ["dist", "T", "V", "H"], metrics)
    summary.to_csv(os.path.join(OUT, "summary_table.csv"), index=False)

    fig02_delay_vs_H(summary, ideal)
    fig03_mechanism(summary)
    fig04_delay_vs_VH(summary)
    fig05_heatmap(summary)
    fig06_distribution().to_csv(os.path.join(OUT, "summary_distribution.csv"), index=False)
    fig07_load(summary)
    fig08_timeseries()
    fig09_animated_gif(summary)
    print("\nAll figures are in", OUT)
