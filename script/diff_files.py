#!/usr/bin/env python

import os

cmd = "diff -U0 "

frgg = "/home/freestyler/frgg"
os.chdir(frgg)

file = open("etc/boards.txt")
for line in file:
    board = line.strip()
    if board == "":
        break

    bdir = "data/boards/" + board
    os.chdir(bdir)
    cmd = "ls M.* | sort > %s.old" % (board)
    os.system(cmd)
    cmd = "diff -U0 %s.old %s > %s.diff" % (board, board, board)
    os.system(cmd)
    os.chdir(frgg)

file.close()
