#!/usr/bin/python3
#%%
import sys

# if (sys.argc < 2):
#   print(sys.argv[0], "Filename")
#   sys.exit(0)

# fname = sys.argv[1]

#fname = "sched_baseline.txt"
#fname = "sched_topoAware.txt"
fname = "sched_greedy.txt"
#fname = "sched_preserve.txt"

X = "X"
NV1 = "NV1"
NV2 = "NV2"
NV3 = "NV3"
SOC = "SOC"

topo_m = [
    [X,      NV1,     NV1,     NV2,     NV2,     SOC,     SOC,     SOC],
    [NV1,      X,      NV2,     NV1,     SOC,     NV2,     SOC,     SOC],
    [NV1,     NV2,      X,      NV2,     SOC,     SOC,     NV1,     SOC],
    [NV2,     NV1,     NV2,      X,      SOC,     SOC,     SOC,     NV1],
    [NV2,     SOC,     SOC,     SOC,      X,      NV1,     NV1,     NV2],
    [SOC,     NV2,     SOC,     SOC,     NV1,      X,      NV2,     NV1],
    [SOC,     SOC,     NV1,     SOC,     NV1,     NV2,      X,      NV2],
    [SOC,     SOC,     SOC,     NV1,     NV2,     NV1,     NV2,      X]
]

def getPredBW(x1, x2, x3):
  linear = 16.39670455 * x1 + 4.536821878 * x2 + 1.55623404 * x3
  inverseLinear = (-20.69484581) * 1 / (x1 + 1) + (-9.467461958) * 1 / (x2 + 1) + 7.615106783 * 1 / (x3 + 1)
  pairwise = (-7.973335727) * x1 * x2 + 12.73323772 * x2 * x3 + (-4.195655221) * x1 * x3
  inversePairwise = (-8.413419363) * 1 / (x1 * x2 + 1) + 62.85125807 * 1 / (x2 * x3 + 1) + 27.41832588 * 1 / (x1 * x3 + 1)
  triplet = (-5.114108079) * x1 * x2 * x3
  inverseTriplet = (-46.97390071) * 1 / (x1 * x2 * x3 + 1)

  return (linear + inverseLinear + pairwise + inversePairwise + triplet + inverseTriplet)

with open(fname) as f:
  data = f.readlines()
  for line in data:
    flines = line.split(",")
    nodes = flines[:-1]
    nodes = [float(i)-1.0 for i in nodes]
    # print(nodes)

    x1 = 0.0
    x2 = 0.0
    x3 = 0.0
    if(len(nodes) > 1):
      i = 0
      while (i < len(nodes)):
        link = topo_m[i][i + 1]
        # print(link)
        if (link == NV2):
          x1 += 1.0
        elif (link == NV1):
          x2 += 1.0
        else:
          x3 += 1.0
        i += 1

    # print(x1, x2, x3)
    if (len(nodes) > 1):
      print(str(x3/(x1+x2+x3)))
    else:
      print(0)
    #print(getPredBW(x1, x2, x3))
#%%
