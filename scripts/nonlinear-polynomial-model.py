#%%
import numpy as np

f = open("netlist.txt", "r")

netlist = []
bwlist = []

data = f.readlines()
for line in data:
    x1, x2, x3, b = line.split("\t")
    print(x1,x2,x3,b)
    # netlist.append([float(x1), float(x2), float(x3), float(x1) * float(x2), float(x2) * float(x3), float(x1) * float(x3)])
    netlist.append([float(x1), float(x2), float(x3),
                    1 / float(int(x1) + 1), 1 / float(int(x2) + 1), 1 / float(int(x3) + 1),
                    float(x1) * float(x2), float(x2) * float(x3), float(x1) * float(x3),
                    1 / (float(x1) * float(x2) + 1), 1 / (float(x2) * float(x3) + 1), 1 / (float(x1) * float(x3) + 1),
                    float(x1) * float(x2) * float(x3),
                    1 / (float(x1) * float(x2) * float(x3) + 1)
                    ])
    bwlist.append(float(b))

A = np.array(netlist)
b = np.array(bwlist)

# A = np.random.rand(13, 6)
# b = np.random.rand(13)

x, res, rank, sigma = np.linalg.lstsq(A, b)
fit = np.linalg.norm(A@x-b)**2
relativeFit = fit / np.linalg.norm(b) ** 2
rmse = np.sqrt(fit) / b.size
mae = np.linalg.norm(A @ x - b, 1) / b.size
print(x)
print("Relative ERROR : %s" % relativeFit)
print("RMSE : %s" % rmse)
print("Mean Absolute Error : %s"%mae)

print("\nPrinting weights\n")
for i in x:
    print(i)

print("\nNormalized weights\n")
tot_weights = 0
for i in x:
    tot_weights += abs(i)
for i in x:
    print(abs(i)/tot_weights)

pred = [i for i in A @ x]
print("\nPredicted Values\n")
for i in pred:
    print(i)

print("\nActual Values\n")
for i in b:
    print(i)

# %%
