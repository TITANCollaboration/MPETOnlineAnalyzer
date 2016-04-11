# Analyzer

The TITAN online analyzer monitors the MIDAS data stream, and creates histograms that can be seen on the web or though the 
roody spectrum display program. The analyzer's purpose is to show the data in real time, so that the state of the 
experiment can be monitored.

# Starting the Analyzer

When logged in as mpet@titan01, go to the Analyzer directory (~/Analyzer), and run the command:

    ./analyzer.exe -p9090

This will start the analyzer, and open port 9090 for roody

# Viewing the analyzer through the web

The easiest way to view the analyzer is through a web browser by navigating to http://titan01.triumf.ca:8084.
To automatically update the histograms, click the 'Monitoring' box. This will refresh the histograms every second or so.

NOTE: This only works with MODERN browsers. konqueror, the broswer we typically use on titan01 and lxmpet, does not implement
enough javascript for the online analyzer view to work. On these machines, use firefox, or roody (see below).

# Viewing the analyzer through roody

When on titan01 you can use "roody" to view the online analyzer. To start roody:

    cd
    roody


first you 'cd' to the home directory, so that the correct plots can be created. 'roody' is started by simply calling roody.
This will automatically connect roody to the midas data stream and open some useful plots.
