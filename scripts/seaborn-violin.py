#!/usr/bin/python3

import seaborn as sns
import pandas as pd

dataframes = []
df = pd.read_csv(
    '/home/stark/workspace/mgap-scheduler/scripts/jobs-0baselineV1Log.csv', header=0, delim_whitespace=True)
df['scheduler'] = 'baselineV1'
df['bwSensitivity'] = 0
dataframes.append(df)

df = pd.read_csv('/home/stark/workspace/mgap-scheduler/scripts/jobs-0baselineV1Log.csv',
                 header=0, delim_whitespace=True)
df['scheduler'] = 'baselineV2'
df['bwSensitivity'] = 0
dataframes.append(df)

df = pd.read_csv('/home/stark/workspace/mgap-scheduler/scripts/greedy-split0.csv',
                 header=0, delim_whitespace=True)
df['scheduler'] = 'greedy'
df['bwSensitivity'] = 0
dataframes.append(df)
df = pd.read_csv('/home/stark/workspace/mgap-scheduler/scripts/greedy-split1.csv',
                 header=0, delim_whitespace=True)
df['scheduler'] = 'greedy'
df['bwSensitivity'] = 1
dataframes.append(df)
df = pd.read_csv('/home/stark/workspace/mgap-scheduler/scripts/bw-split0.csv',
                 header=0, delim_whitespace=True)
df['scheduler'] = 'bw'
df['bwSensitivity'] = 0
dataframes.append(df)
df = pd.read_csv('/home/stark/workspace/mgap-scheduler/scripts/bw-split1.csv',
                 header=0, delim_whitespace=True)
df['scheduler'] = 'bw'
df['bwSensitivity'] = 1
dataframes.append(df)
    

all = pd.concat(dataframes)


sns.set_theme(style="whitegrid")


#tips = pd.read_csv(
#    "/home/stark/workspace/mgap-scheduler/scripts/jobs-0baselineV1Log.csv")
# ax = sns.violinplot(x="scheduler", y="fragScore", hue="bwSensitivity",
#                      data=all, palette="Set2", split=True,
#                      scale="count", inner="quartile")
# fig = ax.get_figure()
# fig.savefig("seaborn-fs.pdf")
# ax = sns.violinplot(x="scheduler", y="lastScore", hue="bwSensitivity",
#                     data=all, palette="Set2", split=True,
#                     scale="count", inner="quartile")
# fig = ax.get_figure()
# fig.savefig("seaborn-ls.pdf")

#ax = sns.boxplot(x="day", y="lastScore", hue="bwSensitivity",
#                 data=all, palette="Set3")

ax = sns.boxplot(x="scheduler", y="fragScore",
                 hue="bwSensitivity", data=all, palette="Set2")
# ax = sns.swarmplot(x="scheduler", y="fragScore",
#                    hue="bwSensitivity",  data=all, dodge=True, color=".25", palette="Set3")
bx = sns.boxplot(x="scheduler", y="lastScore",
                 hue="bwSensitivity", data=all, palette="Set2")
fig = ax.get_figure()
fig.savefig("seaborn-bp.pdf")

fig2 = bx.get_figure()
fig2.savefig("seaborn-last.pdf")
#ax = sns.violinplot(x=tips["ID"])
