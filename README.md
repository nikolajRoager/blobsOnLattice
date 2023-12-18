# blobsOnLattice
Inspired by the YouTuber Primer, a simulation of two "blob" species on a 2D lattice

This C++ script simulates blobs on a 2D lattice.

Each blob lives in a "home" on this lattice, each turn, the blobs randomly go to visit the trees near its home (prioritizing empty trees, and never picking a tree with 2 blobs already), or in the 4 neighbouring sites (the order blobs picks trees is random, and blobs do not get any higher priority when picking trees at their own home).

Each blob gains 2 food when alone, when two blobs meet, the blobs are either "friendly" or "solo". If two friendly blobs meet, they help each other harvest 2 food for each (but lose 1/4 food worth of energy doing so, for a reward of 1.75 each), if a solo blob meets a friendly blob, the solo blob steals 1.5 food, leaving 0.5 for the friendly blob, and if two solo blobs meet, they fight each other before eating, giving 0.75 food for each.

The compiled program has not been included (it needs to be re-compiled for each operating system), but some output is included

##The output
The included data is from the default setup, with 32 by 32 homes, and 4 trees per home, starting with 1 friendly and 1 solo blob far apart. the file population.pdf shows the population on each site (of either species) each turn, the file demographic.pdf shows the total population of both species over time, and the file nations.pdf shows the "friendliness" on each site (friendly-solo population). Not all turns are shown in the files, as the files would be come too large to upload.

With the current setup, two nations form (because the nations don't mix well), but the more successful friendly nation outcompetes the solo nation (very slowly).

##WARNING, and building the program.
The program has not been tested on Windows, and might not work. No executable or binary files are included (because running Linux executables on Windows is a terrible idea).

You need to compile the single C++ file (src/main.cpp), it should only require default C++ libraries.

The Gnu-makefile should only be used on Linux.

The program should not need permission to edit files on your hard drive!

Once built, the program can be run from the command line with two arguments: the width of the square lattice, the number of trees on each site, food from a tree when alone, reward for two team blobs, reward for a team blob vs a solo blob, reward for a solo blob vs a team blob, reward for solo blob vs solo blob.

The Standard console output is used to print the total population each turn, and the Standard error output is used for printing the populations on each site, on Linux, these can be piped to a file like this: `bin/blobs_linux 32 4 2.0 1.75 0.5 1.5 0.75 2> population.tsv 1> demographics.tsv`, which creates the data shown here.

The Gnuplot file plot.gpi creates the plots, based on these two files (Gnuplot is available through most Linux package managers).
