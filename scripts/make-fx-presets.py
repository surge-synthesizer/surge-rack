# probably only need to run this once

import xml.etree.ElementTree as ET
import json
import os


tree = ET.parse('res/fxconfig.xml')
fx = tree.getroot()


snapshots = {}


def parseType(typeel, prefix=""):
    defaultI = typeel.attrib['i']
    for child in typeel:
        if child.tag == "snapshot":
            myI = defaultI
            if 'i' in child.attrib:
                myI = child.attrib['i']
            if myI not in snapshots:
                snapshots[myI] = []
            val = child.attrib
            val['name'] = prefix + val['name']
            snapshots[myI].append(val)


def basedata():
    data = {}
    data["plugin"] = "SurgeRack"
    data["version"] = "1.alpha.LOCALBUILD"
    data["model"] = ""
    data["params"] = []
    for i in range(27):
        blank = {}
        blank["id"] = i
        blank["value"] = 0.0

        data["params"].append(blank)

    # Input Output and Mod Gain
    data["params"][12]["value"] = 1.0
    data["params"][13]["value"] = 1.0
    data["params"][26]["value"] = 1.0

    db = {}
    db["comment"] = "No Comment"
    db["buildinfo"] = "python"
    data["data"] = db
    return data


def savePreset(data, pdir, name):
    try:
        os.mkdir("res/presets/" + pdir)
    except IOError:
        1

    name = name.replace("/", "-")
    print("Creating " + pdir + "/" + name + ".vcvm")
    with open("res/presets/" + pdir + "/" + name + ".vcvm", "w") as outf:
        outf.write(json.dumps(data, indent=4))


def ct_lforate(f):
    minv = -7
    maxv = 9
    return (f - minv) / (maxv - minv)


def ct_percent(f):
    return f


def ct_percent_bidirectional(f):
    minv = -1
    maxv = 1
    return (f - minv) / (maxv - minv)


def ct_detuning(f):
    minv = 0
    maxv = 2
    return (f - minv) / (maxv - minv)


def ct_envtime(f):
    minv = -8
    maxv = 5
    return (f - minv) / (maxv - minv)


def ct_amplitude(f):
    minv = 0
    maxv = 1
    return (f - minv) / (maxv - minv)


def ct_freq_audible(f):
    minv = -60
    maxv = 70
    return (f - minv) / (maxv - minv)


def applyRow(data, row, index, xform):
    rowIdx = "p" + str(index)
    tsRow = "p" + str(index) + "_temposync"

    if rowIdx in row:
        data["params"][index]["value"] = xform(float(row[rowIdx]))
    if tsRow in row:
        data["params"][index + 12 + 2]["value"] = float(row[tsRow])


def writeRotary(row):
    data = basedata()
    data["model"] = "SurgeRotary"

    applyRow(data, row, 0, ct_lforate)
    applyRow(data, row, 1, ct_percent)
    applyRow(data, row, 2, ct_percent)

    savePreset(data, "ROTARY", row["name"])


def writePhaser(row):
    data = basedata()
    data["model"] = "SurgePhaser"

    applyRow(data, row, 0, ct_percent_bidirectional)
    applyRow(data, row, 1, ct_percent_bidirectional)
    applyRow(data, row, 2, ct_percent_bidirectional)
    applyRow(data, row, 3, ct_lforate)
    applyRow(data, row, 4, ct_percent)
    applyRow(data, row, 5, ct_percent)
    applyRow(data, row, 6, ct_percent)

    savePreset(data, "PHASER", row["name"])


def writeDelay(row):
    data = basedata()
    data["model"] = "SurgeDelay"

    applyRow(data, row, 0, ct_envtime)
    applyRow(data, row, 1, ct_envtime)
    applyRow(data, row, 2, ct_amplitude)
    applyRow(data, row, 3, ct_amplitude)
    applyRow(data, row, 4, ct_freq_audible)
    applyRow(data, row, 5, ct_freq_audible)
    applyRow(data, row, 6, ct_lforate)
    applyRow(data, row, 7, ct_detuning)
    applyRow(data, row, 8, ct_percent_bidirectional)
    applyRow(data, row, 10, ct_percent)
    applyRow(data, row, 11, ct_percent)

    savePreset(data, "DELAY", row["name"])


for child in fx:
    if child.tag == "type":
        parseType(child)


for s in snapshots["4"]:
    writeRotary(s)

for s in snapshots["3"]:
    writePhaser(s)

for s in snapshots["1"]:
    writeDelay(s)
