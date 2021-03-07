# Data Compressor
The Data-Compressor is intended to run on the vehicle side coprocessors, to manage th impacts on host storage capacity due to collected logs.

# Work-flow
The Data-Compressor compresses the archived data directories stored in the `/nojournal/bin/archive/` directory. Also, if the disk space running low (below 1 gigabytes), the Data-Compressor deletes the old logs - the older ones are deleted first, till sufficient disk space ecomes free available. 

# Console output and logging
The Data-Compressor does not produce any console output or log files

# Configuration
None

# Requirements
None

# Known issues/limitations 
None
