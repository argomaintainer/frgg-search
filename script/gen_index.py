#!/usr/bin/env python

import os
import time
import shutil

def needs_rebuild(board):
    index_file = "index/frgg.%s.brdidx" % (board)
    result = False
    try:
        mtime = int(os.stat(index_file).st_mtime)
        now = int(time.time())
        if now - mtime > 3600 * 10:
            result = True
    except OSError:
        #index file does not exist
        return True

    filelist = "data/boards/%s/%s" % (board, board)
    cmd = "diff -U0 %s %s.indexed > %s.tmp" % (filelist, filelist, filelist)
    os.system(cmd)
    diff = open(filelist + ".tmp")
    length = len(diff.readlines())
    
    if length > 10:
        result = True
    elif length == 0:
        result = False
    diff.close()
    return result


def main():
    os.chdir("/home/freestyler/frgg")
    file = open("etc/boards.txt")
    for line in file:
        board = line.strip()
        if board == "":
            break
        if needs_rebuild(board):
            cmd = "bin/buildbrdindex " + board + " 2>/dev/null"
            os.system(cmd)
            cmd = "rename index/%s. index/frgg.%s. index/%s.*" % (board, board, board)
            os.system(cmd)
            # record indexed files
            filelist = "data/boards/%s/%s" % (board, board)
            fileindexed = filelist + ".indexed"
            shutil.copyfile(filelist, fileindexed)
            # append log
            logfile = open("reclog/trace.log", "a")    
            logstr = "%s built index %s\n" % (time.ctime(), board)
            logfile.write(logstr)
            logfile.close()
    file.close()


if __name__ == "__main__":
    main()
