# 5G-Vehicle

Project 3 of Performance Evaluation of Computer Systems and Networks (A.Y. 2026/2027).

A car drives back and forth on a 2 km highway covered by 5 base stations. It sends
application packets (one every T seconds, T random) to an Application Server and,
every H seconds, a handover packet with its position to a Mobility Server, which
picks the closest base station. Both packet types share one FIFO queue and the
transmission time is S = alpha * d^2, where d is the distance to the serving base station.
We study the end-to-end delay of the application packets for different V, T and H.

```mermaid
flowchart TB
    V["Vehicle<br/>FIFO queue, moves at speed V"]
    subgraph HW["Highway 0 - 2000 m, one BS every 500 m"]
        direction LR
        BS0["bs 0"] --- BS1["bs 1"] --- BS2["bs 2"] --- BS3["bs 3"] --- BS4["bs 4"]
    end
    AS["Application Server<br/>end-to-end delay"]
    MS["Mobility Server<br/>picks the closest BS"]

    V -- "radio to the serving BS<br/>S = alpha * d^2" --> HW
    HW -- "app packets" --> AS
    HW -- "handover packets" --> MS
    V <-. "new serving BS" .- MS
```

What happens to the two kinds of packets:

```mermaid
sequenceDiagram
    participant V as Vehicle
    participant BS as Serving BS
    participant AS as App Server
    participant MS as Mobility Server

    Note over V: every T seconds (random)
    V->>BS: app packet (waits in the FIFO, then takes S)
    BS->>AS: forward (1 ms)
    Note over AS: delay = arrival time - generation time

    Note over V: every H seconds
    V->>BS: handover packet with the current position
    BS->>MS: forward (1 ms)
    MS-->>V: new serving BS = closest to the reported position
```

## Folders

- `VehicleSim/` - OMNeT++ 6 model (NED, C++, `omnetpp.ini`) and `run_all.sh`
- `Analysis/` - `analyze_results.ipynb` (statistics and Plotly charts), `make_highway_gif.py`,
  exported CSV data in `data/` and the charts in `figures/`

## How to run

1. In the OMNeT++ shell: `cd VehicleSim` and `./run_all.sh`
   (builds the model, runs all the configurations and exports the CSV files to `Analysis/data`)
2. Open `Analysis/analyze_results.ipynb` and run all the cells
3. Optional: `python make_highway_gif.py` in `Analysis/` for the highway animation

Python packages: `plotly kaleido pandas numpy scipy imageio ipykernel`

## Main results

- The delay as a function of H is U-shaped: best H between about 0.1 and 1 s for all speeds.
- Too small H: handover packets load the transmitter. Too large H: the car keeps using a
  far base station and the queue can collapse (above V*H of about 150-200 m).
- With the same mean and variance, lognormal T gives slightly lower delay than exponential T;
  a higher coefficient of variation of T makes things much worse.

Before submitting the ZIP, remove `out/`, `results/`, the generated `Makefile` and the `*_m.cc` / `*_m.h` files.
