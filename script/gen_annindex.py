#!/usr/bin/env python

import os
import time
import shutil

frgg = "/home/freestyler/frgg"
def main():
    os.chdir(frgg)
    file = open("etc/annlist")
    for line in file:
        board = line.strip()
        if board == "":
            break
        cmd = "bin/buildannindex " + board + " 2>/dev/null"
        os.system(cmd)
        cmd = "rename index/%s. index/frgg.%s. index/%s.ann*" % (board, board, board)
        os.system(cmd)
        # append log
        logfile = open("reclog/trace.log", "a")
        logstr = "%s built ann index %s\n" % (time.ctime(), board)
        logfile.write(logstr)
        logfile.close()
    file.close()


if __name__ == "__main__":
    main()
